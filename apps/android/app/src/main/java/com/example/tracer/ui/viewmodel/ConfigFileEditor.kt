package com.example.tracer

/**
 * Owns config-file browsing and plain-TOML editor transitions.
 *
 * It deliberately returns a new [ConfigUiState] instead of holding UI state,
 * so the ViewModel remains the single observable state owner.
 */
internal class ConfigFileEditor(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway
) {
    suspend fun refresh(
        state: ConfigUiState,
        showStatus: Boolean
    ): ConfigUiState {
        val loadingState = state.copy(
            statusText = if (showStatus) "refreshing config toml..." else ""
        )
        val listResult = configGateway.listConfigTomlFiles()
        if (!listResult.ok) return loadingState.copy(statusText = listResult.message)

        val updated = loadingState.copy(
            aliasFiles = listResult.aliasFiles,
            chartFiles = listResult.chartFiles,
            metaFiles = listResult.metaFiles,
            reportFiles = listResult.reportFiles
        )
        val files = configFilesForCategory(updated, updated.selectedCategory)
        val targetFile = preferredConfigFilePath(updated, files)
        if (targetFile.isEmpty()) {
            return clearSelectedConfigFile(
                updated,
                statusText = if (showStatus) listResult.message else ""
            )
        }
        return load(
            baseState = updated,
            path = targetFile,
            statusText = if (showStatus) listResult.message else ""
        )
    }

    suspend fun selectCategory(
        state: ConfigUiState,
        category: ConfigCategory
    ): ConfigUiState {
        val changed = state.copy(selectedCategory = category)
        val files = configFilesForCategory(changed, category)
        if (files.isEmpty()) {
            return clearSelectedConfigFile(
                changed,
                statusText = "No TOML files in ${category.name.lowercase()}."
            )
        }
        val targetFile = preferredConfigFilePath(changed, files)
        return load(
            baseState = changed,
            path = targetFile,
            statusText = "open toml -> $targetFile"
        )
    }

    suspend fun open(
        state: ConfigUiState,
        path: String,
        statusText: String = "open toml -> $path"
    ): ConfigUiState = load(
        baseState = state,
        path = path,
        statusText = statusText
    )

    fun updateEditableContent(
        state: ConfigUiState,
        value: String
    ): ConfigUiState {
        val selectedFile = state.selectedFilePath
        val nextDrafts = state.plainTomlDraftsByFile.toMutableMap()
        if (selectedFile.isNotBlank()) {
            if (value == state.selectedFileContent) {
                nextDrafts.remove(selectedFile)
            } else {
                nextDrafts[selectedFile] = value
            }
        }
        return state.copy(
            editableContent = value,
            plainTomlDraftsByFile = nextDrafts
        )
    }

    suspend fun savePlainTomlFile(
        state: ConfigUiState,
        selectedFile: String
    ): ConfigUiState {
        val saveResult = configGateway.saveConfigTomlFile(
            relativePath = selectedFile,
            content = state.editableContent
        )
        return if (saveResult.ok) {
            state.copy(
                selectedFileContent = state.editableContent,
                plainTomlDraftsByFile = state.plainTomlDraftsByFile - selectedFile,
                autoSaveStatus = ConfigAutoSaveStatus.SAVED,
                statusText = "save toml -> ${saveResult.filePath}"
            )
        } else {
            state.copy(
                statusText = saveResult.message,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
            )
        }
    }

    suspend fun createAliasTomlFile(
        state: ConfigUiState,
        fileName: String
    ): ConfigUiState {
        val targetFilePath = newAliasTomlPath(fileName)
            ?: return state.copy(
                statusText = "Alias file name must be a single non-empty file name."
            )
        val listResult = configGateway.listConfigTomlFiles()
        if (!listResult.ok) return state.copy(statusText = listResult.message)

        val existingPaths = sequenceOf(
            listResult.aliasFiles,
            listResult.chartFiles,
            listResult.metaFiles,
            listResult.reportFiles
        ).flatten().map { entry -> entry.relativePath }
        if (existingPaths.any { path -> path.equals(targetFilePath, ignoreCase = true) }) {
            return state.copy(statusText = "TOML file already exists: $targetFilePath")
        }

        val parent = targetFilePath.substringAfterLast('/').removeSuffix(".toml")
        val saveResult = configGateway.saveConfigTomlFile(
            relativePath = targetFilePath,
            content = newActivityHierarchyToml(parent)
        )
        if (!saveResult.ok) return state.copy(statusText = saveResult.message)

        val refreshedListResult = configGateway.listConfigTomlFiles()
        if (!refreshedListResult.ok) return state.copy(statusText = refreshedListResult.message)
        val updated = state.copy(
            aliasFiles = refreshedListResult.aliasFiles,
            chartFiles = refreshedListResult.chartFiles,
            metaFiles = refreshedListResult.metaFiles,
            reportFiles = refreshedListResult.reportFiles
        )
        return load(
            baseState = updated,
            path = targetFilePath,
            statusText = "created activity hierarchy toml -> $targetFilePath"
        )
    }

    fun discardPlainTomlDraft(state: ConfigUiState): ConfigUiState {
        val selectedFile = state.selectedFilePath
        if (selectedFile.isBlank() || state.editableContent == state.selectedFileContent) {
            return state
        }
        return state.copy(
            editableContent = state.selectedFileContent,
            plainTomlDraftsByFile = state.plainTomlDraftsByFile - selectedFile
        )
    }

    private suspend fun load(
        baseState: ConfigUiState,
        path: String,
        statusText: String
    ): ConfigUiState {
        val readResult = configGateway.readConfigTomlFile(path)
        if (!readResult.ok) return baseState.copy(statusText = readResult.message)

        val aliasParentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            activityHierarchyGateway = activityHierarchyGateway,
            aliasFiles = baseState.aliasFiles,
            selectedFilePath = readResult.filePath,
            selectedFileContent = readResult.content
        )
        val hierarchyResult = if (isAliasConfigFilePath(readResult.filePath)) {
            activityHierarchyGateway.describeActivityHierarchy(readResult.content)
        } else {
            null
        }
        return applyLoadedConfigFile(
            state = baseState,
            filePath = readResult.filePath,
            content = readResult.content,
            aliasParentOptions = aliasParentOptions,
            statusText = statusText,
            coreDocument = hierarchyResult?.hierarchy?.toActivityAliasDocument(),
            coreErrorMessage = hierarchyResult?.takeIf { !it.ok }?.message.orEmpty()
        )
    }
}
