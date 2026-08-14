package com.example.tracer

enum class ActivityHierarchyNodeKind(val wireValue: String) {
    LEAF("leaf"),
    GROUP("group");

    companion object {
        fun fromWireValue(value: String, legacyIsGroup: Boolean): ActivityHierarchyNodeKind =
            entries.firstOrNull { it.wireValue == value } ?: if (legacyIsGroup) GROUP else LEAF
    }
}

data class ActivityHierarchyNode(
    val canonicalKey: String,
    val path: String,
    val kind: ActivityHierarchyNodeKind,
    val aliases: List<String>,
    val children: List<ActivityHierarchyNode>
) {
    val isGroup: Boolean
        get() = kind == ActivityHierarchyNodeKind.GROUP
}

data class ActivityHierarchySnapshot(
    val parent: String,
    val color: String? = null,
    val nodes: List<ActivityHierarchyNode>
)

data class ActivityHierarchyDescribeResult(
    val ok: Boolean,
    val hierarchy: ActivityHierarchySnapshot? = null,
    val message: String = ""
)

data class ActivityHierarchyDocumentInput(
    val sourceName: String,
    val tomlContent: String
)

data class ActivityHierarchyValidationResult(
    val ok: Boolean,
    val message: String = ""
)

enum class ActivityHierarchyOperationKind(val wireValue: String) {
    ADD_GROUP("add_group"),
    DELETE_GROUP("delete_group"),
    RENAME_GROUP_CANONICAL("rename_group_canonical"),
    SET_GROUP_ALIASES("set_group_aliases"),
    RENAME_GROUP_ALIAS("rename_group_alias"),
    APPEND_GROUP_ALIAS("append_group_alias"),
    MOVE_GROUP("move_group"),
    ADD_LEAF("add_leaf"),
    DELETE_LEAF("delete_leaf"),
    RENAME_LEAF_CANONICAL("rename_leaf_canonical"),
    SET_LEAF_ALIASES("set_leaf_aliases"),
    APPEND_LEAF_ALIAS("append_leaf_alias"),
    MOVE_LEAF("move_leaf"),
    MERGE_LEAF_CANONICAL("merge_leaf_canonical"),
    PROMOTE_LEAF("promote_leaf"),
    RENAME_PARENT("rename_parent"),
    SET_PARENT_COLOR("set_parent_color")
}

data class ActivityHierarchyOperation(
    val kind: ActivityHierarchyOperationKind,
    val targetPath: String = "",
    val destinationPath: String = "",
    val canonicalKey: String = "",
    val newName: String = "",
    val oldParent: String = "",
    val color: String = "",
    val targetAlias: String = "",
    val oldAlias: String = "",
    val aliases: List<String> = emptyList()
)

data class AliasKeyReplacement(
    val oldAlias: String,
    val newAlias: String
)

/** Core-produced token replacements applied together with hierarchy changes. */
data class ActivityNameReplacementPlan(
    val canonical: List<CanonicalActivityNameReplacement> = emptyList(),
    val aliases: List<AliasKeyReplacement> = emptyList()
) {
    val isEmpty: Boolean
        get() = canonical.isEmpty() && aliases.isEmpty()

    companion object {
        fun fromCore(
            canonical: List<CanonicalActivityNameReplacement>,
            aliases: List<AliasKeyReplacement>
        ): ActivityNameReplacementPlan = ActivityNameReplacementPlan(
            canonical = canonical,
            aliases = aliases
        )
    }
}

data class ActivityHierarchyOperationResult(
    val ok: Boolean,
    val updatedTomlContent: String,
    val replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan(),
    val hierarchy: ActivityHierarchySnapshot? = null,
    val message: String = ""
)

data class ActivityHierarchyDocumentOutput(
    val sourceName: String,
    val updatedTomlContent: String
)

data class ActivityHierarchyDocumentRename(
    val oldSourceName: String,
    val newSourceName: String
)

data class ActivityHierarchyCrossDocumentOperationResult(
    val ok: Boolean,
    val updatedDocuments: List<ActivityHierarchyDocumentOutput> = emptyList(),
    val replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan(),
    val message: String = ""
)

data class ActivityHierarchyMigrationRequest(
    val configRelativePath: String,
    val updatedTomlContent: String,
    val replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan(),
    val updatedDocuments: List<ActivityHierarchyDocumentInput> = emptyList(),
    val configFileRename: ActivityHierarchyDocumentRename? = null,
    val allowMissingConfig: Boolean = false
)

data class ActivityHierarchyMigrationResult(
    val ok: Boolean,
    val message: String,
    val updatedTxtFileCount: Int = 0,
    val updatedTomlContent: String = "",
    val updatedConfigRelativePath: String = ""
)
