package com.example.tracer

internal enum class ConfigCategory {
    ALIAS,
    CHARTS,
    META,
    REPORTS
}

internal enum class ConfigAutoSaveStatus {
    IDLE,
    SAVING,
    SAVED,
    FAILED
}

internal data class ConfigUiState(
    val selectedCategory: ConfigCategory = ConfigCategory.ALIAS,
    val aliasFiles: List<ConfigTomlFileEntry> = emptyList(),
    val chartFiles: List<ConfigTomlFileEntry> = emptyList(),
    val metaFiles: List<ConfigTomlFileEntry> = emptyList(),
    val reportFiles: List<ConfigTomlFileEntry> = emptyList(),
    val selectedFilePath: String = "",
    val selectedFileDisplayName: String = "",
    val selectedFileContent: String = "",
    val editableContent: String = "",
    val plainTomlDraftsByFile: Map<String, String> = emptyMap(),
    val aliasEditorMode: AliasEditorMode = AliasEditorMode.STRUCTURED,
    val aliasDocumentDraft: AliasTomlDocument? = null,
    val aliasBaselineDocument: AliasTomlDocument? = null,
    val aliasParentOptions: List<String> = emptyList(),
    val aliasAdvancedTomlDraft: String = "",
    val aliasStructuredDraftsByFile: Map<String, AliasTomlDocument> = emptyMap(),
    val aliasAdvancedDraftsByFile: Map<String, String> = emptyMap(),
    val aliasEditorModeByFile: Map<String, AliasEditorMode> = emptyMap(),
    val aliasEntryMovePlan: AliasEntryMovePlan? = null,
    val aliasEntryMoveDestinations: List<AliasEntryMoveDestinationDocument> = emptyList(),
    val aliasEntryMoveDestinationsLoading: Boolean = false,
    val aliasEditorErrorMessage: String = "",
    val txtReloadRequestVersion: Long = 0L,
    val autoSaveStatus: ConfigAutoSaveStatus = ConfigAutoSaveStatus.IDLE,
    val statusText: String = "Preparing config..."
)

internal object UnavailableActivityHierarchyGateway : ActivityHierarchyGateway {
    private const val MESSAGE = "Activity hierarchy runtime is unavailable."

    override suspend fun describeActivityHierarchy(
        tomlContent: String
    ): ActivityHierarchyDescribeResult = ActivityHierarchyDescribeResult(false, message = MESSAGE)

    override suspend fun validateActivityHierarchyDocuments(
        documents: List<ActivityHierarchyDocumentInput>
    ): ActivityHierarchyValidationResult = ActivityHierarchyValidationResult(false, MESSAGE)

    override suspend fun applyActivityHierarchyOperation(
        tomlContent: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyOperationResult = ActivityHierarchyOperationResult(
        ok = false,
        updatedTomlContent = tomlContent,
        message = MESSAGE
    )

    override suspend fun moveActivityHierarchyNodeBetweenDocuments(
        documents: List<ActivityHierarchyDocumentInput>,
        sourceName: String,
        destinationName: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyCrossDocumentOperationResult =
        ActivityHierarchyCrossDocumentOperationResult(false, message = MESSAGE)

    override suspend fun rewriteActivityHierarchyDocument(
        originalTomlContent: String,
        updatedTomlContent: String
    ): ActivityHierarchyOperationResult = ActivityHierarchyOperationResult(
        ok = false,
        updatedTomlContent = updatedTomlContent,
        message = MESSAGE
    )
}

internal object UnavailableActivityHierarchyMigrationGateway : ActivityHierarchyMigrationGateway {
    override suspend fun applyActivityHierarchyMigration(
        request: ActivityHierarchyMigrationRequest
    ): ActivityHierarchyMigrationResult = ActivityHierarchyMigrationResult(
        ok = false,
        message = "Activity hierarchy migration runtime is unavailable."
    )
}
