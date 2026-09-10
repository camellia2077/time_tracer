package com.example.tracer

interface ActivityHierarchyGateway {
    suspend fun describeActivityHierarchy(tomlContent: String): ActivityHierarchyDescribeResult

    /** Core-owned case-insensitive contains search that retains ancestor groups. */
    suspend fun searchActivityHierarchy(
        tomlContent: String,
        query: String
    ): ActivityHierarchyDescribeResult = describeActivityHierarchy(tomlContent)

    suspend fun validateActivityHierarchyDocuments(
        documents: List<ActivityHierarchyDocumentInput>
    ): ActivityHierarchyValidationResult

    suspend fun applyActivityHierarchyOperation(
        tomlContent: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyOperationResult

    suspend fun moveActivityHierarchyNodeBetweenDocuments(
        documents: List<ActivityHierarchyDocumentInput>,
        sourceName: String,
        destinationName: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyCrossDocumentOperationResult

    suspend fun rewriteActivityHierarchyDocument(
        originalTomlContent: String,
        updatedTomlContent: String
    ): ActivityHierarchyOperationResult
}
