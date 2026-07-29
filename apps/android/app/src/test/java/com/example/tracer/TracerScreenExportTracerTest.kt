package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class TracerScreenExportTracerTest {
    @Test
    fun buildConfigTomlExportEntries_normalizesAndDeduplicatesAliasFiles() {
        val result = buildConfigTomlExportEntries(
            listOf(
                "break.toml",
                "activity_hierarchy/break.toml",
                "activity_hierarchy/study.toml",
                "activity_hierarchy/study.toml",
                "charts/heatmap.toml",
                "meta/bundle.toml",
                "reports/markdown/en/day.toml",
                "config.toml"
            )
        )

        assertEquals(
            listOf(
                ConfigTomlExportEntry("activity_hierarchy/break.toml", "activity_hierarchy/break.toml"),
                ConfigTomlExportEntry("activity_hierarchy/study.toml", "activity_hierarchy/study.toml"),
                ConfigTomlExportEntry("charts/heatmap.toml", "charts/heatmap.toml"),
                ConfigTomlExportEntry("config.toml", "config.toml"),
                ConfigTomlExportEntry("meta/bundle.toml", "meta/bundle.toml"),
                ConfigTomlExportEntry(
                    "reports/markdown/en/day.toml",
                    "reports/markdown/en/day.toml"
                )
            ),
            result
        )
    }
}
