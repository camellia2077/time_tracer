package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONArray
import org.json.JSONObject

internal class RuntimeAliasHierarchyService(
    private val initializeRuntimeInternal: () -> NativeCallResult,
    private val nativeConfig: (String) -> String,
    private val codec: NativeTxtRuntimeCodec
) {
    suspend fun create(parent: String): AliasHierarchyCreateResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) return@withContext AliasHierarchyCreateResult(false, message = "native init failed")
        codec.parseAliasHierarchyCreate(nativeConfig(JSONObject()
            .put("action", "create_alias_hierarchy_document")
            .put("parent", parent)
            .toString()))
    }

    suspend fun validate(
        documents: List<AliasHierarchyDocumentInput>
    ): AliasHierarchyValidationResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext AliasHierarchyValidationResult(false, "native init failed")
        }
        val encodedDocuments = JSONArray()
        documents.forEach { document ->
            encodedDocuments.put(JSONObject()
                .put("source_name", document.sourceName)
                .put("toml_content", document.tomlContent))
        }
        codec.parseAliasHierarchyValidation(nativeConfig(JSONObject()
            .put("action", "validate_alias_hierarchy_documents")
            .put("documents", encodedDocuments)
            .toString()))
    }

    suspend fun describe(tomlContent: String): AliasHierarchyDescribeResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext AliasHierarchyDescribeResult(
                ok = false,
                message = "native init failed"
            )
        }
        val payload = JSONObject()
            .put("action", "describe_alias_hierarchy")
            .put("toml_content", tomlContent)
        codec.parseAliasHierarchyDescribe(nativeConfig(payload.toString()))
    }

    suspend fun apply(
        tomlContent: String,
        operation: AliasHierarchyOperation
    ): AliasHierarchyOperationResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext AliasHierarchyOperationResult(
                ok = false,
                updatedTomlContent = tomlContent,
                replacements = emptyList(),
                message = "native init failed"
            )
        }
        val payload = JSONObject()
            .put("action", "apply_alias_hierarchy_operation")
            .put("toml_content", tomlContent)
            .put("operation", JSONObject()
                .put("kind", operation.kind)
                .put("target_path", operation.targetPath)
                .put("destination_path", operation.destinationPath)
                .put("canonical_key", operation.canonicalKey)
                .put("new_name", operation.newName)
                .put("target_alias", operation.targetAlias)
                .put("old_alias", operation.oldAlias)
                .put("aliases", JSONArray(operation.aliases)))
        codec.parseAliasHierarchyOperation(nativeConfig(payload.toString()), tomlContent)
    }

    suspend fun rewrite(
        originalTomlContent: String,
        updatedTomlContent: String
    ): AliasHierarchyOperationResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext AliasHierarchyOperationResult(
                ok = false,
                updatedTomlContent = updatedTomlContent,
                replacements = emptyList(),
                message = "native init failed"
            )
        }
        val payload = JSONObject()
            .put("action", "rewrite_alias_hierarchy_document")
            .put("original_toml_content", originalTomlContent)
            .put("updated_toml_content", updatedTomlContent)
        codec.parseAliasHierarchyOperation(nativeConfig(payload.toString()), updatedTomlContent)
    }
}
