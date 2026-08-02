package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class RuntimeActivityHierarchyAutoRegistrarTest {
    @Test
    fun wakeKeywordActivity_isNotRegisteredAsHierarchyActivity() {
        assertEquals(
            true,
            isWakeKeywordActivity(" w ", listOf("起床", "w", "wake"))
        )
        assertEquals(
            false,
            isWakeKeywordActivity("study", listOf("起床", "w", "wake"))
        )
    }

    @Test
    fun activityHierarchyTomlFileName_followsLanguageAndStaysStable() {
        assertEquals("default", activityHierarchyTomlFileName("en"))
        assertEquals("默认", activityHierarchyTomlFileName("zh-CN"))
        assertEquals("デフォルト", activityHierarchyTomlFileName("ja"))
        assertEquals("default", activityHierarchyTomlFileName("fr"))
    }
}
