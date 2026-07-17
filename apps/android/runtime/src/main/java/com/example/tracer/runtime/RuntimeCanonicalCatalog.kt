package com.example.tracer

import org.tomlj.Toml
import org.tomlj.TomlTable

internal data class CanonicalAliasDocument(
    val parent: String,
    val nodes: List<CanonicalAliasNode>
)

internal sealed interface CanonicalAliasNode

internal data class CanonicalAliasGroup(
    val name: String,
    val nodes: List<CanonicalAliasNode> = emptyList()
) : CanonicalAliasNode

internal data class CanonicalAliasEntry(
    val aliasKey: String,
    val canonicalLeaf: String
) : CanonicalAliasNode

internal data class CanonicalAliasParseResult(
    val document: CanonicalAliasDocument? = null,
    val errorMessage: String = ""
)

internal object RuntimeCanonicalCatalogParser {
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
                errorMessage = "Alias child file must contain a non-empty `parent` string."
            )
        }

        val aliasesTable = parsed.getTable("aliases")
            ?: return CanonicalAliasParseResult(
                errorMessage = "Alias child file must contain an `aliases` table."
            )

        return runCatching {
            CanonicalAliasParseResult(
                document = CanonicalAliasDocument(
                    parent = parent,
                    nodes = parseNodes(aliasesTable)
                )
            )
        }.getOrElse { error ->
            CanonicalAliasParseResult(
                errorMessage = error.message ?: "unknown canonical catalog parse error"
            )
        }
    }

    private fun parseNodes(table: TomlTable): List<CanonicalAliasNode> = buildList {
        for ((key, node) in table.entrySet()) {
            when (node) {
                is String -> add(
                    CanonicalAliasEntry(
                        aliasKey = key,
                        canonicalLeaf = node.trim()
                    )
                )

                is TomlTable -> add(
                    CanonicalAliasGroup(
                        name = key,
                        nodes = parseNodes(node)
                    )
                )

                else -> throw IllegalArgumentException(
                    "Alias field `$key` must be a string or nested table."
                )
            }
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
                    aggregate.aliases += node.aliasKey.trim()
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
