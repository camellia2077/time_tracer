package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

internal enum class ConfigCategory {
    // Converter = `converter/*.toml` and `converter/aliases/*.toml`
    CONVERTER,
    // Charts = `charts/*.toml`
    CHARTS,
    // Meta = `config.toml` plus `meta/*.toml`
    META,
    // Reports = `reports/**/*.toml`
    REPORTS
}

internal enum class ConverterSubcategory {
    // Aliases represent the user-managed `converter/aliases/` directory.
    // This set is intentionally dynamic because users define their own
    // activity-name mapping files, so the UI treats it as a high-frequency
    // editable bucket instead of a fixed file list.
    ALIASES,
    // Rules represent the small, stable converter-owned TOML set outside
    // `converter/aliases/`, such as alias index and interval/rule config files.
    RULES
}

internal data class ConfigUiState(
    val selectedCategory: ConfigCategory = ConfigCategory.CONVERTER,
    val selectedConverterSubcategory: ConverterSubcategory = ConverterSubcategory.ALIASES,
    val converterFiles: List<ConfigTomlFileEntry> = emptyList(),
    val chartFiles: List<ConfigTomlFileEntry> = emptyList(),
    val metaFiles: List<ConfigTomlFileEntry> = emptyList(),
    val reportFiles: List<ConfigTomlFileEntry> = emptyList(),
    val selectedFilePath: String = "",
    val selectedFileDisplayName: String = "",
    val selectedFileContent: String = "",
    val editableContent: String = "",
    val aliasEditorMode: AliasEditorMode = AliasEditorMode.STRUCTURED,
    val aliasDocumentDraft: AliasTomlDocument? = null,
    val aliasParentOptions: List<String> = emptyList(),
    val aliasAdvancedTomlDraft: String = "",
    val aliasEditorErrorMessage: String = "",
    val statusText: String = "Preparing config..."
)

internal class ConfigViewModel(private val configGateway: ConfigGateway) : ViewModel() {
    var uiState by mutableStateOf(ConfigUiState())
        private set

    init {
        refreshConfigFiles()
    }

    fun refreshConfigFiles() {
        viewModelScope.launch {
            uiState = uiState.copy(statusText = "refreshing config toml...")
            val listResult = configGateway.listConfigTomlFiles()
            if (!listResult.ok) {
                uiState = uiState.copy(statusText = listResult.message)
                return@launch
            }

            val updated = uiState.copy(
                converterFiles = listResult.converterFiles,
                chartFiles = listResult.chartFiles,
                metaFiles = listResult.metaFiles,
                reportFiles = listResult.reportFiles
            )
            val files = filesForCategory(updated, updated.selectedCategory)
            val targetFile = when {
                files.any { it.relativePath == updated.selectedFilePath } -> updated.selectedFilePath
                files.isNotEmpty() -> files.first().relativePath
                else -> ""
            }
            if (targetFile.isEmpty()) {
                uiState = updated.copy(
                    selectedFilePath = "",
                    selectedFileDisplayName = "",
                    selectedFileContent = "",
                    editableContent = "",
                    aliasDocumentDraft = null,
                    aliasParentOptions = emptyList(),
                    aliasAdvancedTomlDraft = "",
                    aliasEditorErrorMessage = "",
                    statusText = listResult.message
                )
                return@launch
            }

            val readResult = configGateway.readConfigTomlFile(targetFile)
            uiState = if (readResult.ok) {
                val aliasParentOptions = resolveAliasParentOptions(
                    converterFiles = updated.converterFiles,
                    selectedFilePath = readResult.filePath,
                    selectedFileContent = readResult.content
                )
                applyLoadedFile(
                    state = updated,
                    filePath = readResult.filePath,
                    content = readResult.content,
                    aliasParentOptions = aliasParentOptions,
                    statusText = listResult.message
                )
            } else {
                updated.copy(statusText = readResult.message)
            }
        }
    }

