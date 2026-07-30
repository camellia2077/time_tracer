package com.example.tracer

import java.io.File
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
internal class RuntimeActivityHierarchyMigrationService(
    private val ensureRuntimePaths: () -> RuntimePaths,
    private val ensureTextStorage: () -> TextStorage,
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage,
    private val nativeInit: (RuntimePaths) -> String,
    private val nativeInitPipeline: (RuntimePaths) -> String,
    private val nativeShutdown: () -> String,
    private val nativeIngest: (String, Int, Boolean) -> String,
    private val nativeTxt: (String) -> String,
    private val responseCodec: NativeResponseCodec,
    private val txtCodec: NativeTxtRuntimeCodec = NativeTxtRuntimeCodec(),
    private val databaseSwap: RuntimeSqliteDatabaseSwap = RuntimeSqliteDatabaseSwap()
) {
    private val mutex = Mutex()

    suspend fun apply(request: ActivityHierarchyMigrationRequest): ActivityHierarchyMigrationResult =
        withContext(Dispatchers.IO) {
            mutex.withLock { applyLocked(request) }
        }

    private fun applyLocked(request: ActivityHierarchyMigrationRequest): ActivityHierarchyMigrationResult {
        val paths = try {
            ensureRuntimePaths()
        } catch (error: Exception) {
            return ActivityHierarchyMigrationResult(false, formatNativeFailure("prepare activity hierarchy migration failed", error))
        }
        val textStorage = ensureTextStorage()
        val configStorage = ensureConfigTomlStorage()
        val baseDocumentUpdates = if (request.updatedDocuments.isEmpty()) {
            listOf(ActivityHierarchyDocumentInput(request.configRelativePath, request.updatedTomlContent))
        } else {
            request.updatedDocuments
        }
        val documentUpdates = request.configFileRename?.let { rename ->
            baseDocumentUpdates.map { document ->
                if (document.sourceName == rename.oldSourceName) {
                    document.copy(sourceName = rename.newSourceName)
                } else {
                    document
                }
            }
        } ?: baseDocumentUpdates
        val originalConfigPaths = buildList {
            addAll(documentUpdates.map { it.sourceName })
            request.configFileRename?.oldSourceName?.let(::add)
        }.distinct()
        val originalTomls = linkedMapOf<String, String?>()
        originalConfigPaths.forEach { sourceName ->
            val original = configStorage.readTomlFile(sourceName)
            val expectedMissingTarget = request.configFileRename?.newSourceName == sourceName && !original.ok
            if (request.configFileRename?.oldSourceName == sourceName && !original.ok) {
                return ActivityHierarchyMigrationResult(false, original.message)
            }
            if (!original.ok && !request.allowMissingConfig && !expectedMissingTarget) {
                return ActivityHierarchyMigrationResult(false, original.message)
            }
            if (request.configFileRename?.newSourceName == sourceName && original.ok) {
                return ActivityHierarchyMigrationResult(false, "Target activity category already exists: $sourceName")
            }
            originalTomls[sourceName] = original.content.takeIf { original.ok }
        }
        val updatedConfigPath = request.configFileRename?.newSourceName ?: request.configRelativePath
        var updatedTomlContent = documentUpdates
            .firstOrNull { it.sourceName == updatedConfigPath }
            ?.tomlContent
            ?: request.updatedTomlContent
        val updatedDocuments = documentUpdates
        val replacementPlan = request.replacementPlan
        val txtOriginals = linkedMapOf<String, String>()
        val txtCandidates = linkedMapOf<String, String>()
        val listed = textStorage.listTxtFiles()
        if (!listed.ok) {
            return ActivityHierarchyMigrationResult(false, listed.message)
        }
        try {
            val init = responseCodec.parse(nativeInit(paths))
            if (!init.ok) {
                return ActivityHierarchyMigrationResult(false, init.errorMessage.ifBlank { "native init failed." })
            }
            for (relativePath in listed.files) {
                val original = textStorage.readTxtFile(relativePath)
                if (!original.ok) {
                    return ActivityHierarchyMigrationResult(false, original.message)
                }
                txtOriginals[relativePath] = original.content
                val canonicalReplaced = replaceCanonicalNames(
                    original.content,
                    replacementPlan.canonical
                )
                if (!canonicalReplaced.ok) {
                    return ActivityHierarchyMigrationResult(false, canonicalReplaced.message)
                }
                val replaced = replaceAliasNames(
                    canonicalReplaced.updatedContent,
                    replacementPlan.aliases
                )
                if (!replaced.ok) {
                    return ActivityHierarchyMigrationResult(false, replaced.message)
                }
                if (replaced.updatedContent != original.content) {
                    txtCandidates[relativePath] = replaced.updatedContent
                }
            }
        } catch (error: Exception) {
            return ActivityHierarchyMigrationResult(false, formatNativeFailure("build TXT migration plan failed", error))
        }

        val transactionRoot = File(paths.cacheRootPath, "alias-move-${UUID.randomUUID()}")
        var sourcesWritten = false
        var databaseSwapStarted = false
        try {
            require(transactionRoot.mkdirs()) { "Cannot create migration cache directory." }
            sourcesWritten = true
            writeSources(
                configStorage = configStorage,
                textStorage = textStorage,
                updatedDocuments = updatedDocuments,
                removedTomlPaths = request.configFileRename
                    ?.let { listOf(it.oldSourceName) }
                    .orEmpty(),
                txtCandidates = txtCandidates
            )

            val candidatePaths = paths.copy(
                dbPath = File(transactionRoot, "candidate/time_data.sqlite3").absolutePath,
                outputRoot = File(transactionRoot, "candidate/output").absolutePath,
                cacheRootPath = File(transactionRoot, "candidate/cache").absolutePath
            )
            File(candidatePaths.outputRoot).mkdirs()
            File(candidatePaths.cacheRootPath).mkdirs()
            val candidateInit = responseCodec.parse(nativeInitPipeline(candidatePaths))
            require(candidateInit.ok) { candidateInit.errorMessage.ifBlank { "candidate native init failed." } }
            val ingest = responseCodec.parse(
                nativeIngest(paths.inputRootPath, NativeBridge.DATE_CHECK_CONTINUITY, false)
            )
            require(ingest.ok) { ingest.errorMessage.ifBlank { "candidate database ingest failed." } }
            val shutdown = responseCodec.parse(nativeShutdown())
            require(shutdown.ok) { shutdown.errorMessage.ifBlank { "candidate runtime shutdown failed." } }

            databaseSwapStarted = true
            databaseSwap.replace(paths.dbPath, candidatePaths.dbPath, transactionRoot)
            val activeInit = responseCodec.parse(nativeInit(paths))
            require(activeInit.ok) { activeInit.errorMessage.ifBlank { "active runtime init failed." } }
            transactionRoot.deleteRecursively()
            return ActivityHierarchyMigrationResult(
                ok = true,
                message = "Applied activity hierarchy migration and rebuilt database.",
                updatedTxtFileCount = txtCandidates.size,
                updatedTomlContent = updatedTomlContent,
                updatedConfigRelativePath = updatedConfigPath
            )
        } catch (error: Exception) {
            runCatching { nativeShutdown() }
            val rollbackProblems = mutableListOf<String>()
            if (sourcesWritten) {
                rollbackSources(configStorage, textStorage, originalTomls, txtOriginals, rollbackProblems)
            }
            if (databaseSwapStarted) {
                runCatching { databaseSwap.restore(paths.dbPath, transactionRoot) }
                    .onFailure { rollbackProblems += it.message ?: "database restore failed" }
            }
            val restoreInit = responseCodec.parse(nativeInit(paths))
            if (!restoreInit.ok) {
                rollbackProblems += restoreInit.errorMessage.ifBlank { "old runtime reinitialization failed" }
            }
            val suffix = if (rollbackProblems.isEmpty()) "" else " Rollback issues: ${rollbackProblems.joinToString("; ")}" 
            return ActivityHierarchyMigrationResult(false, (error.message ?: "Activity hierarchy migration failed.") + suffix)
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
        return txtCodec.parseCanonicalActivityReplacement(
            nativeTxt(payload.toString()), content
        )
    }

    private fun replaceAliasNames(
        content: String,
        replacements: List<AliasKeyReplacement>
    ): TxtCanonicalActivityReplacementResult {
        val payload = JSONObject()
            .put("action", "replace_alias_activity_names")
            .put("content", content)
            .put("replacements", JSONArray().apply {
                replacements.forEach { replacement ->
                    put(JSONObject()
                        .put("old_alias", replacement.oldAlias)
                        .put("new_alias", replacement.newAlias))
                }
            })
        return txtCodec.parseCanonicalActivityReplacement(
            nativeTxt(payload.toString()), content
        )
    }

    private fun writeSources(
        configStorage: ConfigTomlStorage,
        textStorage: TextStorage,
        updatedDocuments: List<ActivityHierarchyDocumentInput>,
        removedTomlPaths: List<String>,
        txtCandidates: Map<String, String>
    ) {
        updatedDocuments.forEach { document ->
            val configWrite = configStorage.writeTomlFile(document.sourceName, document.tomlContent)
            require(configWrite.ok) { configWrite.message }
        }
        removedTomlPaths
            .filter { oldPath -> updatedDocuments.none { it.sourceName == oldPath } }
            .forEach { oldPath ->
                val configDelete = configStorage.deleteTomlFile(oldPath)
                require(configDelete.ok) { configDelete.message }
            }
        txtCandidates.forEach { (relativePath, content) ->
            val write = textStorage.writeTxtFile(relativePath, content)
            require(write.ok) { write.message }
        }
    }

    private fun rollbackSources(
        configStorage: ConfigTomlStorage,
        textStorage: TextStorage,
        originalTomls: Map<String, String?>,
        txtOriginals: Map<String, String>,
        problems: MutableList<String>
    ) {
        originalTomls.forEach { (configPath, originalToml) ->
            if (originalToml == null) {
                val current = configStorage.readTomlFile(configPath)
                if (current.ok) {
                    configStorage.deleteTomlFile(configPath)
                        .takeIf { !it.ok }?.let { problems += it.message }
                }
            } else {
                configStorage.writeTomlFile(configPath, originalToml)
                    .takeIf { !it.ok }?.let { problems += it.message }
            }
        }
        txtOriginals.forEach { (relativePath, content) ->
            textStorage.writeTxtFile(relativePath, content).takeIf { !it.ok }?.let { problems += it.message }
        }
    }

}
