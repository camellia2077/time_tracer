package com.example.tracer

import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class TracerScreenTomlImportTest {
    @Test
    fun supportedUserConfigTomlImportPath_acceptsOnlyOwnedConfigFiles() {
        assertTrue(isSupportedUserConfigTomlImportPath("user/behavior.toml"))
        assertTrue(
            isSupportedUserConfigTomlImportPath(
                "user/activity_hierarchy/study.TOML"
            )
        )
        assertFalse(isSupportedUserConfigTomlImportPath("user/charts.toml"))
        assertFalse(isSupportedUserConfigTomlImportPath("program/charts/heatmap.toml"))
    }
}
