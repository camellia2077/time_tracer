package com.example.tracer

/**
 * Owns config-file browsing and plain-TOML editor transitions.
 *
 * It deliberately returns a new [ActivityHierarchyEditorState] instead of holding UI state,
 * so the ViewModel remains the single observable state owner.
 */
internal class ActivityHierarchyFileEditor(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway
) {
    suspend fun openActivityCategories(state: ActivityHierarchyEditorState): ActivityHierarchyEditorState {
        val listResult = configGateway.listConfigTomlFiles()
        if (!listResult.ok) return state.copy(statusText = listResult.message)
        val updated = state.copy(aliasFiles = listResult.aliasFiles)
        val targetFile = updated.aliasFiles
            .firstOrNull { isAliasConfigFilePath(it.relativePath) }
            ?.relativePath
            .orEmpty()
        if (targetFile.isEmpty()) {
            return clearSelectedConfigFile(updated, "No activity hierarchy TOML files.")
        }
        return load(updated, targetFile, "open activity categories -> $targetFile")
    }

    suspend fun open(
        state: ActivityHierarchyEditorState,
        path: String,
        statusText: String = "open toml -> $path"
    ): ActivityHierarchyEditorState = load(
        baseState = state,
        path = path,
        statusText = statusText
    )

    suspend fun createAliasTomlFile(
        state: ActivityHierarchyEditorState,
        fileName: String
    ): ActivityHierarchyEditorState {
        val targetFilePath = newAliasTomlPath(fileName)
            ?: return state.copy(
                statusText = "Canonical file name must be a single non-empty file name."
            )
        val listResult = configGateway.listConfigTomlFiles()
        if (!listResult.ok) return state.copy(statusText = listResult.message)

        val existingPaths = listResult.aliasFiles.map { entry -> entry.relativePath }
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
        val updated = state.copy(aliasFiles = refreshedListResult.aliasFiles)
        return load(
            baseState = updated,
            path = targetFilePath,
            statusText = "created activity hierarchy toml -> $targetFilePath"
        )
    }

    private suspend fun load(
        baseState: ActivityHierarchyEditorState,
        path: String,
        statusText: String
    ): ActivityHierarchyEditorState {
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
            coreDocument = hierarchyResult?.hierarchy?.toActivityHierarchyDocument(),
            coreErrorMessage = hierarchyResult?.takeIf { !it.ok }?.message.orEmpty()
        )
    }
}
