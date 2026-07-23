package com.example.tracer

import java.util.UUID

/**
 * Shared, in-memory representation of one converter alias TOML file.
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
    val aliasKey: String,
    val canonicalLeaf: String
) : ActivityAliasNode

private fun randomActivityAliasNodeId(): String = UUID.randomUUID().toString()
