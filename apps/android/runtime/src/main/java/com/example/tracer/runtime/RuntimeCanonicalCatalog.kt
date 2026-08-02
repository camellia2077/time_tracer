package com.example.tracer

import org.tomlj.Toml
import org.tomlj.TomlArray
import org.tomlj.TomlTable

internal typealias CanonicalAliasDocument = ActivityAliasDocument
internal typealias CanonicalAliasNode = ActivityAliasNode
internal typealias CanonicalAliasGroup = ActivityCategory
internal typealias CanonicalAliasEntry = ActivityAlias

internal data class CanonicalAliasParseResult(
    val document: CanonicalAliasDocument? = null,
    val errorMessage: String = ""
)

internal object RuntimeCanonicalCatalogParser {
    private const val GROUP_ALIASES_KEY = "group_aliases"
    fun parse(rawToml: String): CanonicalAliasParseResult {
        val parsed = Toml.parse(rawToml)
        if (parsed.hasErrors()) {
            return CanonicalAliasParseResult(
                errorMessage = parsed.errors().joinToString("; ") { error -> error.toString() }
            )
        }

        val parent = parsed.getString("parent")?.trim().orEmpty()
        if (parent.isEmpty()) {
            return CanonicalAliasParseResult(
                errorMessage = "Activity hierarchy TOML must contain a non-empty `parent` string."
            )
        }

        val canonicalTable = parsed.getTable("canonical")
            ?: return CanonicalAliasParseResult(
                errorMessage = "Activity hierarchy TOML must contain a `canonical` table."
            )

        return runCatching {
            CanonicalAliasParseResult(
                document = CanonicalAliasDocument(
                    parent = parent,
                    nodes = parseNodes(canonicalTable)
                )
            )
        }.getOrElse { error ->
            CanonicalAliasParseResult(
                errorMessage = error.message ?: "unknown canonical catalog parse error"
            )
        }
    }

    private fun parseNodes(table: TomlTable, isGroupTable: Boolean = false): List<CanonicalAliasNode> = buildList {
        for ((key, node) in table.entrySet()) {
            if (key == GROUP_ALIASES_KEY) {
                if (!isGroupTable) {
                    throw IllegalArgumentException("`group_aliases` is only valid inside an activity category.")
                }
                continue
            }
            when (node) {
                is TomlArray -> add(
                    CanonicalAliasEntry(
                        canonicalLeaf = key.trim(),
                        aliases = node.toStringList(key)
                    )
                )

                is TomlTable -> add(
                    CanonicalAliasGroup(
                        name = key,
                        groupAliases = parseGroupAliases(node),
                        nodes = parseNodes(node, isGroupTable = true)
                    )
                )

                else -> throw IllegalArgumentException(
                    "Alias field `$key` must be an array or nested table."
                )
            }
        }
    }

    private fun parseGroupAliases(table: TomlTable): List<String> {
        val aliases = table.getArray(GROUP_ALIASES_KEY) ?: return emptyList()
        return aliases.toStringList(GROUP_ALIASES_KEY)
    }

    private fun TomlArray.toStringList(fieldName: String): List<String> = buildList {
        for (index in 0 until size()) {
            val value = getString(index)?.trim()
                ?: throw IllegalArgumentException("`$fieldName` must be a string array.")
            if (value.isEmpty()) throw IllegalArgumentException("`$fieldName` must not contain empty names.")
            add(value)
        }
    }
}

internal object RuntimeCanonicalCatalogBuilder {
    private const val CANONICAL_SEGMENT_SEPARATOR = "_"

    fun build(entriesByFile: List<Pair<String, CanonicalAliasDocument>>): CanonicalCatalogResult {
        val rootNodes = linkedMapOf<String, MutableCanonicalPathNode>()
        val canonicalEntries = linkedMapOf<String, MutableCanonicalCatalogEntry>()

        for ((sourceFilePath, document) in entriesByFile) {
            val root = rootNodes.getOrPut(document.parent) {
                MutableCanonicalPathNode(
                    name = document.parent,
                    path = document.parent
                )
            }
            appendNodes(
                currentNode = root,
                currentPathSegments = listOf(document.parent),
                nodes = document.nodes,
                sourceFilePath = sourceFilePath,
                canonicalEntries = canonicalEntries
            )
        }

        val builtRoots = rootNodes.values.map { node ->
            node.toImmutable(canonicalEntries)
        }
            .sortedBy { it.path }
        val builtEntries = canonicalEntries.values
            .map { it.toImmutable() }
            .sortedBy { it.canonicalPath }

        return CanonicalCatalogResult(
            ok = builtEntries.isNotEmpty(),
            roots = builtRoots,
            entries = builtEntries,
            message = if (builtEntries.isNotEmpty()) {
                "Loaded ${builtEntries.size} canonical catalog entries."
            } else {
                "Canonical catalog query failed: empty catalog."
            }
        )
    }

