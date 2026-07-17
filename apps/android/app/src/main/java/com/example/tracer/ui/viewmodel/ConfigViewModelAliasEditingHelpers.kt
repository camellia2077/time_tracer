package com.example.tracer

import android.util.Log

private const val ALIAS_RENAME_LOG_TAG = "AliasRename"

internal fun switchAliasEditorToAdvanced(state: ConfigUiState): ConfigUiState {
    val nextRawToml = state.aliasDocumentDraft?.let(AliasTomlEditorCodec::serialize)
        ?: state.aliasAdvancedTomlDraft
    return state.copy(
        aliasEditorMode = AliasEditorMode.ADVANCED,
        aliasAdvancedTomlDraft = nextRawToml,
        aliasEditorErrorMessage = ""
    )
}

internal data class AliasRenamePlan(
    val entryId: String,
    val oldAliasKey: String,
    val newAliasKey: String,
    val canonicalLeaf: String
)

internal data class TxtAliasMigrationCandidate(
    val relativePath: String,
    val originalContent: String,
    val updatedContent: String
)

internal data class QuickActivitiesAliasMigrationCandidate(
    val originalValues: List<String>,
    val updatedValues: List<String>
)

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
        // keys are globally unique across active alias files, while canonical
        // leaves are intentionally reusable. For example, both "吃饭" and
        // "饭" may map to the same canonical leaf "dining".
        // The group path does not create a separate key namespace.
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

internal fun collectAliasRenamePlans(
    baseline: AliasTomlDocument,
    current: AliasTomlDocument
): List<AliasRenamePlan> {
    val baselineEntries = baseline.entriesById()
    val currentEntries = current.entriesById()
    val renamePlans = currentEntries.values.mapNotNull { currentEntry ->
        val baselineEntry = baselineEntries[currentEntry.id] ?: return@mapNotNull null
        val oldAliasKey = baselineEntry.aliasKey.trim()
        val newAliasKey = currentEntry.aliasKey.trim()
        if (
            oldAliasKey.isEmpty() ||
            newAliasKey.isEmpty() ||
            oldAliasKey == newAliasKey ||
            baselineEntry.canonicalLeaf.trim() != currentEntry.canonicalLeaf.trim()
        ) {
            return@mapNotNull null
        }
        AliasRenamePlan(
            entryId = currentEntry.id,
            oldAliasKey = oldAliasKey,
            newAliasKey = newAliasKey,
            canonicalLeaf = currentEntry.canonicalLeaf.trim()
        )
    }
    logInfo(
        ALIAS_RENAME_LOG_TAG,
        "collectRenamePlans count=${renamePlans.size} plans=${
            renamePlans.joinToString { plan ->
                "${plan.oldAliasKey}->${plan.newAliasKey}@${plan.canonicalLeaf}"
            }
        }"
    )
    return renamePlans
}

internal suspend fun buildTxtAliasMigrationCandidates(
    txtStorageGateway: TxtStorageGateway,
    renamePlans: List<AliasRenamePlan>
): Result<List<TxtAliasMigrationCandidate>> {
    if (renamePlans.isEmpty()) {
        logInfo(ALIAS_RENAME_LOG_TAG, "buildTxtCandidates skipped because renamePlans is empty")
        return Result.success(emptyList())
    }
    val txtFilesResult = txtStorageGateway.listTxtFiles()
    if (!txtFilesResult.ok) {
        logError(ALIAS_RENAME_LOG_TAG, "listTxtFiles failed: ${txtFilesResult.message}")
        return Result.failure(IllegalStateException(txtFilesResult.message))
    }
    logInfo(
        ALIAS_RENAME_LOG_TAG,
        "buildTxtCandidates scanning fileCount=${txtFilesResult.files.size} files=${txtFilesResult.files}"
    )
    val candidates = mutableListOf<TxtAliasMigrationCandidate>()
    for (relativePath in txtFilesResult.files) {
        val readResult = txtStorageGateway.readTxtFile(relativePath)
        if (!readResult.ok) {
            logError(ALIAS_RENAME_LOG_TAG, "readTxtFile failed path=$relativePath message=${readResult.message}")
            return Result.failure(
                IllegalStateException("Cannot read TXT `$relativePath`: ${readResult.message}")
            )
        }
        val updatedContent = applyAliasRenamesToTxtContent(readResult.content, renamePlans)
        logInfo(
            ALIAS_RENAME_LOG_TAG,
            "scanTxt path=${readResult.filePath.ifBlank { relativePath }} changed=${updatedContent != readResult.content} " +
                "before=${readResult.content.toLogSnippet()} after=${updatedContent.toLogSnippet()}"
        )
        if (updatedContent != readResult.content) {
            candidates += TxtAliasMigrationCandidate(
                relativePath = readResult.filePath.ifBlank { relativePath },
                originalContent = readResult.content,
                updatedContent = updatedContent
            )
        }
    }
    logInfo(
        ALIAS_RENAME_LOG_TAG,
        "buildTxtCandidates matched=${candidates.size} paths=${candidates.map { it.relativePath }}"
    )
    return Result.success(candidates)
}

