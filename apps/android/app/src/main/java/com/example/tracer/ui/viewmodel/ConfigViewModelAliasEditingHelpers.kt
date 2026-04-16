package com.example.tracer

internal fun switchAliasEditorToAdvanced(state: ConfigUiState): ConfigUiState {
    val nextRawToml = state.aliasDocumentDraft?.let(AliasTomlEditorCodec::serialize)
        ?: state.aliasAdvancedTomlDraft
    return state.copy(
        aliasEditorMode = AliasEditorMode.ADVANCED,
        aliasAdvancedTomlDraft = nextRawToml,
        aliasEditorErrorMessage = ""
    )
}

internal fun switchAliasEditorToStructured(state: ConfigUiState): ConfigUiState {
    val parseResult = AliasTomlEditorCodec.parse(state.aliasAdvancedTomlDraft)
    val document = parseResult.document
        ?: return state.copy(
            aliasEditorErrorMessage = parseResult.errorMessage,
            statusText = parseResult.errorMessage
        )
    return state.copy(
        aliasEditorMode = AliasEditorMode.STRUCTURED,
        aliasDocumentDraft = document,
        aliasParentOptions = normalizeAliasParentOptions(
            state.aliasParentOptions + document.parent
        ),
        aliasEditorErrorMessage = ""
    )
}

internal suspend fun validateAliasKeyUniqueness(
    configGateway: ConfigGateway,
    converterFiles: List<ConfigTomlFileEntry>,
    currentFilePath: String,
    currentDocument: AliasTomlDocument
): String? {
    val aliasSources = linkedMapOf<String, String>()

    fun addKeys(pathLabel: String, document: AliasTomlDocument): String? {
        // Keep uniqueness semantics aligned with core alias loader: alias
        // keys are globally unique across active alias files, and group
        // path does not create a separate key namespace.
        for (aliasKey in AliasTomlEditorCodec.collectAliasKeys(document)) {
            val existing = aliasSources.putIfAbsent(aliasKey, pathLabel)
            if (existing != null) {
                return "Duplicate alias key `$aliasKey` across alias files: $existing and $pathLabel."
            }
        }
        return null
    }

    val currentEntry = converterFiles.firstOrNull { it.relativePath == currentFilePath }
    val currentLabel = currentEntry?.displayName ?: currentFilePath
    val currentError = addKeys(currentLabel, currentDocument)
    if (currentError != null) {
        return currentError
    }

    val otherAliasFiles = converterFiles.filter { entry ->
        entry.relativePath != currentFilePath && isAliasConfigFilePath(entry.relativePath)
    }
    for (entry in otherAliasFiles) {
        val readResult = configGateway.readConfigTomlFile(entry.relativePath)
        if (!readResult.ok) {
            return "Cannot validate alias uniqueness for ${entry.displayName}: ${readResult.message}"
        }
        val parseResult = AliasTomlEditorCodec.parse(readResult.content)
        val document = parseResult.document
            ?: return "Cannot validate alias uniqueness for ${entry.displayName}: ${parseResult.errorMessage}"
        val error = addKeys(entry.displayName, document)
        if (error != null) {
            return error
        }
    }
    return null
}

internal suspend fun resolveAliasParentOptions(
    configGateway: ConfigGateway,
    converterFiles: List<ConfigTomlFileEntry>,
    selectedFilePath: String,
    selectedFileContent: String
): List<String> {
    if (!isAliasConfigFilePath(selectedFilePath)) {
        return emptyList()
    }

    val options = linkedSetOf<String>()
    val selectedParseResult = AliasTomlEditorCodec.parse(selectedFileContent)
    selectedParseResult.document?.parent
        ?.trim()
        ?.takeIf { it.isNotEmpty() }
        ?.let(options::add)

    val aliasFiles = converterFiles.filter { entry ->
        isAliasConfigFilePath(entry.relativePath)
    }
    for (entry in aliasFiles) {
        entry.parentCandidateFromDisplayName()
            ?.let(options::add)

        val parentValue = if (entry.relativePath == selectedFilePath) {
            selectedParseResult.document?.parent
        } else {
            val readResult = configGateway.readConfigTomlFile(entry.relativePath)
            if (!readResult.ok) {
                null
            } else {
                AliasTomlEditorCodec.parse(readResult.content).document?.parent
            }
        }
        parentValue
            ?.trim()
            ?.takeIf { it.isNotEmpty() }
            ?.let(options::add)
    }
    return normalizeAliasParentOptions(options)
}

internal suspend fun resolveAliasFilePathForParent(
    configGateway: ConfigGateway,
    converterFiles: List<ConfigTomlFileEntry>,
    currentFilePath: String,
    currentAliasDocument: AliasTomlDocument?,
    parent: String
): String? {
    val normalizedParent = parent.trim()
    if (normalizedParent.isEmpty()) {
        return null
    }

    val aliasFiles = converterFiles.filter { entry ->
        isAliasConfigFilePath(entry.relativePath)
    }
    // Prefer real document parent values first; if a file cannot be parsed,
    // fallback by deriving candidate parent from display name.
    for (entry in aliasFiles) {
        val parentValue = if (entry.relativePath == currentFilePath) {
            currentAliasDocument?.parent
        } else {
            val readResult = configGateway.readConfigTomlFile(entry.relativePath)
            if (!readResult.ok) {
                null
            } else {
                AliasTomlEditorCodec.parse(readResult.content).document?.parent
            }
        }
        if (parentValue?.trim() == normalizedParent) {
            return entry.relativePath
        }
    }

    return aliasFiles.firstOrNull { entry ->
        entry.parentCandidateFromDisplayName() == normalizedParent
    }?.relativePath
}

private fun ConfigTomlFileEntry.parentCandidateFromDisplayName(): String? {
    val aliasName = displayName.removePrefix("aliases/")
    if (!aliasName.endsWith(".toml", ignoreCase = true)) {
        return null
    }
    return aliasName
        .removeSuffix(".toml")
        .trim()
        .takeIf { it.isNotEmpty() }
}

private fun normalizeAliasParentOptions(values: Iterable<String>): List<String> {
    // Keep parent picker options stable and easy to scan on mobile.
    return values
        .map { it.trim() }
        .filter { it.isNotEmpty() }
        .distinct()
        .sortedWith(
            compareBy<String> { it.lowercase() }
                .thenBy { it }
        )
}
