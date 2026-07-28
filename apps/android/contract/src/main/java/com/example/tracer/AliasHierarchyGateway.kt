package com.example.tracer

interface AliasHierarchyGateway {
    suspend fun createAliasHierarchyDocument(parent: String): AliasHierarchyCreateResult

    suspend fun describeAliasHierarchy(tomlContent: String): AliasHierarchyDescribeResult

    suspend fun validateAliasHierarchyDocuments(
        documents: List<AliasHierarchyDocumentInput>
    ): AliasHierarchyValidationResult

    suspend fun applyAliasHierarchyOperation(
        tomlContent: String,
        operation: AliasHierarchyOperation
    ): AliasHierarchyOperationResult

    suspend fun rewriteAliasHierarchyDocument(
        originalTomlContent: String,
        updatedTomlContent: String
    ): AliasHierarchyOperationResult
}