internal suspend fun writeTxtAliasMigrationCandidates(
    txtStorageGateway: TxtStorageGateway,
    candidates: List<TxtAliasMigrationCandidate>
): Result<Unit> {
    val writtenCandidates = mutableListOf<TxtAliasMigrationCandidate>()
    for (candidate in candidates) {
        logInfo(
            ALIAS_RENAME_LOG_TAG,
            "writeTxtCandidate path=${candidate.relativePath} content=${candidate.updatedContent.toLogSnippet()}"
        )
        val saveResult = txtStorageGateway.saveTxtFile(
            relativePath = candidate.relativePath,
            content = candidate.updatedContent
        )
        if (!saveResult.ok) {
            logError(
                ALIAS_RENAME_LOG_TAG,
                "saveTxtFile failed path=${candidate.relativePath} message=${saveResult.message}"
            )
            rollbackTxtAliasMigrationCandidates(txtStorageGateway, writtenCandidates)
            return Result.failure(
                IllegalStateException(
                    "Cannot save TXT `${candidate.relativePath}`: ${saveResult.message}"
                )
            )
        }
        logInfo(
            ALIAS_RENAME_LOG_TAG,
            "saveTxtFile ok path=${candidate.relativePath} returnedPath=${saveResult.filePath}"
        )
        writtenCandidates += candidate
    }
    return Result.success(Unit)
}

internal fun buildQuickActivitiesAliasMigrationCandidate(
    quickActivities: List<String>,
    renamePlans: List<AliasRenamePlan>
): QuickActivitiesAliasMigrationCandidate? {
    if (renamePlans.isEmpty() || quickActivities.isEmpty()) {
        logInfo(
            ALIAS_RENAME_LOG_TAG,
            "buildQuickAccessCandidate skipped renamePlanCount=${renamePlans.size} quickActivityCount=${quickActivities.size}"
        )
        return null
    }
    val renameMap = renamePlans.associate { plan ->
        plan.oldAliasKey to plan.newAliasKey
    }
    val updatedValues = quickActivities.map { activity ->
        renameMap[activity] ?: activity
    }
    if (updatedValues == quickActivities) {
        logInfo(ALIAS_RENAME_LOG_TAG, "buildQuickAccessCandidate no matching quick activities")
        return null
    }
    logInfo(
        ALIAS_RENAME_LOG_TAG,
        "buildQuickAccessCandidate updated from=$quickActivities to=$updatedValues"
    )
    return QuickActivitiesAliasMigrationCandidate(
        originalValues = quickActivities,
        updatedValues = updatedValues
    )
}

internal suspend fun rollbackTxtAliasMigrationCandidates(
    txtStorageGateway: TxtStorageGateway,
    candidates: List<TxtAliasMigrationCandidate>
): List<String> {
    val rollbackErrors = mutableListOf<String>()
    for (candidate in candidates.asReversed()) {
        logWarn(ALIAS_RENAME_LOG_TAG, "rollbackTxt path=${candidate.relativePath}")
        val rollbackResult = txtStorageGateway.saveTxtFile(
            relativePath = candidate.relativePath,
            content = candidate.originalContent
        )
        if (!rollbackResult.ok) {
            rollbackErrors += "${candidate.relativePath}: ${rollbackResult.message}"
        }
    }
    return rollbackErrors
}

