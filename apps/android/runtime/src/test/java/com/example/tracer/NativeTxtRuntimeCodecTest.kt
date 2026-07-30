package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Test

class NativeTxtRuntimeCodecTest {
    private val codec = NativeTxtRuntimeCodec()

    @Test
    fun parseDayMarker_readsNormalizedMarkerFromNativeResponse() {
        val payload = codec.parseDayMarker(
            """{"ok":true,"normalized_day_marker":"0228","error_message":""}"""
        )

        assertTrue(payload.ok)
        assertEquals("0228", payload.normalizedDayMarker)
        assertEquals("", payload.message)
    }

    @Test
    fun parseResolve_readsFoundStateAndIsoDate() {
        val payload = codec.parseResolve(
            """
                {
                  "ok": true,
                  "normalized_day_marker": "0102",
                  "found": true,
                  "is_marker_valid": true,
                  "can_save": true,
                  "day_body": "0656w\n2207minecraft\n",
                  "day_content_iso_date": "2025-01-02",
                  "error_message": ""
                }
            """.trimIndent()
        )

        assertTrue(payload.ok)
        assertEquals("0102", payload.normalizedDayMarker)
        assertTrue(payload.found)
        assertTrue(payload.isMarkerValid)
        assertTrue(payload.canSave)
        assertEquals("0656w\n2207minecraft\n", payload.dayBody)
        assertEquals("2025-01-02", payload.dayContentIsoDate)
    }

    @Test
    fun parseResolve_keepsMissingIsoDateAsNull() {
        val payload = codec.parseResolve(
            """
                {
                  "ok": true,
                  "normalized_day_marker": "0103",
                  "found": false,
                  "is_marker_valid": true,
                  "can_save": false,
                  "day_body": "",
                  "error_message": ""
                }
            """.trimIndent()
        )

        assertTrue(payload.ok)
        assertFalse(payload.found)
        assertTrue(payload.isMarkerValid)
        assertFalse(payload.canSave)
        assertNull(payload.dayContentIsoDate)
    }

    @Test
    fun parseReplace_readsUpdatedContent() {
        val payload = codec.parseReplace(
            """
                {
                  "ok": true,
                  "normalized_day_marker": "0102",
                  "found": true,
                  "is_marker_valid": true,
                  "updated_content": "d0102\n1111work\n",
                  "error_message": ""
                }
            """.trimIndent()
        )

        assertTrue(payload.ok)
        assertEquals("0102", payload.normalizedDayMarker)
        assertTrue(payload.found)
        assertTrue(payload.isMarkerValid)
        assertEquals("d0102\n1111work\n", payload.updatedContent)
    }

    @Test
    fun parseAliasCrossDocumentMove_readsUpdatedDocumentsAndReplacements() {
        val payload = codec.parseActivityHierarchyCrossDocumentOperation(
            """
                {
                  "ok": true,
                  "updated_documents": [
                    {
                      "source_name": "user/activity_hierarchy/exercise.toml",
                      "updated_toml_content": "parent = exercise"
                    },
                    {
                      "source_name": "user/activity_hierarchy/meal.toml",
                      "updated_toml_content": "parent = meal"
                    }
                  ],
                  "replacements": [
                    {"old_canonical":"exercise_go","new_canonical":"meal_go"}
                  ],
                  "alias_replacements": [],
                  "error_message": ""
                }
            """.trimIndent()
        )

        assertTrue(payload.ok)
        assertEquals(2, payload.updatedDocuments.size)
        assertEquals("user/activity_hierarchy/meal.toml", payload.updatedDocuments[1].sourceName)
        assertEquals("meal_go", payload.replacementPlan.canonical.single().newCanonical)
    }

    @Test
    fun parseActivityHierarchyDescribe_consumesCoreNodeKind_withoutChangingPresentationData() {
        val payload = codec.parseActivityHierarchyDescribe(
            """
                {
                  "ok": true,
                  "hierarchy": {
                    "parent": "exercise",
                    "nodes": [
                      {
                        "canonical_key": "walk",
                        "path": "walk",
                        "kind": "leaf",
                        "aliases": ["步行"],
                        "children": []
                      },
                      {
                        "canonical_key": "cardio",
                        "path": "cardio",
                        "kind": "group",
                        "aliases": ["有氧"],
                        "children": [
                          {
                            "canonical_key": "swimming",
                            "path": "cardio.swimming",
                            "kind": "leaf",
                            "aliases": ["游泳"],
                            "children": []
                          }
                        ]
                      }
                    ]
                  },
                  "error_message": ""
                }
            """.trimIndent()
        )

        val hierarchy = payload.hierarchy!!
        assertEquals(ActivityHierarchyNodeKind.LEAF, hierarchy.nodes[0].kind)
        assertEquals(ActivityHierarchyNodeKind.GROUP, hierarchy.nodes[1].kind)
        assertTrue(hierarchy.nodes[1].isGroup)
        assertEquals("swimming", hierarchy.nodes[1].children.single().canonicalKey)
        assertEquals(listOf("游泳"), hierarchy.nodes[1].children.single().aliases)

        val presentation = hierarchy.toActivityAliasDocument()
        assertEquals("walk", (presentation.nodes[0] as ActivityAlias).canonicalLeaf)
        val cardio = presentation.nodes[1] as ActivityCategory
        assertEquals("cardio", cardio.name)
        assertEquals("swimming", (cardio.nodes.single() as ActivityAlias).canonicalLeaf)
    }

    @Test
    fun parseResolve_returnsFailurePayloadWhenJsonIsInvalid() {
        val payload = codec.parseResolve("not-json")

        assertFalse(payload.ok)
        assertFalse(payload.found)
        assertFalse(payload.isMarkerValid)
        assertEquals("Invalid native TXT response.", payload.message)
    }
}
