package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertTrue
import org.junit.Test

class ActivityHierarchyParentColorEditorTest {
    @Test
    fun input_normalizes_optional_hash_case_and_length() {
        assertEquals("A1B2C3", normalizeParentColorInput("#a1B2c3ff"))
        assertEquals("FF", normalizeParentColorInput("#fF"))
    }

    @Test
    fun input_pads_missing_digits_for_preview_and_storage() {
        assertEquals("#FF0000", parentColorForStorage("ff"))
        assertEquals("#A00000", parentColorForStorage("a"))
        assertTrue(previewParentColor("a") != null)
        assertEquals("", parentColorForStorage(""))
    }
}
