package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class TracerScreenExportTracerTest {
    @Test
    fun buildConfigTomlExportEntries_exportsAllUserConfigFiles() {
        val result = buildConfigTomlExportEntries(
            listOf(
                "user/behavior.toml",
                "user/activity_hierarchy/study.toml",
                "user/charts.toml",
                "user/heatmap.toml",
                "user/heatmap.toml",
                "program/charts/heatmap.toml"
            )
        )

        assertEquals(
            listOf(
                ConfigTomlExportEntry("user/activity_hierarchy/study.toml", "user/activity_hierarchy/study.toml"),
                ConfigTomlExportEntry("user/behavior.toml", "user/behavior.toml"),
                ConfigTomlExportEntry("user/charts.toml", "user/charts.toml"),
                ConfigTomlExportEntry("user/heatmap.toml", "user/heatmap.toml")
            ),
            result
        )
    }
}
