package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ActivityAliasMigrationUseCaseTest {
    @Test
    fun applies_core_result_as_one_runtime_migration_request() = runTest {
        val gateway = FakeMigrationGateway()

        val outcome = ActivityAliasMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "aliases/recreation.toml",
            updatedTomlContent = "parent = \"recreation\"\n",
            replacements = listOf(CanonicalActivityNameReplacement("上网", "网上活动"))
        )

        assertTrue(outcome is ActivityAliasMigrationOutcome.Applied)
        assertEquals("aliases/recreation.toml", gateway.request?.configRelativePath)
        assertEquals("parent = \"recreation\"\n", requireNotNull(gateway.request).updatedTomlContent)
    }

    @Test
    fun propagates_failed_runtime_migration() = runTest {
        val gateway = FakeMigrationGateway(
            AliasEntryMoveMigrationResult(ok = false, message = "migration failed")
        )
        val outcome = ActivityAliasMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "aliases/recreation.toml",
            updatedTomlContent = "invalid",
            replacements = emptyList()
        )

        assertTrue(outcome is ActivityAliasMigrationOutcome.Invalid)
        assertEquals("migration failed", (outcome as ActivityAliasMigrationOutcome.Invalid).message)
    }

    @Test
    fun alias_replacements_are_delegated_to_core_migration_request() = runTest {
        val gateway = FakeMigrationGateway()

        ActivityAliasMigrationUseCase(gateway).applyCoreResult(
            configRelativePath = "aliases/exercise.toml",
            updatedTomlContent = "parent = \"exercise\"\n",
            replacements = emptyList(),
            aliasReplacements = listOf(AliasKeyReplacement("有氧", "有氧aa"))
        )

        assertEquals(
            AliasKeyReplacement("有氧", "有氧aa"),
            requireNotNull(gateway.request).aliasReplacements.single()
        )
    }
}

private class FakeMigrationGateway(
    private val result: AliasEntryMoveMigrationResult =
        AliasEntryMoveMigrationResult(ok = true, message = "ok", updatedTxtFileCount = 1)
) : AliasMoveMigrationGateway {
    var request: AliasEntryMoveMigrationRequest? = null

    override suspend fun applyAliasEntryMoveMigration(
        request: AliasEntryMoveMigrationRequest
    ): AliasEntryMoveMigrationResult {
        this.request = request
        return result
    }
}
