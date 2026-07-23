package com.example.tracer

/** Pure tree-edit operations used by Config; persistence remains outside this use case. */
internal class ActivityAliasEditorUseCase {
    fun updateParent(document: ActivityAliasDocument, value: String): ActivityAliasDocument =
        document.updateParent(value)

    fun addCategory(document: ActivityAliasDocument, parentId: String?, name: String): ActivityAliasDocument =
        document.addGroup(parentId, name)

    fun deleteCategory(document: ActivityAliasDocument, categoryId: String): ActivityAliasDocument =
        document.deleteGroup(categoryId)

    fun addAlias(document: ActivityAliasDocument, parentId: String?, name: String, canonicalLeaf: String): ActivityAliasDocument =
        document.addEntry(parentId, name, canonicalLeaf)

    fun updateAlias(document: ActivityAliasDocument, aliasId: String, name: String, canonicalLeaf: String): ActivityAliasDocument =
        document.updateEntry(aliasId, name, canonicalLeaf)

    fun deleteAlias(document: ActivityAliasDocument, aliasId: String): ActivityAliasDocument =
        document.deleteEntry(aliasId)

    fun promoteAlias(document: ActivityAliasDocument, aliasId: String): AliasEntryPromotePlanResult =
        document.planEntryPromotion(aliasId)

    fun applyPromotion(document: ActivityAliasDocument, plan: AliasEntryPromotePlan): ActivityAliasDocument =
        document.applyEntryPromotePlan(plan)

    fun planMove(document: ActivityAliasDocument, aliasId: String, targetCategoryId: String): AliasEntryMovePlanResult =
        document.planEntryMove(aliasId, targetCategoryId)

    fun applyMove(document: ActivityAliasDocument, plan: AliasEntryMovePlan): ActivityAliasDocument =
        document.applyEntryMovePlan(plan)
}
