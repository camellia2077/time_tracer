package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ActivityHierarchyMigrationUseCaseTest {
    @Test
    fun applies_core_result_as_one_runtime_migration_request() = runTest {
        val gateway = FakeMigrationGateway()

        val outcome = ActivityHierarchyMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "user/activity_hierarchy/recreation.toml",
            updatedTomlContent = "parent = \"recreation\"\n",
            replacementPlan = ActivityNameReplacementPlan(
                canonical = listOf(CanonicalActivityNameReplacement("上网", "网上活动"))
            )
        )

        assertTrue(outcome is ActivityHierarchyMigrationOutcome.Applied)
        assertEquals("user/activity_hierarchy/recreation.toml", gateway.request?.configRelativePath)
        assertEquals("parent = \"recreation\"\n", requireNotNull(gateway.request).updatedTomlContent)
    }

    @Test
    fun propagates_failed_runtime_migration() = runTest {
        val gateway = FakeMigrationGateway(
            ActivityHierarchyMigrationResult(ok = false, message = "migration failed")
        )
        val outcome = ActivityHierarchyMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "user/activity_hierarchy/recreation.toml",
            updatedTomlContent = "invalid",
            replacementPlan = ActivityNameReplacementPlan()
        )

        assertTrue(outcome is ActivityHierarchyMigrationOutcome.Invalid)
        assertEquals("migration failed", (outcome as ActivityHierarchyMigrationOutcome.Invalid).message)
    }

    @Test
    fun alias_replacements_are_delegated_to_core_migration_request() = runTest {
        val gateway = FakeMigrationGateway()

        ActivityHierarchyMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "user/activity_hierarchy/exercise.toml",
            updatedTomlContent = "parent = \"exercise\"\n",
            replacementPlan = ActivityNameReplacementPlan(
                aliases = listOf(AliasKeyReplacement("有氧", "有氧aa"))
            )
        )

        assertEquals(
            AliasKeyReplacement("有氧", "有氧aa"),
            requireNotNull(gateway.request).replacementPlan.aliases.single()
        )
    }

    @Test
    fun cross_document_updates_are_delegated_as_one_migration_request() = runTest {
        val gateway = FakeMigrationGateway()

        ActivityHierarchyMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "user/activity_hierarchy/exercise.toml",
            updatedTomlContent = "source",
            replacementPlan = ActivityNameReplacementPlan(
                canonical = listOf(CanonicalActivityNameReplacement("exercise_go", "meal_go"))
            ),
            updatedDocuments = listOf(
                ActivityHierarchyDocumentInput("user/activity_hierarchy/exercise.toml", "source"),
                ActivityHierarchyDocumentInput("user/activity_hierarchy/meal.toml", "destination")
            )
        )

        assertEquals(
            listOf("user/activity_hierarchy/exercise.toml", "user/activity_hierarchy/meal.toml"),
            requireNotNull(gateway.request).updatedDocuments.map { it.sourceName }
        )
    }

    @Test
    fun canonical_and_alias_replacements_share_one_migration_plan() = runTest {
        val gateway = FakeMigrationGateway()

        ActivityHierarchyMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "user/activity_hierarchy/exercise.toml",
            updatedTomlContent = "parent = \"exercise-renamed\"\n",
            replacementPlan = ActivityNameReplacementPlan(
                canonical = listOf(CanonicalActivityNameReplacement("exercise_go", "meal_go")),
                aliases = listOf(AliasKeyReplacement("旧名称", "新名称"))
            )
        )

        val plan = requireNotNull(gateway.request).replacementPlan
        assertEquals("exercise_go", plan.canonical.single().oldCanonical)
        assertEquals("新名称", plan.aliases.single().newAlias)
    }

    @Test
    fun applies_alias_replacements_to_quick_access_once_after_runtime_commit() = runTest {
        val gateway = FakeMigrationGateway()
        val quickActivities = FakeQuickActivitiesPreferenceGateway(listOf("旧名称", "固定入口"))

        val outcome = ActivityHierarchyMigrationUseCase(
            gateway = gateway,
            quickActivitiesPreferenceGateway = quickActivities
        ).applyCoreResult(
            configRelativePath = "user/activity_hierarchy/exercise.toml",
            updatedTomlContent = "parent = \"exercise\"\n",
            replacementPlan = ActivityNameReplacementPlan(
                aliases = listOf(AliasKeyReplacement("旧名称", "新名称"))
            )
        )

        assertTrue(outcome is ActivityHierarchyMigrationOutcome.Applied)
        assertEquals(listOf("新名称", "固定入口"), quickActivities.quickActivities)
    }
}

private class FakeMigrationGateway(
    private val result: ActivityHierarchyMigrationResult =
        ActivityHierarchyMigrationResult(ok = true, message = "ok", updatedTxtFileCount = 1)
) : ActivityHierarchyMigrationGateway {
    var request: ActivityHierarchyMigrationRequest? = null

    override suspend fun applyActivityHierarchyMigration(
        request: ActivityHierarchyMigrationRequest
    ): ActivityHierarchyMigrationResult {
        this.request = request
        return result
    }
}
