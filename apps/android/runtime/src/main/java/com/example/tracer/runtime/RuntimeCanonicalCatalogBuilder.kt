package com.example.tracer

internal object RuntimeCanonicalCatalogBuilder {
    private const val CANONICAL_SEGMENT_SEPARATOR = "_"

    fun buildFromSnapshots(
        entriesByFile: List<Pair<String, ActivityHierarchySnapshot>>
    ): CanonicalCatalogResult {
        val rootNodes = linkedMapOf<String, MutableCanonicalPathNode>()
        val canonicalEntries = linkedMapOf<String, MutableCanonicalCatalogEntry>()

        for ((sourceFilePath, snapshot) in entriesByFile) {
            if (snapshot.nodes.isEmpty()) {
                continue
            }
            val root = rootNodes.getOrPut(snapshot.parent) {
                MutableCanonicalPathNode(
                    name = snapshot.parent,
                    path = snapshot.parent
                )
            }
            appendNodes(
                currentNode = root,
                currentPathSegments = listOf(snapshot.parent),
                nodes = snapshot.nodes,
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
        nodes: List<ActivityHierarchyNode>,
        sourceFilePath: String,
        canonicalEntries: MutableMap<String, MutableCanonicalCatalogEntry>
    ) {
        for (node in nodes) {
            val canonicalKey = node.canonicalKey.trim()
            if (canonicalKey.isEmpty()) {
                continue
            }
            if (!node.isGroup) {
                val canonicalPath = (currentPathSegments + canonicalKey)
                    .joinToString(CANONICAL_SEGMENT_SEPARATOR)
                val aggregate = canonicalEntries.getOrPut(canonicalPath) {
                    MutableCanonicalCatalogEntry(
                        canonicalLeaf = canonicalKey,
                        canonicalPath = canonicalPath,
                        sourceFilePath = sourceFilePath
                    )
                }
                aggregate.aliases += node.aliases
                currentNode.entryPaths += canonicalPath
                continue
            }

            val childPathSegments = currentPathSegments + canonicalKey
            val childPath = childPathSegments.joinToString(CANONICAL_SEGMENT_SEPARATOR)
            val childNode = currentNode.children.getOrPut(canonicalKey) {
                MutableCanonicalPathNode(name = canonicalKey, path = childPath)
            }
            if (node.aliases.isNotEmpty()) {
                val aggregate = canonicalEntries.getOrPut(childPath) {
                    MutableCanonicalCatalogEntry(
                        canonicalLeaf = canonicalKey,
                        canonicalPath = childPath,
                        sourceFilePath = sourceFilePath
                    )
                }
                aggregate.aliases += node.aliases
                childNode.entryPaths += childPath
            }
            appendNodes(
                currentNode = childNode,
                currentPathSegments = childPathSegments,
                nodes = node.children,
                sourceFilePath = sourceFilePath,
                canonicalEntries = canonicalEntries
            )
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
