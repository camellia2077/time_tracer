package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ActivityHierarchyEditCoordinatorTest {
    @Test
    fun applies_core_result_before_persisting_runtime_migration() = runTest {
        val core = FakeActivityHierarchyGateway(
            operationResult = ActivityHierarchyOperationResult(
                ok = true,
                updatedTomlContent = "parent = \"work\"\n",
                hierarchy = ActivityHierarchySnapshot(parent = "work", nodes = emptyList())
            )
        )
        val migration = FakeCoordinatorMigrationGateway(
            result = ActivityHierarchyMigrationResult(
                ok = true,
                message = "ok",
                updatedTomlContent = "rendered by runtime"
            )
        )

        val outcome = ActivityHierarchyEditCoordinator(
            hierarchyGateway = core,
            migrationUseCase = ActivityHierarchyMigrationUseCase(migration)
        ).apply(
            ActivityHierarchyEditRequest(
                configRelativePath = "user/activity_hierarchy/work.toml",
                tomlContent = "old",
                operation = ActivityHierarchyOperation(
                    kind = ActivityHierarchyOperationKind.RENAME_PARENT,
                    oldParent = "old",
                    newName = "work"
                )
            )
        )

        assertTrue(outcome is ActivityHierarchyEditOutcome.Applied)
        assertEquals("old", core.receivedTomlContent)
        assertEquals(
            "parent = \"work\"\n",
            requireNotNull(migration.request).updatedTomlContent
        )
        val applied = outcome as ActivityHierarchyEditOutcome.Applied
        assertEquals("work", applied.document.parent)
        assertEquals("rendered by runtime", applied.renderedToml)
    }

    @Test
    fun does_not_persist_when_core_rejects_the_operation() = runTest {
        val core = FakeActivityHierarchyGateway(
            operationResult = ActivityHierarchyOperationResult(
                ok = false,
                updatedTomlContent = "",
                message = "invalid hierarchy"
            )
        )
        val migration = FakeCoordinatorMigrationGateway()

        val outcome = ActivityHierarchyEditCoordinator(
            hierarchyGateway = core,
            migrationUseCase = ActivityHierarchyMigrationUseCase(migration)
        ).apply(
            ActivityHierarchyEditRequest(
                configRelativePath = "user/activity_hierarchy/work.toml",
                tomlContent = "old",
                operation = ActivityHierarchyOperation(ActivityHierarchyOperationKind.ADD_GROUP)
            )
        )

        assertEquals(
            ActivityHierarchyEditOutcome.Failed("invalid hierarchy"),
            outcome
        )
        assertEquals(null, migration.request)
    }
}

private class FakeActivityHierarchyGateway(
    private val operationResult: ActivityHierarchyOperationResult
) : ActivityHierarchyGateway {
    var receivedTomlContent: String? = null

    override suspend fun describeActivityHierarchy(
        tomlContent: String
    ): ActivityHierarchyDescribeResult = ActivityHierarchyDescribeResult(ok = true)

    override suspend fun validateActivityHierarchyDocuments(
        documents: List<ActivityHierarchyDocumentInput>
    ): ActivityHierarchyValidationResult = ActivityHierarchyValidationResult(ok = true)

    override suspend fun applyActivityHierarchyOperation(
        tomlContent: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyOperationResult {
        receivedTomlContent = tomlContent
        return operationResult
    }

    override suspend fun moveActivityHierarchyNodeBetweenDocuments(
        documents: List<ActivityHierarchyDocumentInput>,
        sourceName: String,
        destinationName: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyCrossDocumentOperationResult =
        ActivityHierarchyCrossDocumentOperationResult(ok = true)

    override suspend fun rewriteActivityHierarchyDocument(
        originalTomlContent: String,
        updatedTomlContent: String
    ): ActivityHierarchyOperationResult = operationResult
}

private class FakeCoordinatorMigrationGateway(
    private val result: ActivityHierarchyMigrationResult =
        ActivityHierarchyMigrationResult(ok = true, message = "ok")
) : ActivityHierarchyMigrationGateway {
    var request: ActivityHierarchyMigrationRequest? = null

    override suspend fun applyActivityHierarchyMigration(
        request: ActivityHierarchyMigrationRequest
    ): ActivityHierarchyMigrationResult {
        this.request = request
        return result
    }
}
