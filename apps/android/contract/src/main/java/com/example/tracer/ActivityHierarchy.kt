package com.example.tracer

import java.util.UUID

/**
 * Shared, in-memory representation of one alias TOML file.
 *
 * A category can have child aliases and its own record names. Category record
 * names resolve to the category's canonical path; child aliases resolve below
 * that path.
 */
data class ActivityAliasDocument(
    val parent: String,
    val nodes: List<ActivityAliasNode>
)

sealed interface ActivityAliasNode {
    val id: String
}

data class ActivityCategory(
    override val id: String = randomActivityAliasNodeId(),
    val name: String,
    val groupAliases: List<String> = emptyList(),
    val nodes: List<ActivityAliasNode> = emptyList()
) : ActivityAliasNode

data class ActivityAlias(
    override val id: String = randomActivityAliasNodeId(),
    val canonicalLeaf: String,
    val aliases: List<String> = emptyList()
) : ActivityAliasNode {
    /** Compatibility display value for flows that operate on one alias. */
    val aliasKey: String
        get() = aliases.firstOrNull().orEmpty()
}

private fun randomActivityAliasNodeId(): String = UUID.randomUUID().toString()

/** Adapts the core-owned hierarchy snapshot into presentation-only row state. */
fun ActivityHierarchySnapshot.toActivityAliasDocument(): ActivityAliasDocument =
    ActivityAliasDocument(
        parent = parent,
        nodes = nodes.map(ActivityHierarchyNode::toPresentationNode)
    )

private fun ActivityHierarchyNode.toPresentationNode(): ActivityAliasNode =
    if (kind == ActivityHierarchyNodeKind.GROUP) {
        ActivityCategory(
            id = path,
            name = canonicalKey,
            groupAliases = aliases,
            nodes = children.map(ActivityHierarchyNode::toPresentationNode)
        )
    } else {
        ActivityAlias(
            id = path,
            canonicalLeaf = canonicalKey,
            aliases = aliases
        )
    }
