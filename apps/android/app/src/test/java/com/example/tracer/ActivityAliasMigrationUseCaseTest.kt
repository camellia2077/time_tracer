package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ActivityAliasMigrationUseCaseTest {
    @Test
    fun applies_validated_document_as_one_runtime_migration_request() = runTest {
        val gateway = FakeMigrationGateway()
        val document = ActivityAliasDocument(
            parent = "recreation",
            nodes = listOf(ActivityCategory(name = "online", groupAliases = listOf("上网")))
        )

        val outcome = ActivityAliasMigrationUseCase(gateway).apply(
            configRelativePath = "aliases/recreation.toml",
            updatedDocument = document,
            replacements = listOf(CanonicalActivityNameReplacement("上网", "网上活动"))
        )

        assertTrue(outcome is ActivityAliasMigrationOutcome.Applied)
        assertEquals("aliases/recreation.toml", gateway.request?.configRelativePath)
        assertTrue(requireNotNull(gateway.request).updatedTomlContent.contains("group_aliases = [\"上网\"]"))
    }

    @Test
    fun rejects_invalid_document_without_starting_migration() = runTest {
        val gateway = FakeMigrationGateway()
        val outcome = ActivityAliasMigrationUseCase(gateway).apply(
            configRelativePath = "aliases/recreation.toml",
            updatedDocument = ActivityAliasDocument(parent = "", nodes = emptyList()),
            replacements = emptyList()
        )

        assertTrue(outcome is ActivityAliasMigrationOutcome.Invalid)
        assertEquals(null, gateway.request)
    }
}

private class FakeMigrationGateway : AliasMoveMigrationGateway {
    var request: AliasEntryMoveMigrationRequest? = null

    override suspend fun applyAliasEntryMoveMigration(
        request: AliasEntryMoveMigrationRequest
    ): AliasEntryMoveMigrationResult {
        this.request = request
        return AliasEntryMoveMigrationResult(ok = true, message = "ok", updatedTxtFileCount = 1)
    }
}
