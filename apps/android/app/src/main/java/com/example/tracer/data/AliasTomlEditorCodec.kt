package com.example.tracer

import org.tomlj.TomlTable


internal enum class AliasEditorMode {
    STRUCTURED,
    ADVANCED
}

internal typealias AliasTomlDocument = ActivityAliasDocument
internal typealias AliasTomlNode = ActivityAliasNode
internal typealias AliasTomlGroup = ActivityCategory
internal typealias AliasTomlEntry = ActivityAlias

/**
 * A validated, in-memory-only relocation of one alias leaf into another group.
 *
 * The current Config flow deliberately does not apply this plan: committing a
 * relocation also requires TXT canonical-token migration and a replacement
 * database build. Keeping the plan separate lets the UI show that impact
 * before any persistent state changes.
 */
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

internal sealed interface AliasEntryMovePlanResult {
    data class Ready(val plan: AliasEntryMovePlan) : AliasEntryMovePlanResult
    data class Invalid(val message: String) : AliasEntryMovePlanResult
}

internal data class AliasEntryMoveDestination(
    val groupId: String,
    val groupPath: List<String>
)

internal data class AliasEntryPromotePlan(
    val entryId: String,
    val aliasKey: String,
    val canonicalLeaf: String,
    val sourceParentGroupId: String?,
    val sourceGroupPath: List<String>,
    val preservedCanonical: String
)

internal sealed interface AliasEntryPromotePlanResult {
    data class Ready(val plan: AliasEntryPromotePlan) : AliasEntryPromotePlanResult
    data class Invalid(val message: String) : AliasEntryPromotePlanResult
}

internal data class AliasTomlParseResult(
    val document: AliasTomlDocument? = null,
    val errorMessage: String = ""
)

internal object AliasTomlEditorCodec {
    private val BareTomlKeyPattern = Regex("^[A-Za-z0-9_-]+$")
    private const val GROUP_ALIASES_KEY = "group_aliases"

    fun parse(rawToml: String): AliasTomlParseResult = ActivityAliasTomlParser.parse(rawToml)

    fun serialize(document: AliasTomlDocument): String = ActivityAliasTomlSerializer.serialize(document)

    fun validateForSave(document: AliasTomlDocument): String? = ActivityAliasHierarchyValidator.validateForSave(document)

    fun collectAliasKeys(document: AliasTomlDocument): List<String> = ActivityAliasHierarchyValidator.collectAliasKeys(document)

    private fun validateParsedDocument(document: AliasTomlDocument): String? {
        if (document.parent.isBlank()) {
            return "Alias child file must contain a non-empty `parent` string."
        }
        return validateNodes(document.nodes, path = listOf("aliases"))
    }

    private fun parseNodes(table: TomlTable, isGroupTable: Boolean): List<AliasTomlNode> {
        return buildList {
            // Use immediate entrySet() instead of get(key): TomlTable.get(String)
            // reparses the key as a dotted TOML path, which breaks quoted/non-bare
            // alias keys such as Chinese activity names.
            for ((key, node) in table.entrySet()) {
                if (isGroupTable && key == GROUP_ALIASES_KEY) {
                    continue
                }
                when (node) {
                    is String -> {
                        add(
                            AliasTomlEntry(
                                aliasKey = key,
                                canonicalLeaf = node
                            )
                        )
                    }

                    is TomlTable -> {
                        add(
                            AliasTomlGroup(
                                name = key,
                                groupAliases = parseGroupAliases(node, key),
                                nodes = parseNodes(node, isGroupTable = true)
                            )
                        )
                    }

                    else -> {
                        throw IllegalArgumentException(
                            "Alias field `$key` must be a string or nested table."
                        )
                    }
                }
            }
        }
    }

    private fun parseGroupAliases(table: TomlTable, groupName: String): List<String> {
        val rawAliases = table.getArray(GROUP_ALIASES_KEY) ?: return emptyList()
        return buildList {
            for (index in 0 until rawAliases.size()) {
                val alias = rawAliases.getString(index)
                    ?: throw IllegalArgumentException(
                        "`$GROUP_ALIASES_KEY` in group `$groupName` must contain only strings."
                    )
                add(alias)
            }
        }
    }

