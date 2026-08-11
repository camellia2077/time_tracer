package com.example.tracer

internal sealed interface ActivityHierarchyMovePreviewOutcome {
    data class Ready(val plan: AliasEntryMovePlan) : ActivityHierarchyMovePreviewOutcome
    data class Failed(val message: String) : ActivityHierarchyMovePreviewOutcome
}

internal sealed interface ActivityHierarchyMoveDestinationsOutcome {
    data class Ready(val documents: List<AliasEntryMoveDestinationDocument>) :
        ActivityHierarchyMoveDestinationsOutcome

    data class Failed(val message: String) : ActivityHierarchyMoveDestinationsOutcome
}

private sealed interface MoveDocumentsReadOutcome {
    data class Ready(val documents: List<ActivityHierarchyDocumentInput>) : MoveDocumentsReadOutcome
    data class Failed(val message: String) : MoveDocumentsReadOutcome
}

private sealed interface MoveContentReadOutcome {
    data class Ready(val content: String) : MoveContentReadOutcome
    data class Failed(val message: String) : MoveContentReadOutcome
}

/**
 * Coordinates Core-only move previews. It does not persist anything and does
 * not own UI state; persistence remains a separate migration step.
 */
internal class ActivityHierarchyMoveCoordinator(
    private val configGateway: ConfigGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway
) {
    suspend fun prepareDestinations(
        state: ActivityHierarchyEditorState,
        sourcePath: String,
        excludedGroupPath: List<String>,
        excludeDescendants: Boolean,
        onlyCurrentDocument: Boolean = false
    ): ActivityHierarchyMoveDestinationsOutcome {
        val documents = mutableListOf<AliasEntryMoveDestinationDocument>()
        val files = state.aliasFiles
            .filter { isAliasConfigFilePath(it.relativePath) }
            .filter { !onlyCurrentDocument || it.relativePath == sourcePath }
        for (file in files) {
            val content = when (val read = readContent(state, file.relativePath, sourcePath)) {
                is MoveContentReadOutcome.Ready -> read.content
                is MoveContentReadOutcome.Failed -> return ActivityHierarchyMoveDestinationsOutcome.Failed(
                    read.message.ifBlank { "Cannot read activity hierarchy: ${file.displayName}" }
                )
            }
            val described = activityHierarchyGateway.describeActivityHierarchy(content)
            val document = described.hierarchy?.toActivityHierarchyDocument()
            if (!described.ok || document == null) {
                return ActivityHierarchyMoveDestinationsOutcome.Failed(
                    described.message.ifBlank {
                        "Cannot read activity hierarchy: ${file.displayName}"
                    }
                )
            }
            documents += AliasEntryMoveDestinationDocument(
                sourceName = file.relativePath,
                displayName = file.displayName.removePrefix("user/activity_hierarchy/"),
                document = document,
                rootSelectable = file.relativePath != sourcePath,
                excludedGroupPath = if (file.relativePath == sourcePath) {
                    excludedGroupPath
                } else {
                    emptyList()
                },
                excludeDescendants = file.relativePath == sourcePath && excludeDescendants
            )
        }
        return ActivityHierarchyMoveDestinationsOutcome.Ready(documents)
    }

    suspend fun previewEntryMove(
        state: ActivityHierarchyEditorState,
        entryId: String,
        targetGroupId: String
    ): ActivityHierarchyMovePreviewOutcome {
        val document = state.aliasDocumentDraft ?: return failure("Canonical document is unavailable.")
        val entry = document.findAliasEntry(entryId) ?: return failure("Alias entry is unavailable.")
        val sourcePath = document.canonicalTargetPathForEntry(entryId)
            ?: return failure("Alias entry path is unavailable.")
        val destinationPath = document.canonicalTargetPathForGroup(targetGroupId)
            ?: return failure("Canonical destination group is unavailable.")
        val content = currentContent(state)
        val result = activityHierarchyGateway.applyActivityHierarchyOperation(
            content,
            ActivityHierarchyOperation(
                kind = ActivityHierarchyOperationKind.MOVE_LEAF,
                targetPath = sourcePath,
                destinationPath = destinationPath
            )
        )
        val replacement = result.replacementPlan.canonical.singleOrNull()
            ?: return failure(result.message.ifBlank { "Alias move preview failed." })
        if (!result.ok) return failure(result.message.ifBlank { "Alias move preview failed." })
        return ActivityHierarchyMovePreviewOutcome.Ready(
            AliasEntryMovePlan(
                entryId = entryId,
                aliasKey = entry.aliasKey,
                canonicalLeaf = entry.canonicalLeaf,
                sourceParentGroupId = sourcePath.substringBeforeLast('.', "").ifBlank { null },
                sourceGroupPath = sourcePath.substringBeforeLast('.', "")
                    .split('.')
                    .filter(String::isNotEmpty),
                targetGroupId = targetGroupId,
                targetGroupPath = destinationPath.removePrefix("root.")
                    .split('.')
                    .filter(String::isNotEmpty),
                oldCanonical = replacement.oldCanonical,
                newCanonical = replacement.newCanonical
            )
        )
    }

    suspend fun previewEntryMove(
        state: ActivityHierarchyEditorState,
        entryId: String,
        target: AliasEntryMoveTarget
    ): ActivityHierarchyMovePreviewOutcome {
        val document = state.aliasDocumentDraft ?: return failure("Canonical document is unavailable.")
        val entry = document.findAliasEntry(entryId) ?: return failure("Alias entry is unavailable.")
        val sourcePath = state.selectedFilePath
        val sourceCanonicalPath = document.canonicalTargetPathForEntry(entryId)
            ?: return failure("Alias entry path is unavailable.")
        val documents = when (val read = readDocuments(state, sourcePath)) {
            is MoveDocumentsReadOutcome.Ready -> read.documents
            is MoveDocumentsReadOutcome.Failed -> return failure(read.message)
        }
        val result = activityHierarchyGateway.moveActivityHierarchyNodeBetweenDocuments(
            documents = documents,
            sourceName = sourcePath,
            destinationName = target.sourceName,
            operation = ActivityHierarchyOperation(
                kind = ActivityHierarchyOperationKind.MOVE_LEAF,
                targetPath = sourceCanonicalPath,
                destinationPath = target.groupPath.joinToString(".").ifBlank { "root" }
            )
        )
        val replacement = result.replacementPlan.canonical.firstOrNull()
        if (!result.ok || replacement == null || result.updatedDocuments.isEmpty()) {
            return failure(result.message.ifBlank { "Alias move preview failed." })
        }
        return ActivityHierarchyMovePreviewOutcome.Ready(
            AliasEntryMovePlan(
                entryId = entryId,
                aliasKey = entry.aliasKey,
                canonicalLeaf = entry.canonicalLeaf,
                sourceParentGroupId = sourceCanonicalPath.substringBeforeLast('.', "").ifBlank { null },
                sourceGroupPath = sourceCanonicalPath.split('.').dropLast(1),
                targetGroupId = target.groupId.orEmpty(),
                targetGroupPath = target.groupPath,
                oldCanonical = replacement.oldCanonical,
                newCanonical = replacement.newCanonical,
                sourceFilePath = sourcePath,
                destinationFilePath = target.sourceName,
                destinationGroupPath = target.groupPath,
                updatedDocuments = result.updatedDocuments,
                replacementPlan = result.replacementPlan
            )
        )
    }

    suspend fun previewGroupMove(
        state: ActivityHierarchyEditorState,
        groupId: String,
        target: AliasEntryMoveTarget
    ): ActivityHierarchyMovePreviewOutcome {
        val document = state.aliasDocumentDraft ?: return failure("Canonical document is unavailable.")
        val group = document.findAliasGroup(groupId) ?: return failure("Alias group is unavailable.")
        val sourcePath = state.selectedFilePath
        val sourceCanonicalPath = document.canonicalTargetPathForGroup(groupId)
            ?: return failure("Alias group path is unavailable.")
        val operation = ActivityHierarchyOperation(
            kind = ActivityHierarchyOperationKind.MOVE_GROUP,
            targetPath = sourceCanonicalPath,
            destinationPath = target.groupPath.joinToString(".").ifBlank { "root" }
        )
        val result = if (target.sourceName == sourcePath) {
            activityHierarchyGateway.applyActivityHierarchyOperation(currentContent(state), operation).let {
                ActivityHierarchyMovePreviewResult(
                    ok = it.ok,
                    replacementPlan = it.replacementPlan,
                    updatedDocuments = emptyList(),
                    message = it.message
                )
            }
        } else {
            val documents = when (val read = readDocuments(state, sourcePath)) {
                is MoveDocumentsReadOutcome.Ready -> read.documents
                is MoveDocumentsReadOutcome.Failed -> return failure(read.message)
            }
            activityHierarchyGateway.moveActivityHierarchyNodeBetweenDocuments(
                documents = documents,
                sourceName = sourcePath,
                destinationName = target.sourceName,
                operation = operation
            ).let {
                ActivityHierarchyMovePreviewResult(
                    ok = it.ok,
                    replacementPlan = it.replacementPlan,
                    updatedDocuments = it.updatedDocuments,
                    message = it.message
                )
            }
        }
        val replacement = result.replacementPlan.canonical.firstOrNull()
        val missingCrossDocumentResult = target.sourceName != sourcePath && result.updatedDocuments.isEmpty()
        if (!result.ok || replacement == null || missingCrossDocumentResult) {
            return failure(result.message.ifBlank { "Group move preview failed." })
        }
        return ActivityHierarchyMovePreviewOutcome.Ready(
            AliasEntryMovePlan(
                entryId = groupId,
                aliasKey = group.name,
                canonicalLeaf = group.name,
                nodeKind = AliasMoveNodeKind.GROUP,
                sourceParentGroupId = sourceCanonicalPath.substringBeforeLast('.', "").ifBlank { null },
                sourceGroupPath = sourceCanonicalPath.split('.').dropLast(1),
                targetGroupId = target.groupId.orEmpty(),
                targetGroupPath = target.groupPath,
                oldCanonical = replacement.oldCanonical,
                newCanonical = replacement.newCanonical,
                sourceFilePath = sourcePath,
                destinationFilePath = target.sourceName,
                destinationGroupPath = target.groupPath,
                updatedDocuments = result.updatedDocuments,
                replacementPlan = result.replacementPlan
            )
        )
    }

    private fun currentContent(state: ActivityHierarchyEditorState): String =
        state.aliasAdvancedTomlDraft.ifBlank { state.selectedFileContent }

    private suspend fun readDocuments(
        state: ActivityHierarchyEditorState,
        currentPath: String
    ): MoveDocumentsReadOutcome {
        val documents = mutableListOf<ActivityHierarchyDocumentInput>()
        for (file in state.aliasFiles.filter { isAliasConfigFilePath(it.relativePath) }) {
            when (val read = readContent(state, file.relativePath, currentPath)) {
                is MoveContentReadOutcome.Ready -> documents += ActivityHierarchyDocumentInput(
                    file.relativePath,
                    read.content
                )
                is MoveContentReadOutcome.Failed -> return MoveDocumentsReadOutcome.Failed(read.message)
            }
        }
        return MoveDocumentsReadOutcome.Ready(documents)
    }

    private suspend fun readContent(
        state: ActivityHierarchyEditorState,
        path: String,
        currentPath: String
    ): MoveContentReadOutcome = if (path == currentPath) {
        MoveContentReadOutcome.Ready(currentContent(state))
    } else {
        val read = configGateway.readConfigTomlFile(path)
        if (read.ok) {
            MoveContentReadOutcome.Ready(read.content)
        } else {
            MoveContentReadOutcome.Failed(read.message)
        }
    }

    private fun failure(message: String): ActivityHierarchyMovePreviewOutcome.Failed =
        ActivityHierarchyMovePreviewOutcome.Failed(message)
}

private data class ActivityHierarchyMovePreviewResult(
    val ok: Boolean,
    val replacementPlan: ActivityNameReplacementPlan,
    val updatedDocuments: List<ActivityHierarchyDocumentOutput>,
    val message: String
)
