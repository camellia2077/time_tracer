package com.example.tracer

import kotlinx.coroutines.runBlocking
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeActivityHierarchyServiceTest {
    @Test
    fun moveNodeBetweenDocuments_forwardsGroupSubtreeOperationToCore() = runBlocking {
        var request = JSONObject()
        val service = RuntimeActivityHierarchyService(
            initializeRuntimeInternal = {
                NativeCallResult(initialized = true, operationOk = true, rawResponse = "{}")
            },
            nativeConfig = { requestJson ->
                request = JSONObject(requestJson)
                JSONObject()
                    .put("ok", true)
                    .put("updated_documents", JSONArray())
                    .put(
                        "replacements",
                        listOf(
                            JSONObject()
                                .put("old_canonical", "exercise_cardio")
                                .put("new_canonical", "meal_cardio")
                        )
                    )
                    .put("alias_replacements", JSONArray())
                    .put("error_message", "")
                    .toString()
            },
            codec = NativeTxtRuntimeCodec()
        )

        val result = service.moveNodeBetweenDocuments(
            documents = listOf(
                ActivityHierarchyDocumentInput("user/activity_hierarchy/exercise.toml", "source"),
                ActivityHierarchyDocumentInput("user/activity_hierarchy/meal.toml", "destination")
            ),
            sourceName = "user/activity_hierarchy/exercise.toml",
            destinationName = "user/activity_hierarchy/meal.toml",
            operation = ActivityHierarchyOperation(
                kind = ActivityHierarchyOperationKind.MOVE_GROUP,
                targetPath = "cardio",
                destinationPath = "root"
            )
        )

        assertTrue(result.ok)
        assertEquals(
            "move_activity_hierarchy_node_between_documents",
            request.getString("action")
        )
        assertEquals("move_group", request.getJSONObject("operation").getString("kind"))
        assertEquals("cardio", request.getJSONObject("operation").getString("target_path"))
        assertEquals("root", request.getJSONObject("operation").getString("destination_path"))
    }

    @Test
    fun apply_forwardsRenameParentGuardToCore() = runBlocking {
        var request = JSONObject()
        val service = RuntimeActivityHierarchyService(
            initializeRuntimeInternal = {
                NativeCallResult(initialized = true, operationOk = true, rawResponse = "{}")
            },
            nativeConfig = { requestJson ->
                request = JSONObject(requestJson)
                JSONObject()
                    .put("ok", true)
                    .put("updated_toml_content", "parent = training")
                    .put("replacements", JSONArray())
                    .put("alias_replacements", JSONArray())
                    .put("hierarchy", JSONObject().put("parent", "training"))
                    .toString()
            },
            codec = NativeTxtRuntimeCodec()
        )

        val result = service.apply(
            tomlContent = "parent = exercise",
            operation = ActivityHierarchyOperation(
                kind = ActivityHierarchyOperationKind.RENAME_PARENT,
                oldParent = "exercise",
                newName = "training"
            )
        )

        assertTrue(result.ok)
        assertEquals(
            "exercise",
            request.getJSONObject("operation").getString("old_parent")
        )
        assertEquals(
            "training",
            request.getJSONObject("operation").getString("new_name")
        )
    }
}
