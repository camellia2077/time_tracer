package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class TracerScreenExportTracerTest {
    @Test
    fun buildConfigTomlExportEntries_exportsOnlyAndroidSupportedUserConfig() {
        val result = buildConfigTomlExportEntries(
            listOf(
                "user/behavior.toml",
                "user/activity_hierarchy/study.toml",
                "user/charts.toml",
                "user/heatmap.toml",
                "user/insights.toml",
                "user/unsupported.toml",
                "user/unsupported.toml",
                "program/charts/heatmap.toml"
            )
        )

        assertEquals(
            listOf(
                ConfigTomlExportEntry("user/activity_hierarchy/study.toml", "user/activity_hierarchy/study.toml"),
                ConfigTomlExportEntry("user/behavior.toml", "user/behavior.toml")
            ),
            result
        )
    }
}
