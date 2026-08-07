package com.example.tracer

/** Presentation-only state rendered from the core hierarchy snapshot. */
internal enum class AliasEditorMode { STRUCTURED, ADVANCED }

internal enum class AliasMoveNodeKind {
    LEAF,
    GROUP
}

/** Preview data only; core owns move validation and the actual edit. */
internal data class AliasEntryMovePlan(
    val entryId: String,
    val aliasKey: String,
    val canonicalLeaf: String,
    val nodeKind: AliasMoveNodeKind = AliasMoveNodeKind.LEAF,
    val sourceParentGroupId: String?,
    val sourceGroupPath: List<String>,
    val targetGroupId: String,
    val targetGroupPath: List<String>,
    val oldCanonical: String,
    val newCanonical: String,
    val sourceFilePath: String = "",
    val destinationFilePath: String = "",
    val destinationGroupPath: List<String> = emptyList(),
    val updatedDocuments: List<ActivityHierarchyDocumentOutput> = emptyList(),
    val replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan()
)

internal data class AliasEntryMoveDestination(
    val groupId: String,
    val groupPath: List<String>,
    val sourceName: String = "",
    val isRoot: Boolean = false
)

internal data class AliasEntryMoveDestinationDocument(
    val sourceName: String,
    val displayName: String,
    val document: ActivityHierarchyDocument,
    val rootSelectable: Boolean = true,
    val excludedGroupPath: List<String> = emptyList(),
    val excludeDescendants: Boolean = false
)

internal data class AliasEntryMoveTarget(
    val sourceName: String,
    val groupPath: List<String>,
    val groupId: String? = null
)

internal fun ActivityHierarchyDocument.findAliasEntry(entryId: String): ActivityHierarchyLeaf? =
    nodes.entryLocations().firstOrNull { it.entry.id == entryId }?.entry

internal fun ActivityHierarchyDocument.allAliasEntries(): List<ActivityHierarchyLeaf> =
    nodes.entryLocations().map { it.entry }

internal fun ActivityHierarchyDocument.findAliasGroup(groupId: String): ActivityHierarchyGroup? =
    nodes.groupLocations().firstOrNull { it.group.id == groupId }?.group

internal fun ActivityHierarchyDocument.canonicalTargetPathForEntry(entryId: String): String? =
    nodes.entryLocations().firstOrNull { it.entry.id == entryId }?.let { location ->
        (location.groups.map(ActivityHierarchyGroup::name) + location.entry.canonicalLeaf).joinToString(".")
    }

internal fun ActivityHierarchyDocument.canonicalTargetPathForGroup(groupId: String): String? =
    nodes.groupLocations().firstOrNull { it.group.id == groupId }
        ?.groups
        ?.joinToString(".") { it.name }

internal fun ActivityHierarchyDocument.moveDestinationsForEntry(entryId: String): List<AliasEntryMoveDestination> {
    val sourceParent = nodes.entryLocations().firstOrNull { it.entry.id == entryId }
        ?.groups?.lastOrNull()?.id
    return nodes.groupLocations()
        .filter { it.group.id != sourceParent }
        .map { AliasEntryMoveDestination(it.group.id, it.groups.map(ActivityHierarchyGroup::name)) }
        .sortedBy { it.groupPath.joinToString("\u0000") }
}

private data class EntryLocation(val entry: ActivityHierarchyLeaf, val groups: List<ActivityHierarchyGroup>)
private data class GroupLocation(val group: ActivityHierarchyGroup, val groups: List<ActivityHierarchyGroup>)

private fun List<ActivityHierarchyDocumentNode>.entryLocations(): List<EntryLocation> = buildList {
    fun visit(nodes: List<ActivityHierarchyDocumentNode>, groups: List<ActivityHierarchyGroup>) {
        nodes.forEach { node -> when (node) {
            is ActivityHierarchyLeaf -> add(EntryLocation(node, groups))
            is ActivityHierarchyGroup -> visit(node.nodes, groups + node)
        } }
    }
    visit(this@entryLocations, emptyList())
}

private fun List<ActivityHierarchyDocumentNode>.groupLocations(): List<GroupLocation> = buildList {
    fun visit(nodes: List<ActivityHierarchyDocumentNode>, groups: List<ActivityHierarchyGroup>) {
        nodes.filterIsInstance<ActivityHierarchyGroup>().forEach { group ->
            val path = groups + group
            add(GroupLocation(group, path))
            visit(group.nodes, path)
        }
    }
    visit(this@groupLocations, emptyList())
}
