package com.example.tracer.data

import android.content.res.AssetManager
import com.example.tracer.InsightsPiePaletteConfig
import com.example.tracer.InsightsPiePalettePreset
import com.example.tracer.defaultInsightsPiePaletteConfig
import com.example.tracer.installInsightsPiePaletteConfig
import org.tomlj.Toml
import org.tomlj.TomlArray

internal object InsightsPiePaletteTomlLoader {
    private const val AssetPath = "config/program/charts/pie.toml"
    private val HexColorPattern = Regex("^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$")

    fun installFromAssets(assetManager: AssetManager) {
        installInsightsPiePaletteConfig(load(assetManager))
    }

    fun load(assetManager: AssetManager): InsightsPiePaletteConfig {
        val rawToml = runCatching {
            assetManager.open(AssetPath).bufferedReader().use { it.readText() }
        }.getOrNull() ?: return defaultConfig()
        return parse(rawToml)
    }

    fun parse(rawToml: String): InsightsPiePaletteConfig {
        val fallback = defaultConfig()
        val parsed = Toml.parse(rawToml)
        if (parsed.hasErrors()) {
            return fallback
        }

        val palettesTable = parsed.getTable("palettes")
        val palettes = InsightsPiePalettePreset.entries.associateWith { preset ->
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

        return InsightsPiePaletteConfig(palettes = palettes, othersHexColor = others)
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

    private fun defaultConfig(): InsightsPiePaletteConfig {
        return defaultInsightsPiePaletteConfig()
    }
}
