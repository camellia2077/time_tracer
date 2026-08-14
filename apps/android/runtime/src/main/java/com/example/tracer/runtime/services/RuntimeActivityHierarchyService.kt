package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONArray
import org.json.JSONObject

internal class RuntimeActivityHierarchyService(
    private val initializeRuntimeInternal: () -> NativeCallResult,
    private val nativeConfig: (String) -> String,
    private val codec: NativeTxtRuntimeCodec
) {
    suspend fun validate(
        documents: List<ActivityHierarchyDocumentInput>
    ): ActivityHierarchyValidationResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext ActivityHierarchyValidationResult(false, "native init failed")
        }
        val encodedDocuments = JSONArray()
        documents.forEach { document ->
            encodedDocuments.put(JSONObject()
                .put("source_name", document.sourceName)
                .put("toml_content", document.tomlContent))
        }
        codec.parseActivityHierarchyValidation(nativeConfig(JSONObject()
            .put("action", "validate_activity_hierarchy_documents")
            .put("documents", encodedDocuments)
            .toString()))
    }

    suspend fun describe(tomlContent: String): ActivityHierarchyDescribeResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext ActivityHierarchyDescribeResult(
                ok = false,
                message = "native init failed"
            )
        }
        val payload = JSONObject()
            .put("action", "describe_activity_hierarchy")
            .put("toml_content", tomlContent)
        codec.parseActivityHierarchyDescribe(nativeConfig(payload.toString()))
    }

    suspend fun apply(
        tomlContent: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyOperationResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext ActivityHierarchyOperationResult(
                ok = false,
                updatedTomlContent = tomlContent,
                message = "native init failed"
            )
        }
        val payload = JSONObject()
            .put("action", "apply_activity_hierarchy_operation")
            .put("toml_content", tomlContent)
            .put("operation", JSONObject()
                .put("kind", operation.kind.wireValue)
                .put("target_path", operation.targetPath)
                .put("destination_path", operation.destinationPath)
                .put("canonical_key", operation.canonicalKey)
                .put("new_name", operation.newName)
                .put("old_parent", operation.oldParent)
                .put("color", operation.color.ifBlank { JSONObject.NULL })
                .put("target_alias", operation.targetAlias)
                .put("old_alias", operation.oldAlias)
                .put("aliases", JSONArray(operation.aliases)))
        codec.parseActivityHierarchyOperation(nativeConfig(payload.toString()), tomlContent)
    }

    suspend fun moveNodeBetweenDocuments(
        documents: List<ActivityHierarchyDocumentInput>,
        sourceName: String,
        destinationName: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyCrossDocumentOperationResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext ActivityHierarchyCrossDocumentOperationResult(
                ok = false,
                message = "native init failed"
            )
        }
        val encodedDocuments = JSONArray()
        documents.forEach { document ->
            encodedDocuments.put(JSONObject()
                .put("source_name", document.sourceName)
                .put("toml_content", document.tomlContent))
        }
        val payload = JSONObject()
            .put("action", "move_activity_hierarchy_node_between_documents")
            .put("source_name", sourceName)
            .put("destination_name", destinationName)
            .put("documents", encodedDocuments)
            .put("operation", JSONObject()
                .put("kind", operation.kind.wireValue)
                .put("target_path", operation.targetPath)
                .put("destination_path", operation.destinationPath)
                .put("target_alias", operation.targetAlias))
        codec.parseActivityHierarchyCrossDocumentOperation(nativeConfig(payload.toString()))
    }

    suspend fun rewrite(
        originalTomlContent: String,
        updatedTomlContent: String
    ): ActivityHierarchyOperationResult = withContext(Dispatchers.IO) {
        val initialized = initializeRuntimeInternal()
        if (!initialized.operationOk) {
            return@withContext ActivityHierarchyOperationResult(
                ok = false,
                updatedTomlContent = updatedTomlContent,
                message = "native init failed"
            )
        }
        val payload = JSONObject()
            .put("action", "rewrite_activity_hierarchy_document")
            .put("original_toml_content", originalTomlContent)
            .put("updated_toml_content", updatedTomlContent)
        codec.parseActivityHierarchyOperation(nativeConfig(payload.toString()), updatedTomlContent)
    }
}
