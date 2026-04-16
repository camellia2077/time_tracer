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
            val files = configFilesForCategory(updated, updated.selectedCategory)
            val targetFile = preferredConfigFilePath(updated, files)
            if (targetFile.isEmpty()) {
                uiState = clearSelectedConfigFile(updated, statusText = listResult.message)
                return@launch
            }

            uiState = readConfigFileIntoState(
                baseState = updated,
                path = targetFile,
                statusText = listResult.message
            )
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
            val files = configFilesForCategory(changed, category)
            if (files.isEmpty()) {
                uiState = clearSelectedConfigFile(
                    changed,
                    statusText = "No TOML files in ${category.name.lowercase()}."
                )
                return@launch
            }

            uiState = readConfigFileIntoState(
                baseState = changed,
                path = preferredConfigFilePath(changed, files),
                statusText = "open toml -> ${preferredConfigFilePath(changed, files)}"
            )
        }
    }

    fun selectConverterSubcategory(subcategory: ConverterSubcategory) {
        if (uiState.selectedCategory != ConfigCategory.CONVERTER) {
            return
        }
        viewModelScope.launch {
            val changed = uiState.copy(selectedConverterSubcategory = subcategory)
            val files = configFilesForCategory(changed, ConfigCategory.CONVERTER)
            if (files.isEmpty()) {
                uiState = clearSelectedConfigFile(
                    changed,
                    statusText = "No TOML files in converter."
                )
                return@launch
            }

            uiState = readConfigFileIntoState(
                baseState = changed,
                path = preferredConfigFilePath(changed, files),
                statusText = "open toml -> ${preferredConfigFilePath(changed, files)}"
            )
        }
    }

    fun openFile(path: String) {
        val trimmedPath = path.trim()
        if (trimmedPath.isEmpty()) {
            return
        }
        viewModelScope.launch {
            uiState = readConfigFileIntoState(
                baseState = uiState.copy(
                    selectedConverterSubcategory = nextConverterSubcategoryForOpenedFile(
                        filePath = trimmedPath,
                        currentSubcategory = uiState.selectedConverterSubcategory
                    )
                ),
                path = trimmedPath,
                statusText = "open toml -> $trimmedPath"
            )
        }
    }

    fun onEditableContentChange(value: String) {
        uiState = uiState.copy(editableContent = value)
    }

    fun onAliasAdvancedTomlChange(value: String) {
        uiState = uiState.copy(aliasAdvancedTomlDraft = value)
    }

    fun selectAliasEditorMode(mode: AliasEditorMode) {
        if (!isAliasConfigFilePath(uiState.selectedFilePath) || uiState.aliasEditorMode == mode) {
            return
        }
        uiState = when (mode) {
            AliasEditorMode.ADVANCED -> switchAliasEditorToAdvanced(uiState)
            AliasEditorMode.STRUCTURED -> switchAliasEditorToStructured(uiState)
        }
    }

    fun updateAliasParent(value: String) {
        val normalizedValue = value.trim()
        if (normalizedValue.isEmpty() || !isAliasConfigFilePath(uiState.selectedFilePath)) {
            return
        }
        viewModelScope.launch {
            val currentFilePath = uiState.selectedFilePath
            val targetFilePath = resolveAliasFilePathForParent(
                configGateway = configGateway,
                converterFiles = uiState.converterFiles,
                currentFilePath = currentFilePath,
                currentAliasDocument = uiState.aliasDocumentDraft,
                parent = normalizedValue
            )
            if (targetFilePath != null && targetFilePath != currentFilePath) {
                uiState = readConfigFileIntoState(
                    baseState = uiState.copy(selectedConverterSubcategory = ConverterSubcategory.ALIASES),
                    path = targetFilePath,
                    statusText = "open toml -> $targetFilePath"
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
            if (isAliasConfigFilePath(selectedFile)) {
                saveAliasFile(selectedFile)
            } else {
                savePlainTomlFile(selectedFile)
            }
        }
    }

    fun discardUnsavedDraft() {
        if (isAliasConfigFilePath(uiState.selectedFilePath)) {
            uiState = applyLoadedConfigFile(
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
        uiState = uiState.copy(editableContent = uiState.selectedFileContent)
    }

    private suspend fun readConfigFileIntoState(
        baseState: ConfigUiState,
        path: String,
        statusText: String
    ): ConfigUiState {
        val readResult = configGateway.readConfigTomlFile(path)
        if (!readResult.ok) {
            return baseState.copy(statusText = readResult.message)
        }
        val aliasParentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            converterFiles = baseState.converterFiles,
            selectedFilePath = readResult.filePath,
            selectedFileContent = readResult.content
        )
        return applyLoadedConfigFile(
            state = baseState.copy(
                selectedConverterSubcategory = nextConverterSubcategoryForOpenedFile(
                    filePath = readResult.filePath,
                    currentSubcategory = baseState.selectedConverterSubcategory
                )
            ),
            filePath = readResult.filePath,
            content = readResult.content,
            aliasParentOptions = aliasParentOptions,
            statusText = statusText
        )
    }

    private suspend fun savePlainTomlFile(selectedFile: String) {
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

    private suspend fun saveAliasFile(selectedFile: String) {
        uiState = when (uiState.aliasEditorMode) {
            AliasEditorMode.STRUCTURED -> saveStructuredAliasFile(selectedFile)
            AliasEditorMode.ADVANCED -> saveAdvancedAliasFile(selectedFile)
        }
    }

    private suspend fun saveStructuredAliasFile(selectedFile: String): ConfigUiState {
        val document = uiState.aliasDocumentDraft
            ?: return uiState.copy(statusText = "Alias editor is unavailable for this file.")
        val validationMessage = AliasTomlEditorCodec.validateForSave(document)
        if (validationMessage != null) {
            return uiState.copy(
                aliasEditorErrorMessage = validationMessage,
                statusText = validationMessage
            )
        }
        val duplicateMessage = validateAliasKeyUniqueness(
            configGateway = configGateway,
            converterFiles = uiState.converterFiles,
            currentFilePath = selectedFile,
            currentDocument = document
        )
        if (duplicateMessage != null) {
            return uiState.copy(
                aliasEditorErrorMessage = duplicateMessage,
                statusText = duplicateMessage
            )
        }
        val renderedToml = AliasTomlEditorCodec.serialize(document)
        val saveResult = configGateway.saveConfigTomlFile(
            relativePath = selectedFile,
            content = renderedToml
        )
        if (!saveResult.ok) {
            return uiState.copy(statusText = saveResult.message)
        }
        val aliasParentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            converterFiles = uiState.converterFiles,
            selectedFilePath = saveResult.filePath,
            selectedFileContent = saveResult.content
        )
        return applyLoadedConfigFile(
            state = uiState.copy(aliasEditorMode = AliasEditorMode.STRUCTURED),
            filePath = saveResult.filePath,
            content = saveResult.content,
            aliasParentOptions = aliasParentOptions,
            statusText = "save toml -> ${saveResult.filePath}"
        )
    }

    private suspend fun saveAdvancedAliasFile(selectedFile: String): ConfigUiState {
        val parseResult = AliasTomlEditorCodec.parse(uiState.aliasAdvancedTomlDraft)
        val document = parseResult.document
            ?: return uiState.copy(
                aliasEditorErrorMessage = parseResult.errorMessage,
                statusText = parseResult.errorMessage
            )
        val validationMessage = AliasTomlEditorCodec.validateForSave(document)
        if (validationMessage != null) {
            return uiState.copy(
                aliasEditorErrorMessage = validationMessage,
                statusText = validationMessage
            )
        }
        val duplicateMessage = validateAliasKeyUniqueness(
            configGateway = configGateway,
            converterFiles = uiState.converterFiles,
            currentFilePath = selectedFile,
            currentDocument = document
        )
        if (duplicateMessage != null) {
            return uiState.copy(
                aliasEditorErrorMessage = duplicateMessage,
                statusText = duplicateMessage
            )
        }
        val saveResult = configGateway.saveConfigTomlFile(
            relativePath = selectedFile,
            content = uiState.aliasAdvancedTomlDraft
        )
        if (!saveResult.ok) {
            return uiState.copy(statusText = saveResult.message)
        }
        val aliasParentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            converterFiles = uiState.converterFiles,
            selectedFilePath = saveResult.filePath,
            selectedFileContent = saveResult.content
        )
        return uiState.copy(
            selectedFileContent = saveResult.content,
            aliasDocumentDraft = document,
            aliasParentOptions = aliasParentOptions,
            aliasAdvancedTomlDraft = saveResult.content,
            aliasEditorErrorMessage = "",
            statusText = "save toml -> ${saveResult.filePath}"
        )
    }
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
