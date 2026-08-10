package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeAssetBootstrapperTest {
    @Test
    fun runtimeAssetBootstrapEntries_refreshesProgramButPreservesUserFiles() {
        val entries = runtimeAssetBootstrapEntries("config")

        assertEquals(
            listOf("config/program", "config/user"),
            entries.map(RuntimeAssetBootstrapEntry::targetRelativePath)
        )
        assertTrue(entries.first { it.targetRelativePath == "config/program" }.overwriteExistingFiles)
        assertFalse(entries.first { it.targetRelativePath == "config/user" }.overwriteExistingFiles)
    }
}
