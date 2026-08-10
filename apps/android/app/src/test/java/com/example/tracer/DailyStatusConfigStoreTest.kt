package com.example.tracer

import com.example.tracer.data.DailyStatusConfigStore
import org.junit.Assert.assertEquals
import org.junit.Test

class DailyStatusConfigStoreTest {
    @Test
    fun idForParent_replacesHierarchySeparators() {
        assertEquals(
            "study__math",
            DailyStatusConfigStore.idForParent("study/math")
        )
    }
}
