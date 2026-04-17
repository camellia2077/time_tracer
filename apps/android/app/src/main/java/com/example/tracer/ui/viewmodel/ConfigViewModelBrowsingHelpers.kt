package com.example.tracer

internal fun configFilesForCategory(
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

internal fun preferredConfigFilePath(
    state: ConfigUiState,
    files: List<ConfigTomlFileEntry>
): String {
    return when {
        files.any { it.relativePath == state.selectedFilePath } -> state.selectedFilePath
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
        aliasParentOptions = emptyList(),
        aliasAdvancedTomlDraft = "",
        aliasEditorErrorMessage = "",
        statusText = statusText
    )
}

internal fun nextConverterSubcategoryForOpenedFile(
    filePath: String,
    currentSubcategory: ConverterSubcategory
): ConverterSubcategory {
    return when {
        filePath.startsWith("converter/aliases/") -> ConverterSubcategory.ALIASES
        filePath.startsWith("converter/") -> ConverterSubcategory.RULES
        else -> currentSubcategory
    }
}

internal fun applyLoadedConfigFile(
    state: ConfigUiState,
    filePath: String,
    content: String,
    aliasParentOptions: List<String>,
    statusText: String
): ConfigUiState {
    val selectedEntry = findConfigFileEntry(state, filePath)
    val base = state.copy(
        selectedFilePath = filePath,
        selectedFileDisplayName = selectedEntry?.displayName ?: filePath,
        selectedFileContent = content,
        statusText = statusText
    )
    if (!isAliasConfigFilePath(filePath)) {
        val restoredDraft = state.plainTomlDraftsByFile[filePath] ?: content
        return base.copy(
            editableContent = restoredDraft,
            aliasEditorMode = AliasEditorMode.STRUCTURED,
            aliasDocumentDraft = null,
            aliasParentOptions = emptyList(),
            aliasAdvancedTomlDraft = "",
            aliasEditorErrorMessage = ""
        )
    }

    val parseResult = AliasTomlEditorCodec.parse(content)
    val document = parseResult.document
    val restoredAdvancedDraft = state.aliasAdvancedDraftsByFile[filePath] ?: content
    val restoredStructuredDraft = state.aliasStructuredDraftsByFile[filePath]
    val restoredMode = state.aliasEditorModeByFile[filePath]
    // Re-opening a config file should restore the user's in-session draft instead of snapping
    // back to the last saved content. The saved file remains the persistence source of truth; the
    // caches here only keep the editor surface stable while users browse elsewhere.
    return when {
        restoredMode == AliasEditorMode.ADVANCED -> {
            val advancedParseResult = AliasTomlEditorCodec.parse(restoredAdvancedDraft)
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.ADVANCED,
                aliasDocumentDraft = restoredStructuredDraft ?: advancedParseResult.document,
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = restoredAdvancedDraft,
                aliasEditorErrorMessage = advancedParseResult.errorMessage
            )
        }
        restoredStructuredDraft != null -> {
            base.copy(
                editableContent = "",
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = restoredStructuredDraft,
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
                aliasParentOptions = aliasParentOptions,
                aliasAdvancedTomlDraft = restoredAdvancedDraft,
                aliasEditorErrorMessage = parseResult.errorMessage,
                statusText = parseResult.errorMessage
            )
        }
    }
}

internal fun findConfigFileEntry(
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

internal fun isAliasConfigFilePath(path: String): Boolean =
    path.startsWith("converter/aliases/") && path.endsWith(".toml", ignoreCase = true)