    private fun buildSection(
        path: List<String>,
        nodes: List<AliasTomlNode>,
        groupAliases: List<String>
    ): String {
        val entries = nodes.filterIsInstance<AliasTomlEntry>()
        val groups = nodes.filterIsInstance<AliasTomlGroup>()
        val bodyLines = mutableListOf<String>()
        if (groupAliases.isNotEmpty()) {
            bodyLines += "$GROUP_ALIASES_KEY = [${groupAliases.joinToString(", ") { formatTomlString(it) }}]"
        }
        for (entry in entries) {
            bodyLines += "${formatTomlString(entry.aliasKey)} = ${formatTomlString(entry.canonicalLeaf)}"
        }

        val header = "[${path.joinToString(".") { segment -> formatTableSegment(segment) }}]"
        val sections = mutableListOf<String>()
        sections += if (bodyLines.isEmpty()) {
            header
        } else {
            "$header\n${bodyLines.joinToString("\n")}"
        }

        for (group in groups) {
            sections += buildSection(
                path = path + group.name,
                nodes = group.nodes,
                groupAliases = group.groupAliases
            )
        }
        return sections.joinToString("\n\n")
    }

    private fun formatTableSegment(segment: String): String {
        return if (BareTomlKeyPattern.matches(segment)) {
            segment
        } else {
            formatTomlString(segment)
        }
    }

    private fun formatTomlString(value: String): String {
        return buildString {
            append('"')
            for (character in value) {
                when (character) {
                    '\\' -> append("\\\\")
                    '"' -> append("\\\"")
                    '\n' -> append("\\n")
                    '\r' -> append("\\r")
                    '\t' -> append("\\t")
                    else -> append(character)
                }
            }
            append('"')
        }
    }

    private fun validateNodes(nodes: List<AliasTomlNode>, path: List<String>): String? {
        val groupNames = linkedSetOf<String>()
        val aliasKeys = linkedSetOf<String>()
        for (node in nodes) {
            when (node) {
                is AliasTomlGroup -> {
                    if (node.name.isBlank()) {
                        return "Alias group `${path.joinToString(".")}` must not contain an empty group name."
                    }
                    if (!groupNames.add(node.name)) {
                        return "Alias group `${path.joinToString(".")}` contains duplicate child group `${node.name}`."
                    }
                    if (aliasKeys.contains(node.name)) {
                        return "Alias group `${path.joinToString(".")}` cannot reuse `${node.name}` as both alias key and group name."
                    }
                    if (node.name == GROUP_ALIASES_KEY) {
                        return "Alias group name `$GROUP_ALIASES_KEY` is reserved."
                    }
                    for (groupAlias in node.groupAliases) {
                        if (groupAlias.isBlank()) {
                            return "Recordable aliases in group `${node.name}` must not be empty."
                        }
                        if (!aliasKeys.add(groupAlias)) {
                            return "Alias group `${path.joinToString(".")}` contains duplicate alias key `$groupAlias`."
                        }
                    }
                    val childError = validateNodes(node.nodes, path + node.name)
                    if (childError != null) {
                        return childError
                    }
                }

                is AliasTomlEntry -> {
                    if (node.aliasKey.isBlank()) {
                        return "Alias group `${path.joinToString(".")}` must not contain an empty alias key."
                    }
                    if (node.canonicalLeaf.isBlank()) {
                        return "Alias key `${node.aliasKey}` must map to a non-empty canonical leaf."
                    }
                    if (!aliasKeys.add(node.aliasKey)) {
                        return "Alias group `${path.joinToString(".")}` contains duplicate alias key `${node.aliasKey}`."
                    }
                    if (groupNames.contains(node.aliasKey)) {
                        return "Alias group `${path.joinToString(".")}` cannot reuse `${node.aliasKey}` as both alias key and group name."
                    }
                }
            }
        }
        return null
    }

    private fun collectAliasKeysInto(nodes: List<AliasTomlNode>, sink: MutableList<String>) {
        for (node in nodes) {
            when (node) {
                is AliasTomlEntry -> sink += node.aliasKey
                is AliasTomlGroup -> {
                    sink += node.groupAliases
                    collectAliasKeysInto(node.nodes, sink)
                }
            }
        }
    }
}

internal fun AliasTomlDocument.updateParent(value: String): AliasTomlDocument =
    copy(parent = value)

internal fun AliasTomlDocument.addGroup(parentGroupId: String?, name: String): AliasTomlDocument {
    val nextGroup = AliasTomlGroup(name = name)
    return if (parentGroupId == null) {
        copy(nodes = nodes + nextGroup)
    } else {
        copy(nodes = nodes.appendNodeToGroup(parentGroupId, nextGroup))
    }
}

