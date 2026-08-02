package com.example.tracer

import com.example.tracer.data.DailyStatusConfig
import com.example.tracer.data.DailyStatusConfigStore
import com.example.tracer.data.DailyStatusDefinition
import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class DailyStatusConfigStoreTest {
    @Test
    fun parsesParentPresentStatuses() {
        val config = DailyStatusConfigStore.parse(
            """
            schema_version = 1

            [daily_statuses.parent_present.exercise]
            parent = "exercise"
            label = "Exercise"

            [daily_statuses.parent_present.study]
            parent = "study/math"
            label = "Study"
            """.trimIndent()
        )

        assertEquals(1, config.schemaVersion)
        assertEquals(
            listOf(
                DailyStatusDefinition("exercise", "Exercise", "exercise"),
                DailyStatusDefinition("study", "Study", "study/math")
            ),
            config.statuses
        )
    }

    @Test
    fun serializesOnlyConfiguredParentPresentStatuses() {
        val raw = DailyStatusConfigStore.serialize(
            DailyStatusConfig(
                statuses = listOf(DailyStatusDefinition("study", "学习", "study"))
            )
        )

        assertTrue(raw.contains("[daily_statuses.parent_present.study]"))
        assertTrue(raw.contains("parent = \"study\""))
        assertTrue(raw.contains("label = \"学习\""))
    }

    @Test
    fun serializesNestedParentAsQuotedStatusKey() {
        val raw = DailyStatusConfigStore.serialize(
            DailyStatusConfig(
                statuses = listOf(DailyStatusDefinition("study__math", "Study", "study/math"))
            )
        )

        assertTrue(raw.contains("[daily_statuses.parent_present.study__math]"))
        assertEquals(
            listOf(DailyStatusDefinition("study__math", "Study", "study/math")),
            DailyStatusConfigStore.parse(raw).statuses
        )
    }
}
