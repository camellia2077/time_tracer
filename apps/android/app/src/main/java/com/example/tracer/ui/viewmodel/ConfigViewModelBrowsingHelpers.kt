package com.example.tracer

internal fun configFilesForCategory(
    state: ConfigUiState,
    category: ConfigCategory
): List<ConfigTomlFileEntry> {
    return when (category) {
        // `_system.toml` is a runtime/system file. Keep it available for an
        // explicit raw-TOML open, but do not treat it as an activity hierarchy
        // document when choosing the default file for the Alias tab.
        ConfigCategory.ALIAS -> state.aliasFiles.filter { isAliasConfigFilePath(it.relativePath) }

        ConfigCategory.CHARTS -> state.chartFiles
        ConfigCategory.META -> state.metaFiles
        ConfigCategory.REPORTS -> state.reportFiles
    }
}

internal fun preferredConfigFilePath(
    state: ConfigUiState,
    files: List<ConfigTomlFileEntry>
): String {
    return when {
        files.any { it.relativePath == state.selectedFilePath } -> state.selectedFilePath
        state.selectedCategory == ConfigCategory.ALIAS ->
            files.firstOrNull { isAliasConfigFilePath(it.relativePath) }
                ?.relativePath
                ?: files.firstOrNull()?.relativePath.orEmpty()
        files.isNotEmpty() -> files.first().relativePath
        else -> ""
    }
}

internal fun clearSelectedConfigFile(
    state: ConfigUiState,
    statusText: String
): ConfigUiState {
    return state.copy(
        selectedFilePath = "",
        selectedFileDisplayName = "",
        selectedFileContent = "",
        editableContent = "",
        aliasDocumentDraft = null,
        aliasBaselineDocument = null,
        aliasParentOptions = emptyList(),
        aliasAdvancedTomlDraft = "",
        aliasEntryMovePlan = null,
        aliasEditorErrorMessage = "",
        statusText = statusText
    )
}

internal fun applyLoadedConfigFile(
    state: ConfigUiState,
    filePath: String,
    content: String,
    aliasParentOptions: List<String>,
    statusText: String,
    coreDocument: AliasTomlDocument? = null,
    coreErrorMessage: String = ""
): ConfigUiState {
    val selectedEntry = findConfigFileEntry(state, filePath)
    val base = state.copy(
        selectedFilePath = filePath,
        selectedFileDisplayName = selectedEntry?.displayName ?: filePath,
        selectedFileContent = content,
        aliasEntryMovePlan = null,
        statusText = statusText
    )
    if (!isAliasConfigFilePath(filePath)) {
        val restoredDraft = state.plainTomlDraftsByFile[filePath] ?: content
        return base.copy(
            editableContent = restoredDraft,
            aliasEditorMode = AliasEditorMode.STRUCTURED,
            aliasDocumentDraft = null,
            aliasBaselineDocument = null,
            aliasParentOptions = emptyList(),
            aliasAdvancedTomlDraft = "",
            aliasEditorErrorMessage = ""
        )
    }

    val document = coreDocument
    val documentError = coreErrorMessage.ifBlank {
        "Activity hierarchy runtime did not return a structured snapshot."
    }
    val restoredAdvancedDraft = state.aliasAdvancedDraftsByFile[filePath] ?: content
    val restoredStructuredDraft = state.aliasStructuredDraftsByFile[filePath]
    val restoredMode = state.aliasEditorModeByFile[filePath]
    // Re-opening a config file should restore the user's in-session draft instead of snapping
    // back to the last saved content. The saved file remains the persistence source of truth; the
    // caches here only keep the editor surface stable while users browse elsewhere.
    return when {
        restoredMode == AliasEditorMode.ADVANCED -> {
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.ADVANCED,
                aliasDocumentDraft = restoredStructuredDraft ?: document,
                aliasBaselineDocument = document,
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = restoredAdvancedDraft,
                aliasEditorErrorMessage = ""
            )
        }
        restoredStructuredDraft != null -> {
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = restoredStructuredDraft,
                aliasBaselineDocument = document,
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = restoredAdvancedDraft,
                aliasEditorErrorMessage = ""
            )
        }
        document != null -> {
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = document,
                aliasBaselineDocument = document,
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = restoredAdvancedDraft,
                aliasEditorErrorMessage = ""
            )
        }
        else -> {
            // Fail open to raw TOML mode so users can recover malformed alias
            // files instead of getting blocked by structured editor parsing.
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.ADVANCED,
                aliasDocumentDraft = null,
                aliasBaselineDocument = null,
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = restoredAdvancedDraft,
                aliasEditorErrorMessage = documentError,
                statusText = documentError
            )
        }
    }
}

internal fun findConfigFileEntry(
    state: ConfigUiState,
    filePath: String
): ConfigTomlFileEntry? {
    return sequenceOf(
        state.aliasFiles,
        state.chartFiles,
        state.metaFiles,
        state.reportFiles
    ).flatten().firstOrNull { entry -> entry.relativePath == filePath }
}

internal fun isAliasConfigFilePath(path: String): Boolean =
    // `_system.toml` is an alias system config, not a structured alias file.
    path.startsWith("activity_hierarchy/") &&
        !path.endsWith("/_system.toml", ignoreCase = true) &&
        path.endsWith(".toml", ignoreCase = true)

internal fun newAliasTomlPath(
    fileName: String
): String? {
    val requestedName = fileName.trim()
    if (
        requestedName.isBlank() ||
        requestedName == "." ||
        requestedName == ".." ||
        requestedName.any { character -> character == '/' || character == '\\' || character.isISOControl() }
    ) {
        return null
    }
    val normalizedFileName = if (requestedName.endsWith(".toml", ignoreCase = true)) {
        requestedName
    } else {
        "$requestedName.toml"
    }
    return "activity_hierarchy/$normalizedFileName"
}

internal fun newActivityHierarchyToml(parent: String): String {
    val escapedParent = parent
        .replace("\\", "\\\\")
        .replace("\"", "\\\"")
    return "parent = \"$escapedParent\"\n\n[aliases]\n"
}
