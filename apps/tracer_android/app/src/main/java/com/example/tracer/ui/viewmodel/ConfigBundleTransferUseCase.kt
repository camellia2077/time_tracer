package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.tomlj.Toml
import org.tomlj.TomlParseResult

internal class ConfigBundleTransferUseCase(
    private val controller: RuntimeGateway
) {
    suspend fun buildAndroidExportBundle(): ConfigExportBuildResult {
        val entriesByPath = linkedMapOf<String, ConfigExportEntry>()
        for (path in AndroidConfigPathPolicy.requiredImportPaths) {
            val readResult = controller.readConfigTomlFile(path)
            if (!readResult.ok) {
                return ConfigExportBuildResult(
                    ok = false,
                    message = "Export failed: $path -> ${readResult.message}"
                )
            }

            val normalizedPath = AndroidConfigPathPolicy.normalizeRelativePath(readResult.filePath)
                ?: return ConfigExportBuildResult(
                    ok = false,
                    message = "Export failed: invalid config path -> ${readResult.filePath}"
                )
            if (!AndroidConfigPathPolicy.isAllowedImportPath(normalizedPath)) {
                return ConfigExportBuildResult(
                    ok = false,
                    message = "Export failed: path is not in Android whitelist -> $normalizedPath"
                )
            }

            entriesByPath[normalizedPath] = ConfigExportEntry(
                relativePath = normalizedPath,
                content = readResult.content
            )
        }

        val entries = entriesByPath.values.sortedBy { it.relativePath }

        val bundleEntry = entries.firstOrNull { it.relativePath == "meta/bundle.toml" }
            ?: return ConfigExportBuildResult(
                ok = false,
                message = "Export failed: missing meta/bundle.toml."
            )
        val bundleParseResult = Toml.parse(normalizeTomlContent(bundleEntry.content))
        if (bundleParseResult.hasErrors()) {
            val firstError = bundleParseResult.errors().firstOrNull()?.toString().orEmpty()
            return ConfigExportBuildResult(
                ok = false,
                message = "Export failed: invalid meta/bundle.toml -> $firstError"
            )
        }
        val exportProfile = readBundleProfile(bundleParseResult)
        if (exportProfile != "android") {
            return ConfigExportBuildResult(
                ok = false,
                message = "Export failed: meta/bundle.toml is not android profile."
            )
        }

        return ConfigExportBuildResult(
            ok = true,
            message = "Prepared ${entries.size} Android config TOML file(s).",
            bundle = ConfigExportBundle(
                entries = entries
            )
        )
    }

    suspend fun importAndroidConfigBundle(
        entries: List<ConfigImportEntry>
    ): ConfigImportApplyResult = withContext(Dispatchers.IO) {
        if (entries.isEmpty()) {
            return@withContext ConfigImportApplyResult(
                ok = false,
                message = "Import failed: no TOML files were selected."
            )
        }

        val importedByPath = linkedMapOf<String, String>()
        for (entry in entries) {
            val normalizedPath = AndroidConfigPathPolicy.normalizeRelativePath(entry.relativePath)
                ?: return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: invalid TOML path -> ${entry.relativePath}"
                )
            if (!AndroidConfigPathPolicy.isAllowedImportPath(normalizedPath)) {
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: unsupported TOML path -> $normalizedPath"
                )
            }
            if (importedByPath.containsKey(normalizedPath)) {
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: duplicate TOML path -> $normalizedPath"
                )
            }
            importedByPath[normalizedPath] = normalizeTomlContent(entry.content)
        }

        val missingPaths = AndroidConfigPathPolicy.requiredImportPaths
            .filterNot { importedByPath.containsKey(it) }
        if (missingPaths.isNotEmpty()) {
            return@withContext ConfigImportApplyResult(
                ok = false,
                message = "Import failed: missing required TOML file(s): ${missingPaths.joinToString(", ")}"
            )
        }

        val parsedByPath = linkedMapOf<String, TomlParseResult>()
        for (path in AndroidConfigPathPolicy.requiredImportPaths) {
            val content = importedByPath[path]
                ?: return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: missing required TOML file -> $path"
                )
            val parseResult = Toml.parse(content)
            if (parseResult.hasErrors()) {
                val firstError = parseResult.errors().firstOrNull()?.toString().orEmpty()
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: TOML syntax error in $path -> $firstError"
                )
            }
            parsedByPath[path] = parseResult
        }

        val bundleProfile = parsedByPath["meta/bundle.toml"]?.let(::readBundleProfile)
        if (bundleProfile != "android") {
            return@withContext ConfigImportApplyResult(
                ok = false,
                message = "Import failed: meta/bundle.toml profile must be \"android\"."
            )
        }

        val backupByPath = linkedMapOf<String, String>()
        for (path in AndroidConfigPathPolicy.requiredImportPaths) {
            val readResult = controller.readConfigTomlFile(path)
            if (!readResult.ok) {
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: cannot read existing $path -> ${readResult.message}"
                )
            }
            backupByPath[path] = readResult.content
        }

        val changedPaths = mutableListOf<String>()
        for (path in AndroidConfigPathPolicy.requiredImportPaths) {
            val saveResult = controller.saveConfigTomlFile(
                relativePath = path,
                content = importedByPath.getValue(path)
            )
            if (!saveResult.ok) {
                rollbackConfigChanges(changedPaths, backupByPath)
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: cannot replace $path -> ${saveResult.message}"
                )
            }
            changedPaths += path
        }

        return@withContext ConfigImportApplyResult(
            ok = true,
            message = "Import success: replaced ${changedPaths.size} Android config TOML file(s)."
        )
    }

    suspend fun importAndroidConfigPartialUpdate(
        entries: List<ConfigImportEntry>
    ): ConfigImportApplyResult = withContext(Dispatchers.IO) {
        if (entries.isEmpty()) {
            return@withContext ConfigImportApplyResult(
                ok = false,
                message = "Import failed: no TOML files were selected."
            )
        }

        val importedByCanonicalPath = linkedMapOf<String, String>()
        var skippedTomlCount = 0
        for (entry in entries) {
            val canonicalPath = AndroidConfigPathPolicy
                .resolveCanonicalImportPathForPartial(entry.relativePath)
            if (canonicalPath == null) {
                skippedTomlCount += 1
                continue
            }
            if (importedByCanonicalPath.containsKey(canonicalPath)) {
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: duplicate TOML target -> $canonicalPath"
                )
            }
            importedByCanonicalPath[canonicalPath] = normalizeTomlContent(entry.content)
        }

        if (importedByCanonicalPath.isEmpty()) {
            return@withContext ConfigImportApplyResult(
                ok = false,
                message = "Import failed: no recognized Android config TOML files found."
            )
        }

        val updatePaths = AndroidConfigPathPolicy.requiredImportPaths
            .filter { importedByCanonicalPath.containsKey(it) }
        val parsedByPath = linkedMapOf<String, TomlParseResult>()
        for (path in updatePaths) {
            val content = importedByCanonicalPath[path]
                ?: return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: missing partial TOML file -> $path"
                )
            val parseResult = Toml.parse(content)
            if (parseResult.hasErrors()) {
                val firstError = parseResult.errors().firstOrNull()?.toString().orEmpty()
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: TOML syntax error in $path -> $firstError"
                )
            }
            parsedByPath[path] = parseResult
        }

        val bundleParseResult = parsedByPath["meta/bundle.toml"]
        if (bundleParseResult != null) {
            val bundleProfile = readBundleProfile(bundleParseResult)
            if (bundleProfile != "android") {
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: meta/bundle.toml profile must be \"android\"."
                )
            }
        }

        val backupByPath = linkedMapOf<String, String>()
        for (path in updatePaths) {
            val readResult = controller.readConfigTomlFile(path)
            if (!readResult.ok) {
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: cannot read existing $path -> ${readResult.message}"
                )
            }
            backupByPath[path] = readResult.content
        }

        val changedPaths = mutableListOf<String>()
        for (path in updatePaths) {
            val saveResult = controller.saveConfigTomlFile(
                relativePath = path,
                content = importedByCanonicalPath.getValue(path)
            )
            if (!saveResult.ok) {
                rollbackConfigChanges(changedPaths, backupByPath)
                return@withContext ConfigImportApplyResult(
                    ok = false,
                    message = "Import failed: cannot replace $path -> ${saveResult.message}"
                )
            }
            changedPaths += path
        }

        val skipSuffix = if (skippedTomlCount > 0) {
            " (skipped $skippedTomlCount unsupported TOML file(s))"
        } else {
            ""
        }
        return@withContext ConfigImportApplyResult(
            ok = true,
            message = "Import success (partial): replaced ${changedPaths.size} Android config TOML file(s).$skipSuffix"
        )
    }

    private suspend fun rollbackConfigChanges(
        changedPaths: List<String>,
        backupByPath: Map<String, String>
    ) {
        for (path in changedPaths.asReversed()) {
            val backup = backupByPath[path] ?: continue
            controller.saveConfigTomlFile(
                relativePath = path,
                content = backup
            )
        }
    }

    private fun normalizeTomlContent(content: String): String {
        return content
            .removePrefix("\uFEFF")
            .replace("\r\n", "\n")
            .replace('\r', '\n')
    }

    private fun readBundleProfile(parseResult: TomlParseResult): String? {
        return runCatching {
            parseResult.getString("profile")
        }.getOrNull()?.trim()
    }
}
