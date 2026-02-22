package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.tomlj.Toml
import org.tomlj.TomlParseResult

enum class ConfigCategory {
    CONVERTER,
    REPORTS
}

data class ConfigUiState(
    val selectedCategory: ConfigCategory = ConfigCategory.CONVERTER,
    val converterFiles: List<String> = emptyList(),
    val reportFiles: List<String> = emptyList(),
    val selectedFile: String = "",
    val selectedFileContent: String = "",
    val editableContent: String = "",
    val statusText: String = "Preparing config..."
)

data class ConfigExportEntry(
    val relativePath: String,
    val content: String
)

data class ConfigExportBundle(
    val entries: List<ConfigExportEntry>
)

data class ConfigExportBuildResult(
    val ok: Boolean,
    val message: String,
    val bundle: ConfigExportBundle? = null
)

data class ConfigImportEntry(
    val relativePath: String,
    val content: String
)

data class ConfigImportApplyResult(
    val ok: Boolean,
    val message: String
)

class ConfigViewModel(private val controller: RuntimeGateway) : ViewModel() {
    var uiState by mutableStateOf(ConfigUiState())
        private set

    init {
        refreshConfigFiles()
    }

    fun refreshConfigFiles() {
        viewModelScope.launch {
            uiState = uiState.copy(statusText = "refreshing config toml...")
            val listResult = controller.listConfigTomlFiles()
            if (!listResult.ok) {
                uiState = uiState.copy(statusText = listResult.message)
                return@launch
            }

            val updated = uiState.copy(
                converterFiles = listResult.converterFiles,
                reportFiles = listResult.reportFiles
            )
            val files = filesForCategory(updated, updated.selectedCategory)
            val targetFile = when {
                files.contains(updated.selectedFile) -> updated.selectedFile
                files.isNotEmpty() -> files.first()
                else -> ""
            }
            if (targetFile.isEmpty()) {
                uiState = updated.copy(
                    selectedFile = "",
                    selectedFileContent = "",
                    editableContent = "",
                    statusText = listResult.message
                )
                return@launch
            }

            val readResult = controller.readConfigTomlFile(targetFile)
            uiState = if (readResult.ok) {
                updated.copy(
                    selectedFile = readResult.filePath,
                    selectedFileContent = readResult.content,
                    editableContent = readResult.content,
                    statusText = listResult.message
                )
            } else {
                updated.copy(statusText = readResult.message)
            }
        }
    }

    fun selectCategory(category: ConfigCategory) {
        viewModelScope.launch {
            val changed = uiState.copy(selectedCategory = category)
            val files = filesForCategory(changed, category)
            if (files.isEmpty()) {
                uiState = changed.copy(
                    selectedFile = "",
                    selectedFileContent = "",
                    editableContent = "",
                    statusText = "No TOML files in ${category.name.lowercase()}."
                )
                return@launch
            }

            val targetFile = if (files.contains(changed.selectedFile)) {
                changed.selectedFile
            } else {
                files.first()
            }
            val readResult = controller.readConfigTomlFile(targetFile)
            uiState = if (readResult.ok) {
                changed.copy(
                    selectedFile = readResult.filePath,
                    selectedFileContent = readResult.content,
                    editableContent = readResult.content,
                    statusText = "open toml -> ${readResult.filePath}"
                )
            } else {
                changed.copy(statusText = readResult.message)
            }
        }
    }

    fun openFile(path: String) {
        val trimmedPath = path.trim()
        if (trimmedPath.isEmpty()) {
            return
        }
        viewModelScope.launch {
            val readResult = controller.readConfigTomlFile(trimmedPath)
            uiState = if (readResult.ok) {
                uiState.copy(
                    selectedFile = readResult.filePath,
                    selectedFileContent = readResult.content,
                    editableContent = readResult.content,
                    statusText = "open toml -> ${readResult.filePath}"
                )
            } else {
                uiState.copy(statusText = readResult.message)
            }
        }
    }

    fun onEditableContentChange(value: String) {
        uiState = uiState.copy(editableContent = value)
    }

    fun setStatusText(message: String) {
        uiState = uiState.copy(statusText = message)
    }

    fun saveCurrentFile() {
        val selectedFile = uiState.selectedFile
        if (selectedFile.isEmpty()) {
            uiState = uiState.copy(statusText = "No TOML file selected.")
            return
        }
        viewModelScope.launch {
            val saveResult = controller.saveConfigTomlFile(
                relativePath = selectedFile,
                content = uiState.editableContent
            )
            uiState = if (saveResult.ok) {
                uiState.copy(
                    selectedFileContent = uiState.editableContent,
                    statusText = "save toml -> ${saveResult.filePath}"
                )
            } else {
                uiState.copy(statusText = saveResult.message)
            }
        }
    }

    fun discardUnsavedDraft() {
        if (uiState.editableContent == uiState.selectedFileContent) {
            return
        }
        uiState = uiState.copy(
            editableContent = uiState.selectedFileContent
        )
    }

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

    private fun filesForCategory(state: ConfigUiState, category: ConfigCategory): List<String> {
        return when (category) {
            ConfigCategory.CONVERTER -> state.converterFiles
            ConfigCategory.REPORTS -> state.reportFiles
        }
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

class ConfigViewModelFactory(private val controller: RuntimeGateway) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(ConfigViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return ConfigViewModel(controller) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
