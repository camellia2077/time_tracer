package com.example.tracer

internal fun switchAliasEditorToAdvanced(state: ConfigUiState): ConfigUiState = state.copy(
    aliasEditorMode = AliasEditorMode.ADVANCED,
    aliasEditorErrorMessage = ""
)

internal suspend fun validateAliasKeyUniqueness(
    configGateway: ConfigGateway,
    aliasFiles: List<ConfigTomlFileEntry>,
    currentFilePath: String,
    currentTomlContent: String
): String? {
    val gateway = configGateway as? AliasHierarchyGateway
        ?: return "Alias hierarchy runtime is unavailable."
    val documents = mutableListOf(
        AliasHierarchyDocumentInput(currentFilePath, currentTomlContent)
    )
    for (entry in aliasFiles) {
        if (entry.relativePath == currentFilePath || !isAliasConfigFilePath(entry.relativePath)) continue
        val readResult = configGateway.readConfigTomlFile(entry.relativePath)
        if (!readResult.ok) {
            return "Cannot validate alias uniqueness for ${entry.displayName}: ${readResult.message}"
        }
        documents += AliasHierarchyDocumentInput(entry.relativePath, readResult.content)
    }
    return gateway.validateAliasHierarchyDocuments(documents)
        .takeIf { !it.ok }
        ?.message
        ?.ifBlank { "Alias hierarchy validation failed." }
}

internal suspend fun resolveAliasParentOptions(
    configGateway: ConfigGateway,
    aliasFiles: List<ConfigTomlFileEntry>,
    selectedFilePath: String,
    selectedFileContent: String
): List<String> {
    if (!isAliasConfigFilePath(selectedFilePath)) return emptyList()
    val gateway = configGateway as? AliasHierarchyGateway ?: return emptyList()
    val options = linkedSetOf<String>()
    val selectedParent = gateway.describeAliasHierarchy(selectedFileContent).hierarchy?.parent
    selectedParent?.trim()?.takeIf { it.isNotEmpty() }?.let(options::add)
    for (entry in aliasFiles.filter { isAliasConfigFilePath(it.relativePath) }) {
        entry.parentCandidateFromDisplayName()?.let(options::add)
        val parent = if (entry.relativePath == selectedFilePath) selectedParent else {
            configGateway.readConfigTomlFile(entry.relativePath)
                .takeIf { it.ok }
                ?.let { gateway.describeAliasHierarchy(it.content).hierarchy?.parent }
        }
        parent?.trim()?.takeIf { it.isNotEmpty() }?.let(options::add)
    }
    return normalizeAliasParentOptions(options)
}

internal suspend fun resolveAliasFilePathForParent(
    configGateway: ConfigGateway,
    aliasFiles: List<ConfigTomlFileEntry>,
    currentFilePath: String,
    currentAliasDocument: AliasTomlDocument?,
    parent: String
): String? {
    val normalizedParent = parent.trim()
    if (normalizedParent.isEmpty()) return null
    val gateway = configGateway as? AliasHierarchyGateway
    val aliasEntries = aliasFiles.filter { isAliasConfigFilePath(it.relativePath) }
    for (entry in aliasEntries) {
        val currentParent = if (entry.relativePath == currentFilePath) {
            currentAliasDocument?.parent
        } else {
            configGateway.readConfigTomlFile(entry.relativePath)
                .takeIf { it.ok }
                ?.let { gateway?.describeAliasHierarchy(it.content)?.hierarchy?.parent }
        }
        if (currentParent?.trim() == normalizedParent) return entry.relativePath
    }
    return aliasEntries.firstOrNull {
        it.parentCandidateFromDisplayName() == normalizedParent
    }?.relativePath
}

private fun ConfigTomlFileEntry.parentCandidateFromDisplayName(): String? =
    displayName.removePrefix("aliases/")
        .takeIf { it.endsWith(".toml", ignoreCase = true) }
        ?.removeSuffix(".toml")
        ?.trim()
        ?.takeIf { it.isNotEmpty() }

internal fun normalizeAliasParentOptions(values: Iterable<String>): List<String> =
    values.map { it.trim() }
        .filter { it.isNotEmpty() }
        .distinct()
        .sortedWith(compareBy<String> { it.lowercase() }.thenBy { it })
