package com.example.tracer

/** Presentation-only state rendered from the core hierarchy snapshot. */
internal enum class AliasEditorMode { STRUCTURED, ADVANCED }

internal typealias AliasTomlDocument = ActivityAliasDocument
internal typealias AliasTomlNode = ActivityAliasNode
internal typealias AliasTomlGroup = ActivityCategory
internal typealias AliasTomlEntry = ActivityAlias

/** Preview data only; core owns move validation and the actual edit. */
internal data class AliasEntryMovePlan(
    val entryId: String,
    val aliasKey: String,
    val canonicalLeaf: String,
    val sourceParentGroupId: String?,
    val sourceGroupPath: List<String>,
    val targetGroupId: String,
    val targetGroupPath: List<String>,
    val oldCanonical: String,
    val newCanonical: String
)

internal data class AliasEntryMoveDestination(
    val groupId: String,
    val groupPath: List<String>
)

internal fun AliasTomlDocument.findAliasEntry(entryId: String): AliasTomlEntry? =
    nodes.entryLocations().firstOrNull { it.entry.id == entryId }?.entry

internal fun AliasTomlDocument.canonicalTargetPathForEntry(entryId: String): String? =
    nodes.entryLocations().firstOrNull { it.entry.id == entryId }?.let { location ->
        (location.groups.map(AliasTomlGroup::name) + location.entry.canonicalLeaf).joinToString(".")
    }

internal fun AliasTomlDocument.canonicalTargetPathForGroup(groupId: String): String? =
    nodes.groupLocations().firstOrNull { it.group.id == groupId }
        ?.groups
        ?.joinToString(".") { it.name }

internal fun AliasTomlDocument.moveDestinationsForEntry(entryId: String): List<AliasEntryMoveDestination> {
    val sourceParent = nodes.entryLocations().firstOrNull { it.entry.id == entryId }
        ?.groups?.lastOrNull()?.id
    return nodes.groupLocations()
        .filter { it.group.id != sourceParent }
        .map { AliasEntryMoveDestination(it.group.id, it.groups.map(AliasTomlGroup::name)) }
        .sortedBy { it.groupPath.joinToString("\u0000") }
}

private data class EntryLocation(val entry: AliasTomlEntry, val groups: List<AliasTomlGroup>)
private data class GroupLocation(val group: AliasTomlGroup, val groups: List<AliasTomlGroup>)

private fun List<AliasTomlNode>.entryLocations(): List<EntryLocation> = buildList {
    fun visit(nodes: List<AliasTomlNode>, groups: List<AliasTomlGroup>) {
        nodes.forEach { node -> when (node) {
            is AliasTomlEntry -> add(EntryLocation(node, groups))
            is AliasTomlGroup -> visit(node.nodes, groups + node)
        } }
    }
    visit(this@entryLocations, emptyList())
}

private fun List<AliasTomlNode>.groupLocations(): List<GroupLocation> = buildList {
    fun visit(nodes: List<AliasTomlNode>, groups: List<AliasTomlGroup>) {
        nodes.filterIsInstance<AliasTomlGroup>().forEach { group ->
            val path = groups + group
            add(GroupLocation(group, path))
            visit(group.nodes, path)
        }
    }
    visit(this@groupLocations, emptyList())
}