private fun applyAliasRenamesToTxtContent(
    content: String,
    renamePlans: List<AliasRenamePlan>
): String {
    var updatedContent = content
    for (plan in renamePlans) {
        updatedContent = updatedContent.replaceAliasToken(
            oldAliasKey = plan.oldAliasKey,
            newAliasKey = plan.newAliasKey,
            canonicalLeaf = plan.canonicalLeaf
        )
    }
    return updatedContent
}

private fun String.replaceAliasToken(
    oldAliasKey: String,
    newAliasKey: String,
    canonicalLeaf: String
): String {
    val lines = lineSequence().toList()
    val updatedLines = lines.map { line ->
        val activityRange = line.findTxtActivityRange() ?: return@map line
        val activityToken = line.substring(activityRange)
        if (activityToken != oldAliasKey) {
            return@map line
        }
        logInfo(
            ALIAS_RENAME_LOG_TAG,
            "replaceLine canonical=$canonicalLeaf old=$oldAliasKey new=$newAliasKey line=${line.toLogSnippet()}"
        )
        line.replaceRange(activityRange, newAliasKey)
    }
    return updatedLines.joinToString(separator = "\n")
}

private fun logInfo(tag: String, message: String) {
    runCatching { Log.i(tag, message) }
}

private fun logWarn(tag: String, message: String) {
    runCatching { Log.w(tag, message) }
}

private fun logError(tag: String, message: String) {
    runCatching { Log.e(tag, message) }
}

private fun String.toLogSnippet(maxLength: Int = 160): String {
    val sanitized = replace("\r", "\\r").replace("\n", "\\n")
    return if (sanitized.length <= maxLength) {
        sanitized
    } else {
        sanitized.take(maxLength) + "...(${sanitized.length})"
    }
}

private fun String.findTxtActivityRange(): IntRange? {
    val trimmedStart = indexOfFirst { character -> !character.isWhitespace() }
    if (trimmedStart < 0 || length - trimmedStart < 4) {
        return null
    }
    val firstTime = substring(trimmedStart, trimmedStart + 4)
    if (!firstTime.isValidHhmmToken()) {
        return null
    }

    val prefixEndExclusive = if (
        length - trimmedStart >= 9 &&
        this[trimmedStart + 4] == '-' &&
        substring(trimmedStart + 5, trimmedStart + 9).isValidHhmmToken()
    ) {
        trimmedStart + 9
    } else {
        trimmedStart + 4
    }

    val activityStart = (prefixEndExclusive until length)
        .firstOrNull { index -> !this[index].isWhitespace() }
        ?: return null
    val commentStart = findTxtCommentStart(startIndex = activityStart)
    var activityEndExclusive = if (commentStart >= 0) {
        commentStart
    } else {
        length
    }
    while (activityEndExclusive > activityStart && this[activityEndExclusive - 1].isWhitespace()) {
        activityEndExclusive--
    }
    if (activityEndExclusive <= activityStart) {
        return null
    }
    return activityStart until activityEndExclusive
}

private fun String.findTxtCommentStart(startIndex: Int): Int {
    var earliest = -1
    val separators = listOf("//", "#", ";")
    for (separator in separators) {
        val index = indexOf(separator, startIndex = startIndex)
        if (index >= 0 && (earliest < 0 || index < earliest)) {
            earliest = index
        }
    }
    return earliest
}

private fun String.isValidHhmmToken(): Boolean {
    if (length != 4 || any { character -> !character.isDigit() }) {
        return false
    }
    val hours = substring(0, 2).toIntOrNull() ?: return false
    val minutes = substring(2, 4).toIntOrNull() ?: return false
    return hours in 0..23 && minutes in 0..59
}

private fun AliasTomlDocument.entriesById(): Map<String, AliasTomlEntry> = buildMap {
    collectEntriesById(nodes, this)
}

private fun collectEntriesById(
    nodes: List<AliasTomlNode>,
    sink: MutableMap<String, AliasTomlEntry>
) {
    for (node in nodes) {
        when (node) {
            is AliasTomlEntry -> sink[node.id] = node
            is AliasTomlGroup -> collectEntriesById(node.nodes, sink)
        }
    }
}