    fun selectCategory(category: ConfigCategory) {
        viewModelScope.launch {
            val changed = uiState.copy(
                selectedCategory = category,
                selectedConverterSubcategory = if (category == ConfigCategory.CONVERTER) {
                    ConverterSubcategory.ALIASES
                } else {
                    uiState.selectedConverterSubcategory
                }
            )
            val files = filesForCategory(changed, category)
            if (files.isEmpty()) {
                uiState = changed.copy(
                    selectedFilePath = "",
                    selectedFileDisplayName = "",
                    selectedFileContent = "",
                    editableContent = "",
                    aliasDocumentDraft = null,
                    aliasParentOptions = emptyList(),
                    aliasAdvancedTomlDraft = "",
                    aliasEditorErrorMessage = "",
                    statusText = "No TOML files in ${category.name.lowercase()}."
                )
                return@launch
            }

            val targetFile = if (files.any { it.relativePath == changed.selectedFilePath }) {
                changed.selectedFilePath
            } else {
                files.first().relativePath
            }
            val readResult = configGateway.readConfigTomlFile(targetFile)
            uiState = if (readResult.ok) {
                val aliasParentOptions = resolveAliasParentOptions(
                    converterFiles = changed.converterFiles,
                    selectedFilePath = readResult.filePath,
                    selectedFileContent = readResult.content
                )
                applyLoadedFile(
                    state = changed,
                    filePath = readResult.filePath,
                    content = readResult.content,
                    aliasParentOptions = aliasParentOptions,
                    statusText = "open toml -> ${readResult.filePath}"
                )
            } else {
                changed.copy(statusText = readResult.message)
            }
        }
    }

