package com.example.tracer.data

import android.content.res.AssetManager
import com.example.tracer.ReportPiePaletteConfig
import com.example.tracer.ReportPiePalettePreset
import com.example.tracer.defaultReportPiePaletteConfig
import com.example.tracer.installReportPiePaletteConfig
import org.tomlj.Toml
import org.tomlj.TomlArray

internal object ReportPiePaletteTomlLoader {
    private const val AssetPath = "config/program/charts/pie.toml"
    private val HexColorPattern = Regex("^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$")

    fun installFromAssets(assetManager: AssetManager) {
        installReportPiePaletteConfig(load(assetManager))
    }

    fun load(assetManager: AssetManager): ReportPiePaletteConfig {
        val rawToml = runCatching {
            assetManager.open(AssetPath).bufferedReader().use { it.readText() }
        }.getOrNull() ?: return defaultConfig()
        return parse(rawToml)
    }

    fun parse(rawToml: String): ReportPiePaletteConfig {
        val fallback = defaultConfig()
        val parsed = Toml.parse(rawToml)
        if (parsed.hasErrors()) {
            return fallback
        }

        val palettesTable = parsed.getTable("palettes")
        val palettes = ReportPiePalettePreset.entries.associateWith { preset ->
            val parsedColors = parseColors(palettesTable?.getArray(preset.name))
            if (parsedColors.size == fallback.palettes.getValue(preset).size) {
                parsedColors
            } else {
                fallback.palettes.getValue(preset)
            }
        }
        val others = parsed.getString("others")?.trim()
            ?.takeIf { HexColorPattern.matches(it) }
            ?: fallback.othersHexColor

        return ReportPiePaletteConfig(palettes = palettes, othersHexColor = others)
    }

    private fun parseColors(array: TomlArray?): List<String> {
        if (array == null) {
            return emptyList()
        }
        return buildList {
            for (index in 0 until array.size()) {
                val value = (array.get(index) as? String)?.trim() ?: continue
                if (HexColorPattern.matches(value)) {
                    add(value)
                }
            }
        }
    }

    private fun defaultConfig(): ReportPiePaletteConfig {
        return defaultReportPiePaletteConfig()
    }
}
