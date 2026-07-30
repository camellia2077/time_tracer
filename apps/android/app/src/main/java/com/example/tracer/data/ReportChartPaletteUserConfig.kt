package com.example.tracer.data

import com.example.tracer.ConfigGateway
import com.example.tracer.ReportPiePalettePreset
import org.tomlj.Toml

data class ReportChartPaletteUserConfig(
    val heatmapPaletteName: String? = null,
    val piePalettePreset: ReportPiePalettePreset? = null,
)

internal object ReportChartPaletteUserConfigStore {
    private const val ConfigPath = "user/charts.toml"

    suspend fun load(configGateway: ConfigGateway): ReportChartPaletteUserConfig {
        val result = configGateway.readConfigTomlFile(ConfigPath)
        if (!result.ok || result.content.isBlank()) {
            return ReportChartPaletteUserConfig()
        }

        val parsed = Toml.parse(result.content)
        if (parsed.hasErrors()) {
            return ReportChartPaletteUserConfig()
        }

        val heatmap = parsed.getTable("heatmap")?.getString("palette")?.trim()
        val pieName = parsed.getTable("pie")?.getString("palette")?.trim()
        val pie = pieName?.let { name ->
            runCatching { ReportPiePalettePreset.valueOf(name) }.getOrNull()
        }
        return ReportChartPaletteUserConfig(
            heatmapPaletteName = heatmap?.takeIf { it.isNotEmpty() },
            piePalettePreset = pie,
        )
    }

    suspend fun saveHeatmapPalette(
        configGateway: ConfigGateway,
        paletteName: String,
    ): Boolean {
        val current = load(configGateway)
        return save(
            configGateway = configGateway,
            next = current.copy(
                heatmapPaletteName = paletteName.trim().takeIf { it.isNotEmpty() },
            ),
        )
    }

    suspend fun savePiePalette(
        configGateway: ConfigGateway,
        palette: ReportPiePalettePreset,
    ): Boolean {
        val current = load(configGateway)
        return save(
            configGateway = configGateway,
            next = current.copy(piePalettePreset = palette),
        )
    }

    private suspend fun save(
        configGateway: ConfigGateway,
        next: ReportChartPaletteUserConfig,
    ): Boolean {
        val result = configGateway.saveConfigTomlFile(
            ConfigPath,
            buildString {
                appendLine("schema_version = 1")
                appendLine()
                next.heatmapPaletteName?.let {
                    appendLine("[heatmap]")
                    appendLine("palette = \"${escape(it)}\"")
                    appendLine()
                }
                next.piePalettePreset?.let {
                    appendLine("[pie]")
                    appendLine("palette = \"${it.name}\"")
                }
            },
        )
        return result.ok
    }

    private fun escape(value: String): String = value.replace("\\", "\\\\").replace("\"", "\\\"")
}
