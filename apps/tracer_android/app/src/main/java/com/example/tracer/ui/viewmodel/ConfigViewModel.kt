package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

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

class ConfigViewModel(private val controller: RuntimeGateway) : ViewModel() {
    private val bundleTransferUseCase = ConfigBundleTransferUseCase(controller)

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

    suspend fun buildAndroidExportBundle(): ConfigExportBuildResult =
        bundleTransferUseCase.buildAndroidExportBundle()

    suspend fun importAndroidConfigBundle(
        entries: List<ConfigImportEntry>
    ): ConfigImportApplyResult =
        bundleTransferUseCase.importAndroidConfigBundle(entries)

    suspend fun importAndroidConfigPartialUpdate(
        entries: List<ConfigImportEntry>
    ): ConfigImportApplyResult =
        bundleTransferUseCase.importAndroidConfigPartialUpdate(entries)

    private fun filesForCategory(state: ConfigUiState, category: ConfigCategory): List<String> {
        return when (category) {
            ConfigCategory.CONVERTER -> state.converterFiles
            ConfigCategory.REPORTS -> state.reportFiles
        }
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