internal fun AliasTomlDocument.renameGroup(groupId: String, name: String): AliasTomlDocument =
    copy(nodes = nodes.renameGroupById(groupId, name))

internal fun AliasTomlDocument.deleteGroup(groupId: String): AliasTomlDocument =
    copy(nodes = nodes.deleteGroupById(groupId))

internal fun AliasTomlDocument.addEntry(
    parentGroupId: String?,
    aliasKey: String,
    canonicalLeaf: String
): AliasTomlDocument {
    val nextEntry = AliasTomlEntry(aliasKey = aliasKey, canonicalLeaf = canonicalLeaf)
    return if (parentGroupId == null) {
        copy(nodes = nodes + nextEntry)
    } else {
        copy(nodes = nodes.appendNodeToGroup(parentGroupId, nextEntry))
    }
}

internal fun AliasTomlDocument.updateEntry(
    entryId: String,
    aliasKey: String,
    canonicalLeaf: String
): AliasTomlDocument = copy(
    nodes = nodes.updateEntryById(
        entryId = entryId,
        aliasKey = aliasKey,
        canonicalLeaf = canonicalLeaf
    )
)

internal fun AliasTomlDocument.deleteEntry(entryId: String): AliasTomlDocument =
    copy(nodes = nodes.deleteEntryById(entryId))

internal fun AliasTomlDocument.planEntryMove(
    entryId: String,
    targetGroupId: String
): AliasEntryMovePlanResult {
    val source = nodes.findAliasEntryLocation(entryId)
        ?: return AliasEntryMovePlanResult.Invalid("Alias entry to move was not found.")
    val target = nodes.findAliasGroupLocation(targetGroupId)
        ?: return AliasEntryMovePlanResult.Invalid("Destination alias group was not found.")
    val sourceParentGroupId = source.groupPath.lastOrNull()?.id
    if (sourceParentGroupId == target.group.id) {
        return AliasEntryMovePlanResult.Invalid("Alias entry is already in the selected group.")
    }

    val oldCanonical = canonicalPathForAliasEntry(
        parent = parent,
        groups = source.groupPath,
        canonicalLeaf = source.entry.canonicalLeaf
    )
    val newCanonical = canonicalPathForAliasEntry(
        parent = parent,
        groups = target.groupPath,
        canonicalLeaf = source.entry.canonicalLeaf
    )
    val collision = nodes.allAliasEntryLocations().firstOrNull { candidate ->
        candidate.entry.id != source.entry.id &&
            canonicalPathForAliasEntry(
                parent = parent,
                groups = candidate.groupPath,
                canonicalLeaf = candidate.entry.canonicalLeaf
            ) == newCanonical
    }
    if (collision != null) {
        return AliasEntryMovePlanResult.Invalid(
            "Moving `${source.entry.aliasKey}` would collide with alias `${collision.entry.aliasKey}` at canonical path `$newCanonical`."
        )
    }

    return AliasEntryMovePlanResult.Ready(
        AliasEntryMovePlan(
            entryId = source.entry.id,
            aliasKey = source.entry.aliasKey,
            canonicalLeaf = source.entry.canonicalLeaf,
            sourceParentGroupId = sourceParentGroupId,
            sourceGroupPath = source.groupPath.map(AliasTomlGroup::name),
            targetGroupId = target.group.id,
            targetGroupPath = target.groupPath.map(AliasTomlGroup::name),
            oldCanonical = oldCanonical,
            newCanonical = newCanonical
        )
    )
}

internal fun AliasTomlDocument.moveDestinationsForEntry(
    entryId: String
): List<AliasEntryMoveDestination> {
    val source = nodes.findAliasEntryLocation(entryId) ?: return emptyList()
    val sourceParentGroupId = source.groupPath.lastOrNull()?.id
    return nodes.allAliasGroupLocations()
        .asSequence()
        .filter { location -> location.group.id != sourceParentGroupId }
        .map { location ->
            AliasEntryMoveDestination(
                groupId = location.group.id,
                groupPath = location.groupPath.map(AliasTomlGroup::name)
            )
        }
        .sortedBy { destination -> destination.groupPath.joinToString("\u0000") }
        .toList()
}

/** Applies a previously validated move to the draft tree only; it never writes TOML. */
internal fun AliasTomlDocument.applyEntryMovePlan(plan: AliasEntryMovePlan): AliasTomlDocument {
    val source = nodes.findAliasEntryLocation(plan.entryId)
        ?: return this
    if (
        source.entry.aliasKey != plan.aliasKey ||
        source.entry.canonicalLeaf != plan.canonicalLeaf ||
        nodes.findAliasGroupLocation(plan.targetGroupId) == null
    ) {
        return this
    }
    return copy(
        nodes = nodes
            .deleteEntryById(plan.entryId)
            .appendNodeToGroup(plan.targetGroupId, source.entry)
    )
}

