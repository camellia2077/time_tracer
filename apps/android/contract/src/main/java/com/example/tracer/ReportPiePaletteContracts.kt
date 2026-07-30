package com.example.tracer

enum class ReportPiePalettePreset {
    SOFT,
    EDITORIAL,
    VIVID,
    MONO_ACCENT
}

data class ReportPiePaletteConfig(
    val palettes: Map<ReportPiePalettePreset, List<String>>,
    val othersHexColor: String
)

fun defaultReportPiePalettePreset(): ReportPiePalettePreset = ReportPiePalettePreset.SOFT

fun defaultReportPiePaletteConfig(): ReportPiePaletteConfig = ReportPiePaletteConfig(
    palettes = mapOf(
        // Each list is intentionally ordered from the strongest visual weight to the
        // lightest. Pie charts assign these positions by slice rank (largest first).
        ReportPiePalettePreset.SOFT to listOf(
            "#4338CA",
            "#0F766E",
            "#BE123C",
            "#15803D",
            "#C2410C",
            "#7E22CE",
            "#0369A1",
            "#4F46E5",
            "#14B8A6",
            "#C084FC"
        ),
        ReportPiePalettePreset.EDITORIAL to listOf(
            "#355070",
            "#3D7A6B",
            "#A44A3F",
            "#6D8F3F",
            "#7C5C8A",
            "#B56576",
            "#B8893C",
            "#4A6FA5",
            "#D17A5B",
            "#5C677D"
        ),
        ReportPiePalettePreset.VIVID to listOf(
            "#2563EB",
            "#10B981",
            "#EF4444",
            "#7C3AED",
            "#F97316",
            "#06B6D4",
            "#D946EF",
            "#84CC16",
            "#F59E0B",
            "#14B8A6"
        ),
        ReportPiePalettePreset.MONO_ACCENT to listOf(
            "#1E3A8A",
            "#0F766E",
            "#1D4ED8",
            "#475569",
            "#0EA5A4",
            "#2563EB",
            "#64748B",
            "#0C4A6E",
            "#3B82F6",
            "#334155"
        )
    ),
    othersHexColor = "#94A3B8"
)

@Volatile
private var activeReportPiePaletteConfig = defaultReportPiePaletteConfig()

fun installReportPiePaletteConfig(config: ReportPiePaletteConfig) {
    activeReportPiePaletteConfig = config
}

fun reportPiePaletteHexColors(
    preset: ReportPiePalettePreset
): List<String> = activeReportPiePaletteConfig.palettes[preset]
    ?: defaultReportPiePaletteConfig().palettes.getValue(preset)

fun reportPiePaletteOthersHexColor(): String = activeReportPiePaletteConfig.othersHexColor
