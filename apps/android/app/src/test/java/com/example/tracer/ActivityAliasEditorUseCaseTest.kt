package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ActivityAliasEditorUseCaseTest {
    @Test
    fun promotion_and_move_plans_stay_with_the_pure_editor_use_case() {
        val walking = ActivityAlias(aliasKey = "散步", canonicalLeaf = "walk")
        val cardio = ActivityCategory(name = "cardio")
        val document = ActivityAliasDocument(parent = "exercise", nodes = listOf(walking, cardio))
        val useCase = ActivityAliasEditorUseCase()

        val promotion = useCase.promoteAlias(document, walking.id)
        assertTrue(promotion is AliasEntryPromotePlanResult.Ready)
        val promoted = useCase.applyPromotion(document, (promotion as AliasEntryPromotePlanResult.Ready).plan)
        assertEquals("walk", promoted.nodes.filterIsInstance<ActivityCategory>().first().name)

        val movePlan = useCase.planMove(document, walking.id, cardio.id)
        assertTrue(movePlan is AliasEntryMovePlanResult.Ready)
        assertEquals("exercise_cardio_walk", (movePlan as AliasEntryMovePlanResult.Ready).plan.newCanonical)
    }
}