    fun selectConverterSubcategory(subcategory: ConverterSubcategory) {
        if (uiState.selectedCategory != ConfigCategory.CONVERTER) {
            return
        }
        viewModelScope.launch {
            val changed = uiState.copy(selectedConverterSubcategory = subcategory)
            val files = filesForCategory(changed, ConfigCategory.CONVERTER)
            if (files.isEmpty()) {
                uiState = changed.copy(
                    selectedFilePath = "",
                    selectedFileDisplayName = "",
                    selectedFileContent = "",
                    editableContent = "",
                    aliasDocumentDraft = null,
                    aliasParentOptions = emptyList(),
                    aliasAdvancedTomlDraft = "",
                    aliasEditorErrorMessage = "",
                    statusText = "No TOML files in converter."
                )
                return@launch
            }

            val targetFile = if (files.any { it.relativePath == changed.selectedFilePath }) {
                changed.selectedFilePath
            } else {
                files.first().relativePath
            }
            val readResult = configGateway.readConfigTomlFile(targetFile)
            uiState = if (readResult.ok) {
                val aliasParentOptions = resolveAliasParentOptions(
                    converterFiles = changed.converterFiles,
                    selectedFilePath = readResult.filePath,
                    selectedFileContent = readResult.content
                )
                applyLoadedFile(
                    state = changed,
                    filePath = readResult.filePath,
                    content = readResult.content,
                    aliasParentOptions = aliasParentOptions,
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
            val readResult = configGateway.readConfigTomlFile(trimmedPath)
            uiState = if (readResult.ok) {
                val nextConverterSubcategory = if (readResult.filePath.startsWith("converter/aliases/")) {
                    ConverterSubcategory.ALIASES
                } else if (readResult.filePath.startsWith("converter/")) {
                    ConverterSubcategory.RULES
                } else {
                    uiState.selectedConverterSubcategory
                }
                applyLoadedFile(
                    state = uiState.copy(selectedConverterSubcategory = nextConverterSubcategory),
                    filePath = readResult.filePath,
                    content = readResult.content,
                    aliasParentOptions = resolveAliasParentOptions(
                        converterFiles = uiState.converterFiles,
                        selectedFilePath = readResult.filePath,
                        selectedFileContent = readResult.content
                    ),
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

    fun onAliasAdvancedTomlChange(value: String) {
        uiState = uiState.copy(aliasAdvancedTomlDraft = value)
    }

    fun selectAliasEditorMode(mode: AliasEditorMode) {
        if (!isAliasFilePath(uiState.selectedFilePath) || uiState.aliasEditorMode == mode) {
            return
        }
        when (mode) {
            AliasEditorMode.ADVANCED -> {
                val nextRawToml = uiState.aliasDocumentDraft?.let(AliasTomlEditorCodec::serialize)
                    ?: uiState.aliasAdvancedTomlDraft
                uiState = uiState.copy(
                    aliasEditorMode = AliasEditorMode.ADVANCED,
                    aliasAdvancedTomlDraft = nextRawToml,
                    aliasEditorErrorMessage = ""
                )
            }

            AliasEditorMode.STRUCTURED -> {
                val parseResult = AliasTomlEditorCodec.parse(uiState.aliasAdvancedTomlDraft)
                val document = parseResult.document
                if (document == null) {
                    uiState = uiState.copy(
                        aliasEditorErrorMessage = parseResult.errorMessage,
                        statusText = parseResult.errorMessage
                    )
                    return
                }
                uiState = uiState.copy(
                    aliasEditorMode = AliasEditorMode.STRUCTURED,
                    aliasDocumentDraft = document,
                    aliasParentOptions = normalizeAliasParentOptions(
                        uiState.aliasParentOptions + document.parent
                    ),
                    aliasEditorErrorMessage = ""
                )
            }
        }
    }

    fun updateAliasParent(value: String) {
        val normalizedValue = value.trim()
        if (normalizedValue.isEmpty()) {
            return
        }
        val currentFilePath = uiState.selectedFilePath
        if (!isAliasFilePath(currentFilePath)) {
            return
        }
        viewModelScope.launch {
            // Parent selection primarily means "switch to the alias file whose
            // document parent matches the chosen value". Only fall back to
            // in-place parent text update when no matching file exists.
            val targetFilePath = resolveAliasFilePathForParent(
                parent = normalizedValue,
                currentFilePath = currentFilePath
            )
            if (targetFilePath != null && targetFilePath != currentFilePath) {
                val readResult = configGateway.readConfigTomlFile(targetFilePath)
                if (!readResult.ok) {
                    uiState = uiState.copy(statusText = readResult.message)
                    return@launch
                }
                val aliasParentOptions = resolveAliasParentOptions(
                    converterFiles = uiState.converterFiles,
                    selectedFilePath = readResult.filePath,
                    selectedFileContent = readResult.content
                )
                uiState = applyLoadedFile(
                    state = uiState.copy(selectedConverterSubcategory = ConverterSubcategory.ALIASES),
                    filePath = readResult.filePath,
                    content = readResult.content,
                    aliasParentOptions = aliasParentOptions,
                    statusText = "open toml -> ${readResult.filePath}"
                )
                return@launch
            }

            val document = uiState.aliasDocumentDraft ?: return@launch
            uiState = uiState.copy(
                aliasDocumentDraft = document.updateParent(normalizedValue),
                aliasEditorErrorMessage = ""
            )
        }
    }

    fun addAliasGroup(parentGroupId: String?, name: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedName = name.trim()
        if (normalizedName.isEmpty()) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Alias group name must not be empty.")
            return
        }
        uiState = uiState.copy(
            aliasDocumentDraft = document.addGroup(parentGroupId, normalizedName),
            aliasEditorErrorMessage = ""
        )
    }

    fun renameAliasGroup(groupId: String, name: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedName = name.trim()
        if (normalizedName.isEmpty()) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Alias group name must not be empty.")
            return
        }
        uiState = uiState.copy(
            aliasDocumentDraft = document.renameGroup(groupId, normalizedName),
            aliasEditorErrorMessage = ""
        )
    }

    fun deleteAliasGroup(groupId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        uiState = uiState.copy(
            aliasDocumentDraft = document.deleteGroup(groupId),
            aliasEditorErrorMessage = ""
        )
    }

    fun addAliasEntry(parentGroupId: String?, aliasKey: String, canonicalLeaf: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAliasKey = aliasKey.trim()
        val normalizedCanonicalLeaf = canonicalLeaf.trim()
        if (normalizedAliasKey.isEmpty() || normalizedCanonicalLeaf.isEmpty()) {
            uiState = uiState.copy(
                aliasEditorErrorMessage = "Alias key and canonical leaf must not be empty."
            )
            return
        }
        uiState = uiState.copy(
            aliasDocumentDraft = document.addEntry(
                parentGroupId = parentGroupId,
                aliasKey = normalizedAliasKey,
                canonicalLeaf = normalizedCanonicalLeaf
            ),
            aliasEditorErrorMessage = ""
        )
    }

    fun updateAliasEntry(entryId: String, aliasKey: String, canonicalLeaf: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAliasKey = aliasKey.trim()
        val normalizedCanonicalLeaf = canonicalLeaf.trim()
        if (normalizedAliasKey.isEmpty() || normalizedCanonicalLeaf.isEmpty()) {
            uiState = uiState.copy(
                aliasEditorErrorMessage = "Alias key and canonical leaf must not be empty."
            )
            return
        }
        uiState = uiState.copy(
            aliasDocumentDraft = document.updateEntry(
                entryId = entryId,
                aliasKey = normalizedAliasKey,
                canonicalLeaf = normalizedCanonicalLeaf
            ),
            aliasEditorErrorMessage = ""
        )
    }

    fun deleteAliasEntry(entryId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        uiState = uiState.copy(
            aliasDocumentDraft = document.deleteEntry(entryId),
            aliasEditorErrorMessage = ""
        )
    }

    fun setStatusText(message: String) {
        uiState = uiState.copy(statusText = message)
    }

    fun saveCurrentFile() {
        val selectedFile = uiState.selectedFilePath
        if (selectedFile.isEmpty()) {
            uiState = uiState.copy(statusText = "No TOML file selected.")
            return
        }
        viewModelScope.launch {
            if (isAliasFilePath(selectedFile)) {
                saveAliasFile(selectedFile)
                return@launch
            }
            val saveResult = configGateway.saveConfigTomlFile(
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
        if (isAliasFilePath(uiState.selectedFilePath)) {
            uiState = applyLoadedFile(
                state = uiState,
                filePath = uiState.selectedFilePath,
                content = uiState.selectedFileContent,
                aliasParentOptions = uiState.aliasParentOptions,
                statusText = uiState.statusText
            )
            return
        }
        if (uiState.editableContent == uiState.selectedFileContent) {
            return
        }
        uiState = uiState.copy(
            editableContent = uiState.selectedFileContent
        )
    }

    private suspend fun saveAliasFile(selectedFile: String) {
        when (uiState.aliasEditorMode) {
            AliasEditorMode.STRUCTURED -> {
                val document = uiState.aliasDocumentDraft
                if (document == null) {
                    uiState = uiState.copy(statusText = "Alias editor is unavailable for this file.")
                    return
                }
                val validationMessage = AliasTomlEditorCodec.validateForSave(document)
                if (validationMessage != null) {
                    uiState = uiState.copy(
                        aliasEditorErrorMessage = validationMessage,
                        statusText = validationMessage
                    )
                    return
                }
                val duplicateMessage = validateAliasKeyUniqueness(selectedFile, document)
                if (duplicateMessage != null) {
                    uiState = uiState.copy(
                        aliasEditorErrorMessage = duplicateMessage,
                        statusText = duplicateMessage
                    )
                    return
                }
                val renderedToml = AliasTomlEditorCodec.serialize(document)
                val saveResult = configGateway.saveConfigTomlFile(
                    relativePath = selectedFile,
                    content = renderedToml
                )
                uiState = if (saveResult.ok) {
                    val aliasParentOptions = resolveAliasParentOptions(
                        converterFiles = uiState.converterFiles,
                        selectedFilePath = saveResult.filePath,
                        selectedFileContent = saveResult.content
                    )
                    applyLoadedFile(
                        state = uiState.copy(aliasEditorMode = AliasEditorMode.STRUCTURED),
                        filePath = saveResult.filePath,
                        content = saveResult.content,
                        aliasParentOptions = aliasParentOptions,
                        statusText = "save toml -> ${saveResult.filePath}"
                    )
                } else {
                    uiState.copy(statusText = saveResult.message)
                }
            }

            AliasEditorMode.ADVANCED -> {
                val parseResult = AliasTomlEditorCodec.parse(uiState.aliasAdvancedTomlDraft)
                val document = parseResult.document
                if (document == null) {
                    uiState = uiState.copy(
                        aliasEditorErrorMessage = parseResult.errorMessage,
                        statusText = parseResult.errorMessage
                    )
                    return
                }
                val validationMessage = AliasTomlEditorCodec.validateForSave(document)
                if (validationMessage != null) {
                    uiState = uiState.copy(
                        aliasEditorErrorMessage = validationMessage,
                        statusText = validationMessage
                    )
                    return
                }
                val duplicateMessage = validateAliasKeyUniqueness(selectedFile, document)
                if (duplicateMessage != null) {
                    uiState = uiState.copy(
                        aliasEditorErrorMessage = duplicateMessage,
                        statusText = duplicateMessage
                    )
                    return
                }
                val saveResult = configGateway.saveConfigTomlFile(
                    relativePath = selectedFile,
                    content = uiState.aliasAdvancedTomlDraft
                )
                uiState = if (saveResult.ok) {
                    val aliasParentOptions = resolveAliasParentOptions(
                        converterFiles = uiState.converterFiles,
                        selectedFilePath = saveResult.filePath,
                        selectedFileContent = saveResult.content
                    )
                    uiState.copy(
                        selectedFileContent = saveResult.content,
                        aliasDocumentDraft = document,
                        aliasParentOptions = aliasParentOptions,
                        aliasAdvancedTomlDraft = saveResult.content,
                        aliasEditorErrorMessage = "",
                        statusText = "save toml -> ${saveResult.filePath}"
                    )
                } else {
                    uiState.copy(statusText = saveResult.message)
                }
            }
        }
    }

    private suspend fun validateAliasKeyUniqueness(
        currentFilePath: String,
        currentDocument: AliasTomlDocument
    ): String? {
        val aliasSources = linkedMapOf<String, String>()

        fun addKeys(pathLabel: String, document: AliasTomlDocument): String? {
            // Keep uniqueness semantics aligned with core alias loader: alias
            // keys are globally unique across active alias files, and group
            // path does not create a separate key namespace.
            for (aliasKey in AliasTomlEditorCodec.collectAliasKeys(document)) {
                val existing = aliasSources.putIfAbsent(aliasKey, pathLabel)
                if (existing != null) {
                    return "Duplicate alias key `$aliasKey` across alias files: $existing and $pathLabel."
                }
            }
            return null
        }

        val currentEntry = uiState.converterFiles.firstOrNull { it.relativePath == currentFilePath }
        val currentLabel = currentEntry?.displayName ?: currentFilePath
        val currentError = addKeys(currentLabel, currentDocument)
        if (currentError != null) {
            return currentError
        }

        val otherAliasFiles = uiState.converterFiles.filter { entry ->
            entry.relativePath != currentFilePath && isAliasFilePath(entry.relativePath)
        }
        for (entry in otherAliasFiles) {
            val readResult = configGateway.readConfigTomlFile(entry.relativePath)
            if (!readResult.ok) {
                return "Cannot validate alias uniqueness for ${entry.displayName}: ${readResult.message}"
            }
            val parseResult = AliasTomlEditorCodec.parse(readResult.content)
            val document = parseResult.document
                ?: return "Cannot validate alias uniqueness for ${entry.displayName}: ${parseResult.errorMessage}"
            val error = addKeys(entry.displayName, document)
            if (error != null) {
                return error
            }
        }
        return null
    }

    private fun applyLoadedFile(
        state: ConfigUiState,
        filePath: String,
        content: String,
        aliasParentOptions: List<String>,
        statusText: String
    ): ConfigUiState {
        val selectedEntry = findFileEntry(state, filePath)
        val base = state.copy(
            selectedFilePath = filePath,
            selectedFileDisplayName = selectedEntry?.displayName ?: filePath,
            selectedFileContent = content,
            statusText = statusText
        )
        if (!isAliasFilePath(filePath)) {
            return base.copy(
                editableContent = content,
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = null,
                aliasParentOptions = emptyList(),
                aliasAdvancedTomlDraft = "",
                aliasEditorErrorMessage = ""
            )
        }

        val parseResult = AliasTomlEditorCodec.parse(content)
        val document = parseResult.document
        return if (document != null) {
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = document,
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = content,
                aliasEditorErrorMessage = ""
            )
        } else {
            // Fail open to raw TOML mode so users can recover malformed alias
            // files instead of getting blocked by structured editor parsing.
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.ADVANCED,
                aliasDocumentDraft = null,
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = content,
                aliasEditorErrorMessage = parseResult.errorMessage,
                statusText = parseResult.errorMessage
            )
        }
    }

    private suspend fun resolveAliasParentOptions(
        converterFiles: List<ConfigTomlFileEntry>,
        selectedFilePath: String,
        selectedFileContent: String
    ): List<String> {
        if (!isAliasFilePath(selectedFilePath)) {
            return emptyList()
        }

        val options = linkedSetOf<String>()
        val selectedParseResult = AliasTomlEditorCodec.parse(selectedFileContent)
        selectedParseResult.document?.parent
            ?.trim()
            ?.takeIf { it.isNotEmpty() }
            ?.let(options::add)

        val aliasFiles = converterFiles.filter { entry ->
            isAliasFilePath(entry.relativePath)
        }
        for (entry in aliasFiles) {
            entry.parentCandidateFromDisplayName()
                ?.let(options::add)

            val parentValue = if (entry.relativePath == selectedFilePath) {
                selectedParseResult.document?.parent
            } else {
                val readResult = configGateway.readConfigTomlFile(entry.relativePath)
                if (!readResult.ok) {
                    null
                } else {
                    AliasTomlEditorCodec.parse(readResult.content).document?.parent
                }
            }
            parentValue
                ?.trim()
                ?.takeIf { it.isNotEmpty() }
                ?.let(options::add)
        }
        return normalizeAliasParentOptions(options)
    }

    private suspend fun resolveAliasFilePathForParent(
        parent: String,
        currentFilePath: String
    ): String? {
        val normalizedParent = parent.trim()
        if (normalizedParent.isEmpty()) {
            return null
        }

        val aliasFiles = uiState.converterFiles.filter { entry ->
            isAliasFilePath(entry.relativePath)
        }
        // Prefer real document parent values first; if a file cannot be parsed,
        // fallback by deriving candidate parent from display name.
        for (entry in aliasFiles) {
            val parentValue = if (entry.relativePath == currentFilePath) {
                uiState.aliasDocumentDraft?.parent
            } else {
                val readResult = configGateway.readConfigTomlFile(entry.relativePath)
                if (!readResult.ok) {
                    null
                } else {
                    AliasTomlEditorCodec.parse(readResult.content).document?.parent
                }
            }
            if (parentValue?.trim() == normalizedParent) {
                return entry.relativePath
            }
        }

        return aliasFiles.firstOrNull { entry ->
            entry.parentCandidateFromDisplayName() == normalizedParent
        }?.relativePath
    }

    private fun ConfigTomlFileEntry.parentCandidateFromDisplayName(): String? {
        val aliasName = displayName.removePrefix("aliases/")
        if (!aliasName.endsWith(".toml", ignoreCase = true)) {
            return null
        }
        return aliasName
            .removeSuffix(".toml")
            .trim()
            .takeIf { it.isNotEmpty() }
    }

    private fun normalizeAliasParentOptions(values: Iterable<String>): List<String> {
        // Keep parent picker options stable and easy to scan on mobile.
        return values
            .map { it.trim() }
            .filter { it.isNotEmpty() }
            .distinct()
            .sortedWith(
                compareBy<String> { it.lowercase() }
                    .thenBy { it }
            )
    }

    private fun filesForCategory(
        state: ConfigUiState,
        category: ConfigCategory
    ): List<ConfigTomlFileEntry> {
        return when (category) {
            ConfigCategory.CONVERTER -> state.converterFiles.filter { entry ->
                when (state.selectedConverterSubcategory) {
                    ConverterSubcategory.ALIASES -> entry.relativePath.startsWith("converter/aliases/")
                    ConverterSubcategory.RULES -> !entry.relativePath.startsWith("converter/aliases/")
                }
            }

            ConfigCategory.CHARTS -> state.chartFiles
            ConfigCategory.META -> state.metaFiles
            ConfigCategory.REPORTS -> state.reportFiles
        }
    }

    private fun findFileEntry(
        state: ConfigUiState,
        filePath: String
    ): ConfigTomlFileEntry? {
        return sequenceOf(
            state.converterFiles,
            state.chartFiles,
            state.metaFiles,
            state.reportFiles
        ).flatten().firstOrNull { entry -> entry.relativePath == filePath }
    }

    private fun isAliasFilePath(path: String): Boolean =
        path.startsWith("converter/aliases/") && path.endsWith(".toml", ignoreCase = true)
}

internal class ConfigViewModelFactory(private val configGateway: ConfigGateway) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(ConfigViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return ConfigViewModel(configGateway) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
