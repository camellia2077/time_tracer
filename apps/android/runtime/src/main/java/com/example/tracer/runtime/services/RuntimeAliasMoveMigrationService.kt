package com.example.tracer

import java.io.File
import java.nio.file.Files
import java.nio.file.StandardCopyOption
import java.util.UUID
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.sync.Mutex
import kotlinx.coroutines.sync.withLock
import kotlinx.coroutines.withContext
import org.json.JSONArray
import org.json.JSONObject

// The source files are deliberately changed before candidate ingestion: core
// validates exactly the config/TXT pair that will become active. The old DB is
// never touched until that candidate database has been built successfully.
internal class RuntimeAliasMoveMigrationService(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val ensureTextStorage: () -> TextStorage,
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage,
    private val nativeInit: (RuntimePaths) -> String,
    private val nativeShutdown: () -> String,
    private val nativeIngest: (String, Int, Boolean) -> String,
    private val nativeTxt: (String) -> String,
    private val responseCodec: NativeResponseCodec
) {
    private val mutex = Mutex()

    suspend fun apply(request: AliasEntryMoveMigrationRequest): AliasEntryMoveMigrationResult =
        withContext(Dispatchers.IO) {
            mutex.withLock { applyLocked(request) }
        }

    private fun applyLocked(request: AliasEntryMoveMigrationRequest): AliasEntryMoveMigrationResult {
        if (request.replacements.isEmpty()) {
            return AliasEntryMoveMigrationResult(false, "Move plan has no canonical replacement.")
        }
        val paths = try {
            ensureRuntimePaths()
        } catch (error: Exception) {
            return AliasEntryMoveMigrationResult(false, formatNativeFailure("prepare alias move migration failed", error))
        }
        val textStorage = ensureTextStorage()
        val configStorage = ensureConfigTomlStorage()
        val originalToml = configStorage.readTomlFile(request.configRelativePath)
        if (!originalToml.ok) {
            return AliasEntryMoveMigrationResult(false, originalToml.message)
        }
        val txtOriginals = linkedMapOf<String, String>()
        val txtCandidates = linkedMapOf<String, String>()
        val listed = textStorage.listTxtFiles()
        if (!listed.ok) {
            return AliasEntryMoveMigrationResult(false, listed.message)
        }
        try {
            val init = responseCodec.parse(nativeInit(paths))
            if (!init.ok) {
                return AliasEntryMoveMigrationResult(false, init.errorMessage.ifBlank { "native init failed." })
            }
            for (relativePath in listed.files) {
                val original = textStorage.readTxtFile(relativePath)
                if (!original.ok) {
                    return AliasEntryMoveMigrationResult(false, original.message)
                }
                txtOriginals[relativePath] = original.content
                val replaced = replaceCanonicalNames(original.content, request.replacements)
                if (!replaced.ok) {
                    return AliasEntryMoveMigrationResult(false, replaced.message)
                }
                if (replaced.updatedContent != original.content) {
                    txtCandidates[relativePath] = replaced.updatedContent
                }
            }
        } catch (error: Exception) {
            return AliasEntryMoveMigrationResult(false, formatNativeFailure("build TXT migration plan failed", error))
        }

        val transactionRoot = File(paths.cacheRootPath, "alias-move-${UUID.randomUUID()}")
        var sourcesWritten = false
        var databaseSwapStarted = false
        try {
            require(transactionRoot.mkdirs()) { "Cannot create migration cache directory." }
            sourcesWritten = true
            writeSources(configStorage, textStorage, request, txtCandidates)

            val candidatePaths = paths.copy(
                dbPath = File(transactionRoot, "candidate/time_data.sqlite3").absolutePath,
                outputRoot = File(transactionRoot, "candidate/output").absolutePath,
                cacheRootPath = File(transactionRoot, "candidate/cache").absolutePath
            )
            File(candidatePaths.outputRoot).mkdirs()
            File(candidatePaths.cacheRootPath).mkdirs()
            val candidateInit = responseCodec.parse(nativeInit(candidatePaths))
            require(candidateInit.ok) { candidateInit.errorMessage.ifBlank { "candidate native init failed." } }
            val ingest = responseCodec.parse(
                nativeIngest(paths.inputRootPath, NativeBridge.DATE_CHECK_CONTINUITY, false)
            )
            require(ingest.ok) { ingest.errorMessage.ifBlank { "candidate database ingest failed." } }
            val shutdown = responseCodec.parse(nativeShutdown())
            require(shutdown.ok) { shutdown.errorMessage.ifBlank { "candidate runtime shutdown failed." } }

            databaseSwapStarted = true
            replaceDatabase(paths.dbPath, candidatePaths.dbPath, transactionRoot)
            val activeInit = responseCodec.parse(nativeInit(paths))
            require(activeInit.ok) { activeInit.errorMessage.ifBlank { "active runtime init failed." } }
            transactionRoot.deleteRecursively()
            return AliasEntryMoveMigrationResult(
                ok = true,
                message = "Moved canonical path and rebuilt database.",
                updatedTxtFileCount = txtCandidates.size
            )
        } catch (error: Exception) {
            runCatching { nativeShutdown() }
            val rollbackProblems = mutableListOf<String>()
            if (sourcesWritten) {
                rollbackSources(configStorage, textStorage, originalToml.content, request.configRelativePath, txtOriginals, rollbackProblems)
            }
            if (databaseSwapStarted) {
                runCatching { restoreDatabase(paths.dbPath, transactionRoot) }
                    .onFailure { rollbackProblems += it.message ?: "database restore failed" }
            }
            val restoreInit = responseCodec.parse(nativeInit(paths))
            if (!restoreInit.ok) {
                rollbackProblems += restoreInit.errorMessage.ifBlank { "old runtime reinitialization failed" }
            }
            val suffix = if (rollbackProblems.isEmpty()) "" else " Rollback issues: ${rollbackProblems.joinToString("; ")}" 
            return AliasEntryMoveMigrationResult(false, (error.message ?: "Alias move migration failed.") + suffix)
        }
    }

    private fun replaceCanonicalNames(
        content: String,
        replacements: List<CanonicalActivityNameReplacement>
    ): TxtCanonicalActivityReplacementResult {
        val payload = JSONObject()
            .put("action", "replace_canonical_activity_names")
            .put("content", content)
            .put("replacements", JSONArray().apply {
                replacements.forEach { replacement ->
                    put(JSONObject()
                        .put("old_canonical", replacement.oldCanonical)
                        .put("new_canonical", replacement.newCanonical))
                }
            })
        return NativeTxtRuntimeCodec().parseCanonicalActivityReplacement(
            nativeTxt(payload.toString()), content
        )
    }

    private fun writeSources(
        configStorage: ConfigTomlStorage,
        textStorage: TextStorage,
        request: AliasEntryMoveMigrationRequest,
        txtCandidates: Map<String, String>
    ) {
        val configWrite = configStorage.writeTomlFile(request.configRelativePath, request.updatedTomlContent)
        require(configWrite.ok) { configWrite.message }
        txtCandidates.forEach { (relativePath, content) ->
            val write = textStorage.writeTxtFile(relativePath, content)
            require(write.ok) { write.message }
        }
    }

    private fun rollbackSources(
        configStorage: ConfigTomlStorage,
        textStorage: TextStorage,
        originalToml: String,
        configPath: String,
        txtOriginals: Map<String, String>,
        problems: MutableList<String>
    ) {
        configStorage.writeTomlFile(configPath, originalToml).takeIf { !it.ok }?.let { problems += it.message }
        txtOriginals.forEach { (relativePath, content) ->
            textStorage.writeTxtFile(relativePath, content).takeIf { !it.ok }?.let { problems += it.message }
        }
    }

    private fun replaceDatabase(activePath: String, candidatePath: String, transactionRoot: File) {
        val active = File(activePath)
        val candidate = File(candidatePath)
        require(candidate.isFile) { "Candidate database was not created." }
        val backupRoot = File(transactionRoot, "backup")
        require(backupRoot.mkdirs()) { "Cannot create database backup directory." }
        moveSqliteFiles(active, backupRoot, replaceExisting = true)
        moveSqliteFiles(candidate, active.parentFile ?: error("Database directory is unavailable"), replaceExisting = true)
    }

    private fun restoreDatabase(activePath: String, transactionRoot: File) {
        val active = File(activePath)
        deleteSqliteFiles(active)
        val backupDatabase = File(transactionRoot, "backup/${active.name}")
        if (backupDatabase.exists()) {
            moveSqliteFiles(backupDatabase, active.parentFile ?: error("Database directory is unavailable"), replaceExisting = true)
        }
    }

    private fun moveSqliteFiles(sourceDatabase: File, targetDirectory: File, replaceExisting: Boolean) {
        targetDirectory.mkdirs()
        listOf(sourceDatabase, File(sourceDatabase.path + "-wal"), File(sourceDatabase.path + "-shm"))
            .filter(File::exists)
            .forEach { source ->
                val target = File(targetDirectory, source.name)
                if (replaceExisting) {
                    Files.move(source.toPath(), target.toPath(), StandardCopyOption.REPLACE_EXISTING)
                } else {
                    Files.move(source.toPath(), target.toPath())
                }
            }
    }

    private fun deleteSqliteFiles(database: File) {
        listOf(database, File(database.path + "-wal"), File(database.path + "-shm"))
            .forEach { Files.deleteIfExists(it.toPath()) }
    }
}
