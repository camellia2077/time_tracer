package com.example.tracer

import org.tomlj.Toml
import org.tomlj.TomlTable
import java.util.UUID

internal enum class AliasEditorMode {
    STRUCTURED,
    ADVANCED
}

internal data class AliasTomlDocument(
    val parent: String,
    val nodes: List<AliasTomlNode>
)

internal sealed interface AliasTomlNode {
    val id: String
}

internal data class AliasTomlGroup(
    override val id: String = randomAliasNodeId(),
    val name: String,
    val nodes: List<AliasTomlNode> = emptyList()
) : AliasTomlNode

internal data class AliasTomlEntry(
    override val id: String = randomAliasNodeId(),
    val aliasKey: String,
    val canonicalLeaf: String
) : AliasTomlNode

internal data class AliasTomlParseResult(
    val document: AliasTomlDocument? = null,
    val errorMessage: String = ""
)

internal object AliasTomlEditorCodec {
    private val BareTomlKeyPattern = Regex("^[A-Za-z0-9_-]+$")

    fun parse(rawToml: String): AliasTomlParseResult {
        val parsed = Toml.parse(rawToml)
        if (parsed.hasErrors()) {
            return AliasTomlParseResult(
                errorMessage = parsed.errors().joinToString("; ") { error -> error.toString() }
            )
        }

        val parent = parsed.getString("parent")
            ?: return AliasTomlParseResult(
                errorMessage = "Alias child file must contain a non-empty `parent` string."
            )
        if (parent.isBlank()) {
            return AliasTomlParseResult(
                errorMessage = "Alias child file must contain a non-empty `parent` string."
            )
        }

        val aliasesTable = parsed.getTable("aliases")
            ?: return AliasTomlParseResult(
                errorMessage = "Alias child file must contain an `aliases` table."
            )

        return runCatching {
            val document = AliasTomlDocument(
                parent = parent,
                nodes = parseNodes(aliasesTable)
            )
            val structuralError = validateParsedDocument(document)
            if (structuralError != null) {
                AliasTomlParseResult(errorMessage = structuralError)
            } else {
                AliasTomlParseResult(document = document)
            }
        }.getOrElse { error ->
            AliasTomlParseResult(
                errorMessage = error.message ?: "unknown alias parse error"
            )
        }
    }

    fun serialize(document: AliasTomlDocument): String {
        val sections = mutableListOf<String>()
        sections += buildSection(
            path = listOf("aliases"),
            nodes = document.nodes
        )

        val builder = StringBuilder()
        builder.append("parent = ")
        builder.append(formatTomlString(document.parent))
        builder.append("\n\n")
        builder.append(sections.joinToString("\n\n"))
        builder.append("\n")
        return builder.toString()
    }

    fun validateForSave(document: AliasTomlDocument): String? {
        if (document.parent.isBlank()) {
            return "`parent` must not be empty."
        }
        if (document.nodes.isEmpty()) {
            return "Alias document must contain at least one alias group or alias entry."
        }
        return validateNodes(document.nodes, path = listOf("aliases"))
    }

    fun collectAliasKeys(document: AliasTomlDocument): List<String> = buildList {
        collectAliasKeysInto(document.nodes, this)
    }

    private fun validateParsedDocument(document: AliasTomlDocument): String? {
        if (document.parent.isBlank()) {
            return "Alias child file must contain a non-empty `parent` string."
        }
        return validateNodes(document.nodes, path = listOf("aliases"))
    }

    private fun parseNodes(table: TomlTable): List<AliasTomlNode> {
        return buildList {
            // Use immediate entrySet() instead of get(key): TomlTable.get(String)
            // reparses the key as a dotted TOML path, which breaks quoted/non-bare
            // alias keys such as Chinese activity names.
            for ((key, node) in table.entrySet()) {
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
                                nodes = parseNodes(node)
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

    private fun buildSection(path: List<String>, nodes: List<AliasTomlNode>): String {
        val entries = nodes.filterIsInstance<AliasTomlEntry>()
        val groups = nodes.filterIsInstance<AliasTomlGroup>()
        val bodyLines = mutableListOf<String>()
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
                nodes = group.nodes
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
                is AliasTomlGroup -> collectAliasKeysInto(node.nodes, sink)
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

private fun randomAliasNodeId(): String = UUID.randomUUID().toString()