    private fun appendNodes(
        currentNode: MutableCanonicalPathNode,
        currentPathSegments: List<String>,
        nodes: List<CanonicalAliasNode>,
        sourceFilePath: String,
        canonicalEntries: MutableMap<String, MutableCanonicalCatalogEntry>
    ) {
        for (node in nodes) {
            when (node) {
                is CanonicalAliasEntry -> {
                    val canonicalLeaf = node.canonicalLeaf.trim()
                    if (canonicalLeaf.isEmpty()) {
                        continue
                    }
                    val canonicalPath = (currentPathSegments + canonicalLeaf)
                        .joinToString(CANONICAL_SEGMENT_SEPARATOR)
                    val aggregate = canonicalEntries.getOrPut(canonicalPath) {
                        MutableCanonicalCatalogEntry(
                            canonicalLeaf = canonicalLeaf,
                            canonicalPath = canonicalPath,
                            sourceFilePath = sourceFilePath
                        )
                    }
                    aggregate.aliases += node.aliases
                    currentNode.entryPaths += canonicalPath
                }

                is CanonicalAliasGroup -> {
                    val normalizedGroupName = node.name.trim()
                    if (normalizedGroupName.isEmpty()) {
                        continue
                    }
                    val childPathSegments = currentPathSegments + normalizedGroupName
                    val childPath = childPathSegments.joinToString(CANONICAL_SEGMENT_SEPARATOR)
                    val childNode = currentNode.children.getOrPut(normalizedGroupName) {
                        MutableCanonicalPathNode(
                            name = normalizedGroupName,
                            path = childPath
                        )
                    }
                    if (node.groupAliases.isNotEmpty()) {
                        val aggregate = canonicalEntries.getOrPut(childPath) {
                            MutableCanonicalCatalogEntry(
                                canonicalLeaf = normalizedGroupName,
                                canonicalPath = childPath,
                                sourceFilePath = sourceFilePath
                            )
                        }
                        aggregate.aliases += node.groupAliases
                        childNode.entryPaths += childPath
                    }
                    appendNodes(
                        currentNode = childNode,
                        currentPathSegments = childPathSegments,
                        nodes = node.nodes,
                        sourceFilePath = sourceFilePath,
                        canonicalEntries = canonicalEntries
                    )
                }
            }
        }
    }

    private class MutableCanonicalPathNode(
        val name: String,
        val path: String,
        val entryPaths: LinkedHashSet<String> = linkedSetOf(),
        val children: LinkedHashMap<String, MutableCanonicalPathNode> = linkedMapOf()
    ) {
        fun toImmutable(
            canonicalEntries: Map<String, MutableCanonicalCatalogEntry>
        ): CanonicalPathNode = CanonicalPathNode(
            name = name,
            path = path,
            entries = entryPaths.mapNotNull { pathKey ->
                canonicalEntries[pathKey]?.toImmutable()
            }.sortedBy { it.canonicalPath },
            children = children.values
                .map { it.toImmutable(canonicalEntries) }
                .sortedBy { it.path }
        )
    }

    private class MutableCanonicalCatalogEntry(
        val canonicalLeaf: String,
        val canonicalPath: String,
        val sourceFilePath: String,
        val aliases: LinkedHashSet<String> = linkedSetOf()
    ) {
        fun toImmutable(): CanonicalCatalogEntry = CanonicalCatalogEntry(
            canonicalLeaf = canonicalLeaf,
            canonicalPath = canonicalPath,
            sourceFilePath = sourceFilePath,
            aliases = aliases.filter { it.isNotEmpty() }.sorted()
        )
    }
}
