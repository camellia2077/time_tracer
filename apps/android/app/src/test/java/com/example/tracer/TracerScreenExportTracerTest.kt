package com.example.tracer

import org.junit.Assert.assertEquals
import org.junit.Test

class TracerScreenExportTracerTest {
    @Test
    fun buildConfigTomlExportEntries_normalizesAndDeduplicatesAliasFiles() {
        val result = buildConfigTomlExportEntries(
            listOf(
                "break.toml",
                "aliases/break.toml",
                "aliases/study.toml",
                "aliases/study.toml",
                "charts/heatmap.toml",
                "meta/bundle.toml",
                "reports/markdown/day.toml",
                "config.toml"
            )
        )

        assertEquals(
            listOf(
                ConfigTomlExportEntry("aliases/break.toml", "aliases/break.toml"),
                ConfigTomlExportEntry("aliases/study.toml", "aliases/study.toml"),
                ConfigTomlExportEntry("charts/heatmap.toml", "charts/heatmap.toml"),
                ConfigTomlExportEntry("config.toml", "config.toml"),
                ConfigTomlExportEntry("meta/bundle.toml", "meta/bundle.toml"),
                ConfigTomlExportEntry(
                    "reports/markdown/day.toml",
                    "reports/markdown/day.toml"
                )
            ),
            result
        )
    }
}
