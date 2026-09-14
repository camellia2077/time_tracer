package com.example.tracer

internal fun clearSelectedConfigFile(
    state: ActivityHierarchyEditorState,
    statusText: String
): ActivityHierarchyEditorState {
    return state.copy(
        selectedFilePath = "",
        selectedFileDisplayName = "",
        selectedFileContent = "",
        aliasSearchQuery = "",
        aliasSearchDocument = null,
        aliasDocumentDraft = null,
        aliasParentOptions = emptyList(),
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
        aliasSearchQuery = if (state.selectedFilePath == filePath) {
            state.aliasSearchQuery
        } else {
            ""
        },
        aliasSearchDocument = null,
        aliasEntryMovePlan = null,
        statusText = statusText
    )
    val document = coreDocument
    val documentError = coreErrorMessage.ifBlank {
        "Activity hierarchy runtime did not return a structured snapshot."
    }
    return if (document != null) {
        base.copy(
            aliasDocumentDraft = document,
            aliasParentOptions = aliasParentOptions,
            aliasEditorErrorMessage = ""
        )
    } else {
        // The Android editor is structured-only. Keep the file content internal for
        // Core operations, but never expose malformed TOML as a raw text editor.
        base.copy(
            aliasDocumentDraft = null,
            aliasParentOptions = aliasParentOptions,
            aliasEditorErrorMessage = documentError,
            statusText = documentError
        )
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
    if (requestedName.isBlank()) {
        return null
    }
    if (requestedName == "." || requestedName == "..") {
        return null
    }
    if (requestedName.any { character ->
            character == '/' || character == '\\' || character.isISOControl()
        }
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
