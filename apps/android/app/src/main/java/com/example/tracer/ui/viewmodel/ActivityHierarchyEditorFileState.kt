package com.example.tracer

internal fun clearSelectedConfigFile(
    state: ActivityHierarchyEditorState,
    statusText: String
): ActivityHierarchyEditorState {
    return state.copy(
        selectedFilePath = "",
        selectedFileDisplayName = "",
        selectedFileContent = "",
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
    state: ActivityHierarchyEditorState,
    filePath: String,
    content: String,
    aliasParentOptions: List<String>,
    statusText: String,
    coreDocument: ActivityHierarchyDocument? = null,
    coreErrorMessage: String = ""
): ActivityHierarchyEditorState {
    val selectedEntry = findConfigFileEntry(state, filePath)
    val base = state.copy(
        selectedFilePath = filePath,
        selectedFileDisplayName = selectedEntry?.displayName ?: filePath,
        selectedFileContent = content,
        aliasEntryMovePlan = null,
        statusText = statusText
    )
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
    state: ActivityHierarchyEditorState,
    filePath: String
): ConfigTomlFileEntry? {
    return state.aliasFiles.firstOrNull { entry -> entry.relativePath == filePath }
}

internal fun isAliasConfigFilePath(path: String): Boolean =
    path.startsWith("user/activity_hierarchy/") &&
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
    return "user/activity_hierarchy/$normalizedFileName"
}

internal fun newActivityHierarchyToml(parent: String): String {
    val escapedParent = parent
        .replace("\\", "\\\\")
        .replace("\"", "\\\"")
    return "parent = \"$escapedParent\"\n\n[canonical]\n"
}
