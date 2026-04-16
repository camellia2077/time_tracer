package com.example.tracer

enum class ReportPiePalettePreset {
    SOFT,
    EDITORIAL,
    VIVID,
    MONO_ACCENT
}

fun defaultReportPiePalettePreset(): ReportPiePalettePreset = ReportPiePalettePreset.SOFT

fun reportPiePaletteHexColors(
    preset: ReportPiePalettePreset
): List<String> = when (preset) {
    ReportPiePalettePreset.SOFT -> listOf(
        "#4F46E5",
        "#0EA5A4",
        "#22C55E",
        "#F59E0B",
        "#EF6C57",
        "#EC4899",
        "#8B5CF6",
        "#64748B"
    )
    ReportPiePalettePreset.EDITORIAL -> listOf(
        "#355070",
        "#3D7A6B",
        "#6D8F3F",
        "#B8893C",
        "#D17A5B",
        "#B56576",
        "#7C5C8A",
        "#5C677D"
    )
    ReportPiePalettePreset.VIVID -> listOf(
        "#2563EB",
        "#06B6D4",
        "#10B981",
        "#84CC16",
        "#F59E0B",
        "#F97316",
        "#EF4444",
        "#D946EF"
    )
    ReportPiePalettePreset.MONO_ACCENT -> listOf(
        "#1E3A8A",
        "#1D4ED8",
        "#2563EB",
        "#3B82F6",
        "#0F766E",
        "#0EA5A4",
        "#475569",
        "#64748B"
    )
}

fun reportPiePaletteOthersHexColor(): String = "#94A3B8"