/** Renames one existing recordable alias on a category without changing its canonical path. */
internal fun AliasTomlDocument.renameGroupAlias(
    groupId: String,
    oldAlias: String,
    newAlias: String
): AliasTomlDocument = copy(nodes = nodes.renameGroupAlias(groupId, oldAlias, newAlias))

internal fun AliasTomlDocument.addGroupAlias(
    groupId: String,
    alias: String
): AliasTomlDocument = copy(nodes = nodes.addGroupAlias(groupId, alias))

internal fun AliasTomlDocument.planEntryPromotion(entryId: String): AliasEntryPromotePlanResult {
    val source = nodes.findAliasEntryLocation(entryId)
        ?: return AliasEntryPromotePlanResult.Invalid("Alias entry to promote was not found.")
    val collision = source.groupPath.lastOrNull()?.nodes.orEmpty()
        .filterIsInstance<AliasTomlGroup>()
        .any { it.name == source.entry.canonicalLeaf }
        || (source.groupPath.isEmpty() && nodes.filterIsInstance<AliasTomlGroup>()
            .any { it.name == source.entry.canonicalLeaf })
    if (collision) {
        return AliasEntryPromotePlanResult.Invalid(
            "A group named `${source.entry.canonicalLeaf}` already exists at this level."
        )
    }
    return AliasEntryPromotePlanResult.Ready(
        AliasEntryPromotePlan(
            entryId = source.entry.id,
            aliasKey = source.entry.aliasKey,
            canonicalLeaf = source.entry.canonicalLeaf,
            sourceParentGroupId = source.groupPath.lastOrNull()?.id,
            sourceGroupPath = source.groupPath.map(AliasTomlGroup::name),
            preservedCanonical = canonicalPathForAliasEntry(
                parent = parent,
                groups = source.groupPath,
                canonicalLeaf = source.entry.canonicalLeaf
            )
        )
    )
}

internal fun AliasTomlDocument.applyEntryPromotePlan(plan: AliasEntryPromotePlan): AliasTomlDocument {
    val source = nodes.findAliasEntryLocation(plan.entryId) ?: return this
    if (source.entry.aliasKey != plan.aliasKey || source.entry.canonicalLeaf != plan.canonicalLeaf) {
        return this
    }
    return copy(nodes = nodes.replaceEntryWithRecordableGroup(plan))
}

private fun List<AliasTomlNode>.appendNodeToGroup(
    parentGroupId: String,
    nextNode: AliasTomlNode
): List<AliasTomlNode> {
    return map { node ->
        when (node) {
            is AliasTomlEntry -> node
            is AliasTomlGroup -> {
                if (node.id == parentGroupId) {
                    node.copy(nodes = node.nodes + nextNode)
                } else {
                    node.copy(nodes = node.nodes.appendNodeToGroup(parentGroupId, nextNode))
                }
            }
        }
    }
}

private fun List<AliasTomlNode>.renameGroupById(groupId: String, name: String): List<AliasTomlNode> {
    return map { node ->
        when (node) {
            is AliasTomlEntry -> node
            is AliasTomlGroup -> {
                if (node.id == groupId) {
                    node.copy(name = name)
                } else {
                    node.copy(nodes = node.nodes.renameGroupById(groupId, name))
                }
            }
        }
    }
}

private fun List<AliasTomlNode>.deleteGroupById(groupId: String): List<AliasTomlNode> {
    return buildList {
        for (node in this@deleteGroupById) {
            when (node) {
                is AliasTomlEntry -> add(node)
                is AliasTomlGroup -> {
                    if (node.id != groupId) {
                        add(node.copy(nodes = node.nodes.deleteGroupById(groupId)))
                    }
                }
            }
        }
    }
}

private fun List<AliasTomlNode>.updateEntryById(
    entryId: String,
    aliasKey: String,
    canonicalLeaf: String
): List<AliasTomlNode> {
    return map { node ->
        when (node) {
            is AliasTomlEntry -> {
                if (node.id == entryId) {
                    node.copy(aliasKey = aliasKey, canonicalLeaf = canonicalLeaf)
                } else {
                    node
                }
            }

            is AliasTomlGroup -> {
                node.copy(
                    nodes = node.nodes.updateEntryById(
                        entryId = entryId,
                        aliasKey = aliasKey,
                        canonicalLeaf = canonicalLeaf
                    )
                )
            }
        }
    }
}

