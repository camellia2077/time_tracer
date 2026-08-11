package com.example.tracer

internal fun switchAliasEditorToAdvanced(state: ActivityHierarchyEditorState): ActivityHierarchyEditorState = state.copy(
    aliasEditorMode = AliasEditorMode.ADVANCED,
    aliasEditorErrorMessage = ""
)

internal suspend fun validateAliasKeyUniqueness(
    configGateway: ConfigGateway,
    activityHierarchyGateway: ActivityHierarchyGateway,
    aliasFiles: List<ConfigTomlFileEntry>,
    currentFilePath: String,
    currentTomlContent: String
): String? {
    val documents = mutableListOf(
        ActivityHierarchyDocumentInput(currentFilePath, currentTomlContent)
    )
    for (entry in aliasFiles) {
        if (entry.relativePath == currentFilePath || !isAliasConfigFilePath(entry.relativePath)) continue
        val readResult = configGateway.readConfigTomlFile(entry.relativePath)
        if (!readResult.ok) {
            return "Cannot validate alias uniqueness for ${entry.displayName}: ${readResult.message}"
        }
        documents += ActivityHierarchyDocumentInput(entry.relativePath, readResult.content)
    }
    return activityHierarchyGateway.validateActivityHierarchyDocuments(documents)
        .takeIf { !it.ok }
        ?.message
        ?.ifBlank { "Activity hierarchy validation failed." }
}

internal suspend fun resolveAliasParentOptions(
    configGateway: ConfigGateway,
    activityHierarchyGateway: ActivityHierarchyGateway,
    aliasFiles: List<ConfigTomlFileEntry>,
    selectedFilePath: String,
    selectedFileContent: String
): List<String> {
    if (!isAliasConfigFilePath(selectedFilePath)) return emptyList()
    val options = linkedSetOf<String>()
    val selectedParent = activityHierarchyGateway.describeActivityHierarchy(selectedFileContent).hierarchy?.parent
    selectedParent?.trim()?.takeIf { it.isNotEmpty() }?.let(options::add)
    for (entry in aliasFiles.filter { isAliasConfigFilePath(it.relativePath) }) {
        entry.parentCandidateFromDisplayName()?.let(options::add)
        val parent = if (entry.relativePath == selectedFilePath) selectedParent else {
            configGateway.readConfigTomlFile(entry.relativePath)
                .takeIf { it.ok }
                ?.let { activityHierarchyGateway.describeActivityHierarchy(it.content).hierarchy?.parent }
        }
        parent?.trim()?.takeIf { it.isNotEmpty() }?.let(options::add)
    }
    return normalizeAliasParentOptions(options)
}

internal suspend fun resolveAliasFilePathForParent(
    configGateway: ConfigGateway,
    activityHierarchyGateway: ActivityHierarchyGateway,
    aliasFiles: List<ConfigTomlFileEntry>,
    currentFilePath: String,
    currentActivityHierarchyDocument: ActivityHierarchyDocument?,
    parent: String
): String? {
    val normalizedParent = parent.trim()
    if (normalizedParent.isEmpty()) return null
    val aliasEntries = aliasFiles.filter { isAliasConfigFilePath(it.relativePath) }
    for (entry in aliasEntries) {
        val currentParent = if (entry.relativePath == currentFilePath) {
            currentActivityHierarchyDocument?.parent
        } else {
            configGateway.readConfigTomlFile(entry.relativePath)
                .takeIf { it.ok }
                ?.let { activityHierarchyGateway.describeActivityHierarchy(it.content).hierarchy?.parent }
        }
        if (currentParent?.trim() == normalizedParent) return entry.relativePath
    }
    return aliasEntries.firstOrNull {
        it.parentCandidateFromDisplayName() == normalizedParent
    }?.relativePath
}

private fun ConfigTomlFileEntry.parentCandidateFromDisplayName(): String? =
    displayName.removePrefix("user/activity_hierarchy/")
        .takeIf { it.endsWith(".toml", ignoreCase = true) }
        ?.removeSuffix(".toml")
        ?.trim()
        ?.takeIf { it.isNotEmpty() }

internal fun normalizeAliasParentOptions(values: Iterable<String>): List<String> =
    values.map { it.trim() }
        .filter { it.isNotEmpty() }
        .distinct()
        .sortedWith(compareBy<String> { it.lowercase() }.thenBy { it })
