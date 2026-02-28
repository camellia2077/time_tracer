package com.example.tracer

enum class ReportHeatmapThemePolicy {
    FOLLOW_SYSTEM,
    PALETTE
}

data class ReportHeatmapStylePreference(
    val themePolicy: ReportHeatmapThemePolicy = ReportHeatmapThemePolicy.FOLLOW_SYSTEM,
    val paletteName: String = ""
)

data class ReportHeatmapTomlConfig(
    val thresholdsHours: List<Double>,
    val defaultLightPalette: String,
    val defaultDarkPalette: String,
    val palettes: Map<String, List<String>>
) {
    fun paletteNames(): List<String> = palettes.keys.sorted()
}

fun defaultReportHeatmapTomlConfig(): ReportHeatmapTomlConfig =
    ReportHeatmapTomlConfig(
        thresholdsHours = listOf(1.0, 4.0, 7.0, 9.0),
        defaultLightPalette = "GITHUB_GREEN_LIGHT",
        defaultDarkPalette = "GITHUB_GREEN_DARK",
        palettes = linkedMapOf(
            "GITHUB_GREEN_LIGHT" to listOf(
                "#ebedf0",
                "#9be9a8",
                "#40c463",
                "#30a14e",
                "#216e39"
            ),
            "GITHUB_GREEN_DARK" to listOf(
                "#151b23",
                "#033a16",
                "#196c2e",
                "#2ea043",
                "#56d364"
            )
        )
    )
