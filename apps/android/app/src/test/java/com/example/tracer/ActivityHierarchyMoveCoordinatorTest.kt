package com.example.tracer

import kotlinx.coroutines.test.runTest
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ActivityHierarchyMoveCoordinatorTest {
    @Test
    fun previews_same_document_leaf_move_without_persisting() = runTest {
        val gateway = FakeMoveGateway(
            operationResult = ActivityHierarchyOperationResult(
                ok = true,
                updatedTomlContent = "updated",
                replacementPlan = ActivityNameReplacementPlan(
                    canonical = listOf(CanonicalActivityNameReplacement("A.foo", "B.foo"))
                )
            )
        )
        val state = moveState()

        val outcome = ActivityHierarchyMoveCoordinator(
            configGateway = EmptyConfigGateway,
            activityHierarchyGateway = gateway
        ).previewEntryMove(
            state = state,
            entryId = "root.A.foo",
            targetGroupId = "root.B"
        )

        assertTrue(outcome is ActivityHierarchyMovePreviewOutcome.Ready)
        assertEquals("A.foo", gateway.operation?.targetPath)
        assertEquals("B", gateway.operation?.destinationPath)
        assertEquals(null, gateway.crossDocumentRequest)
    }

    @Test
    fun rejects_failed_core_preview_without_creating_a_move_plan() = runTest {
        val gateway = FakeMoveGateway(
            operationResult = ActivityHierarchyOperationResult(
                ok = false,
                updatedTomlContent = "",
                message = "invalid move"
            )
        )

        val outcome = ActivityHierarchyMoveCoordinator(
            configGateway = EmptyConfigGateway,
            activityHierarchyGateway = gateway
        ).previewEntryMove(
            state = moveState(),
            entryId = "root.A.foo",
            targetGroupId = "root.B"
        )

        assertEquals(
            ActivityHierarchyMovePreviewOutcome.Failed("invalid move"),
            outcome
        )
    }

    private fun moveState(): ConfigUiState {
        val leaf = ActivityHierarchyNode(
            canonicalKey = "foo",
            path = "root.A.foo",
            kind = ActivityHierarchyNodeKind.LEAF,
            aliases = listOf("foo"),
            children = emptyList()
        )
        val sourceGroup = ActivityHierarchyNode(
            canonicalKey = "A",
            path = "root.A",
            kind = ActivityHierarchyNodeKind.GROUP,
            aliases = emptyList(),
            children = listOf(leaf)
        )
        val targetGroup = ActivityHierarchyNode(
            canonicalKey = "B",
            path = "root.B",
            kind = ActivityHierarchyNodeKind.GROUP,
            aliases = emptyList(),
            children = emptyList()
        )
        return ConfigUiState(
            aliasFiles = listOf(ConfigTomlFileEntry("user/activity_hierarchy/work.toml", "work.toml")),
            selectedFilePath = "user/activity_hierarchy/work.toml",
            selectedFileContent = "toml",
            aliasDocumentDraft = ActivityHierarchySnapshot(
                parent = "work",
                nodes = listOf(sourceGroup, targetGroup)
            ).toActivityHierarchyDocument()
        )
    }
}

private object EmptyConfigGateway : ConfigGateway {
    override suspend fun listConfigTomlFiles(): ConfigTomlListResult =
        ConfigTomlListResult(false, emptyList(), emptyList(), emptyList(), emptyList(), "unused")

    override suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(false, relativePath, "", "unused")

    override suspend fun saveConfigTomlFile(relativePath: String, content: String): TxtFileContentResult =
        TxtFileContentResult(false, relativePath, "", "unused")

    override suspend fun deleteConfigTomlFile(relativePath: String): TxtFileContentResult =
        TxtFileContentResult(false, relativePath, "", "unused")

    override suspend fun listRecentDiagnostics(limit: Int): RuntimeDiagnosticsListResult =
        RuntimeDiagnosticsListResult(false, emptyList(), "unused")

    override suspend fun buildDiagnosticsPayload(maxEntries: Int): RuntimeDiagnosticsPayloadResult =
        RuntimeDiagnosticsPayloadResult(false, "", "unused")
}

private class FakeMoveGateway(
    private val operationResult: ActivityHierarchyOperationResult
) : ActivityHierarchyGateway {
    var operation: ActivityHierarchyOperation? = null
    var crossDocumentRequest: List<ActivityHierarchyDocumentInput>? = null

    override suspend fun describeActivityHierarchy(tomlContent: String): ActivityHierarchyDescribeResult =
        ActivityHierarchyDescribeResult(false, message = "unused")

    override suspend fun validateActivityHierarchyDocuments(
        documents: List<ActivityHierarchyDocumentInput>
    ): ActivityHierarchyValidationResult = ActivityHierarchyValidationResult(false, "unused")

    override suspend fun applyActivityHierarchyOperation(
        tomlContent: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyOperationResult {
        this.operation = operation
        return operationResult
    }

    override suspend fun moveActivityHierarchyNodeBetweenDocuments(
        documents: List<ActivityHierarchyDocumentInput>,
        sourceName: String,
        destinationName: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyCrossDocumentOperationResult {
        crossDocumentRequest = documents
        return ActivityHierarchyCrossDocumentOperationResult(false, message = "unused")
    }

    override suspend fun rewriteActivityHierarchyDocument(
        originalTomlContent: String,
        updatedTomlContent: String
    ): ActivityHierarchyOperationResult = operationResult
}