private fun List<AliasTomlNode>.deleteEntryById(entryId: String): List<AliasTomlNode> {
    return buildList {
        for (node in this@deleteEntryById) {
            when (node) {
                is AliasTomlEntry -> if (node.id != entryId) add(node)
                is AliasTomlGroup -> add(node.copy(nodes = node.nodes.deleteEntryById(entryId)))
            }
        }
    }
}

private fun List<AliasTomlNode>.replaceEntryWithRecordableGroup(
    plan: AliasEntryPromotePlan
): List<AliasTomlNode> = map { node ->
    when (node) {
        is AliasTomlEntry -> if (node.id == plan.entryId) {
            AliasTomlGroup(
                id = node.id,
                name = plan.canonicalLeaf,
                groupAliases = listOf(plan.aliasKey)
            )
        } else {
            node
        }
        is AliasTomlGroup -> node.copy(nodes = node.nodes.replaceEntryWithRecordableGroup(plan))
    }
}

private fun List<AliasTomlNode>.renameGroupAlias(
    groupId: String,
    oldAlias: String,
    newAlias: String
): List<AliasTomlNode> = map { node ->
    when (node) {
        is AliasTomlEntry -> node
        is AliasTomlGroup -> {
            val aliases = if (node.id == groupId) {
                node.groupAliases.map { alias -> if (alias == oldAlias) newAlias else alias }
            } else {
                node.groupAliases
            }
            node.copy(
                groupAliases = aliases,
                nodes = node.nodes.renameGroupAlias(groupId, oldAlias, newAlias)
            )
        }
    }
}

private fun List<AliasTomlNode>.addGroupAlias(groupId: String, alias: String): List<AliasTomlNode> = map { node ->
    when (node) {
        is AliasTomlEntry -> node
        is AliasTomlGroup -> node.copy(
            groupAliases = if (node.id == groupId) node.groupAliases + alias else node.groupAliases,
            nodes = node.nodes.addGroupAlias(groupId, alias)
        )
    }
}

private data class AliasEntryLocation(
    val entry: AliasTomlEntry,
    val groupPath: List<AliasTomlGroup>
)

private data class AliasGroupLocation(
    val group: AliasTomlGroup,
    val groupPath: List<AliasTomlGroup>
)

private fun List<AliasTomlNode>.findAliasEntryLocation(entryId: String): AliasEntryLocation? =
    allAliasEntryLocations().firstOrNull { location -> location.entry.id == entryId }

private fun List<AliasTomlNode>.allAliasEntryLocations(): List<AliasEntryLocation> = buildList {
    fun collect(nodes: List<AliasTomlNode>, groupPath: List<AliasTomlGroup>) {
        for (node in nodes) {
            when (node) {
                is AliasTomlEntry -> add(AliasEntryLocation(node, groupPath))
                is AliasTomlGroup -> collect(node.nodes, groupPath + node)
            }
        }
    }
    collect(this@allAliasEntryLocations, emptyList())
}

private fun List<AliasTomlNode>.findAliasGroupLocation(groupId: String): AliasGroupLocation? {
    fun find(
        nodes: List<AliasTomlNode>,
        parentPath: List<AliasTomlGroup>
    ): AliasGroupLocation? {
        for (node in nodes) {
            if (node is AliasTomlGroup) {
                val path = parentPath + node
                if (node.id == groupId) {
                    return AliasGroupLocation(node, path)
                }
                find(node.nodes, path)?.let { return it }
            }
        }
        return null
    }
    return find(this, emptyList())
}

private fun List<AliasTomlNode>.allAliasGroupLocations(): List<AliasGroupLocation> = buildList {
    fun collect(nodes: List<AliasTomlNode>, parentPath: List<AliasTomlGroup>) {
        for (node in nodes) {
            if (node is AliasTomlGroup) {
                val path = parentPath + node
                add(AliasGroupLocation(node, path))
                collect(node.nodes, path)
            }
        }
    }
    collect(this@allAliasGroupLocations, emptyList())
}

private fun canonicalPathForAliasEntry(
    parent: String,
    groups: List<AliasTomlGroup>,
    canonicalLeaf: String
): String = (listOf(parent.trim()) + groups.map { group -> group.name.trim() } + canonicalLeaf.trim())
    .joinToString("_")
