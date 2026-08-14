package com.example.tracer

import java.util.UUID

/**
 * Shared, in-memory representation of one canonical TOML file.
 *
 * A category can have child aliases and its own record names. Category record
 * names resolve to the category's canonical path; child aliases resolve below
 * that path.
 */
data class ActivityHierarchyDocument(
    val parent: String,
    val color: String? = null,
    val nodes: List<ActivityHierarchyDocumentNode>
)

sealed interface ActivityHierarchyDocumentNode {
    val id: String
}

data class ActivityHierarchyGroup(
    override val id: String = randomActivityHierarchyNodeId(),
    val name: String,
    val groupAliases: List<String> = emptyList(),
    val nodes: List<ActivityHierarchyDocumentNode> = emptyList()
) : ActivityHierarchyDocumentNode

data class ActivityHierarchyLeaf(
    override val id: String = randomActivityHierarchyNodeId(),
    val canonicalLeaf: String,
    val aliases: List<String> = emptyList()
) : ActivityHierarchyDocumentNode {
    /** Compatibility display value for flows that operate on one alias. */
    val aliasKey: String
        get() = aliases.firstOrNull().orEmpty()
}

private fun randomActivityHierarchyNodeId(): String = UUID.randomUUID().toString()

/** Adapts the core-owned hierarchy snapshot into presentation-only row state. */
fun ActivityHierarchySnapshot.toActivityHierarchyDocument(): ActivityHierarchyDocument =
    ActivityHierarchyDocument(
        parent = parent,
        color = color,
        nodes = nodes.map(ActivityHierarchyNode::toPresentationNode)
    )

private fun ActivityHierarchyNode.toPresentationNode(): ActivityHierarchyDocumentNode =
    if (kind == ActivityHierarchyNodeKind.GROUP) {
        ActivityHierarchyGroup(
            id = path,
            name = canonicalKey,
            groupAliases = aliases,
            nodes = children.map(ActivityHierarchyNode::toPresentationNode)
        )
    } else {
        ActivityHierarchyLeaf(
            id = path,
            canonicalLeaf = canonicalKey,
            aliases = aliases
        )
    }
