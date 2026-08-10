package com.example.tracer

enum class InsightsHeatmapThemePolicy {
    FOLLOW_SYSTEM,
    PALETTE
}

data class InsightsHeatmapStylePreference(
    val themePolicy: InsightsHeatmapThemePolicy = InsightsHeatmapThemePolicy.FOLLOW_SYSTEM,
    val paletteName: String = ""
)

data class InsightsHeatmapTomlConfig(
    val thresholdsHours: List<Double>,
    val defaultLightPalette: String,
    val defaultDarkPalette: String,
    val palettes: Map<String, List<String>>
) {
    fun paletteNames(): List<String> = palettes.keys.sorted()
}

fun defaultInsightsHeatmapTomlConfig(): InsightsHeatmapTomlConfig =
    InsightsHeatmapTomlConfig(
        thresholdsHours = listOf(1.0, 4.0, 7.0, 9.0),
        defaultLightPalette = "GREEN_LIGHT",
        defaultDarkPalette = "GREEN_DARK",
        palettes = linkedMapOf(
            "GREEN_LIGHT" to listOf(
                "#eff2f5",
                "#aceebb",
                "#4ac26b",
                "#2da44e",
                "#116329"
            ),
            "GREEN_DARK" to listOf(
                "#151b23",
                "#033a16",
                "#196c2e",
                "#2ea043",
                "#56d364"
            )
        )
    )
