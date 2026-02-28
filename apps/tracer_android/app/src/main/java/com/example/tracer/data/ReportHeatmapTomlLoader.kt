package com.example.tracer

import org.tomlj.Toml
import org.tomlj.TomlArray
import org.tomlj.TomlTable

internal object ReportHeatmapTomlLoader {
    private const val HeatmapConfigPath = "charts/heatmap.toml"
    private val HexColorPattern = Regex("^#[0-9a-fA-F]{6}([0-9a-fA-F]{2})?$")
    private val SectionHeaderPattern = Regex("""^\s*\[[^\]]+\]\s*$""")

    suspend fun load(controller: RuntimeGateway): ReportHeatmapTomlConfig {
        val readResult = controller.readConfigTomlFile(HeatmapConfigPath)
        if (!readResult.ok || readResult.content.isBlank()) {
            return defaultReportHeatmapTomlConfig()
        }
        return parse(readResult.content)
    }

    fun configPath(): String = HeatmapConfigPath

    fun parse(rawToml: String): ReportHeatmapTomlConfig {
        val fallback = defaultReportHeatmapTomlConfig()
        val parsed = Toml.parse(rawToml)
        if (parsed.hasErrors()) {
            return fallback
        }

        val thresholds = parseThresholds(parsed.getTable("thresholds"))
            ?: fallback.thresholdsHours
        val parsedPalettes = parsePalettes(parsed.getTable("palettes"))
        val palettes = if (parsedPalettes.isEmpty()) fallback.palettes else parsedPalettes

        val defaultsTable = parsed.getTable("defaults")
        val defaultLightPalette = resolveDefaultPaletteName(
            preferredName = defaultsTable?.getString("light_palette"),
            fallbackName = fallback.defaultLightPalette,
            paletteNames = palettes.keys
        )
        val defaultDarkPalette = resolveDefaultPaletteName(
            preferredName = defaultsTable?.getString("dark_palette"),
            fallbackName = fallback.defaultDarkPalette,
            paletteNames = palettes.keys
        )

        return ReportHeatmapTomlConfig(
            thresholdsHours = thresholds,
            defaultLightPalette = defaultLightPalette,
            defaultDarkPalette = defaultDarkPalette,
            palettes = palettes
        )
    }

    private fun parseThresholds(table: TomlTable?): List<Double>? {
        val array = table?.getArray("positive_hours") ?: return null
        val values = mutableListOf<Double>()
        for (index in 0 until array.size()) {
            val raw = array.get(index)
            val value = (raw as? Number)?.toDouble() ?: return null
            if (!value.isFinite() || value <= 0.0) {
                return null
            }
            values += value
        }
        if (values.isEmpty()) {
            return null
        }
        return values.sorted().distinct()
    }

    private fun parsePalettes(table: TomlTable?): Map<String, List<String>> {
        if (table == null) {
            return emptyMap()
        }

        val paletteNames = table.keySet().sorted()
        val palettes = linkedMapOf<String, List<String>>()
        for (name in paletteNames) {
            val colors = parsePaletteColors(table.getArray(name))
            if (colors.isNotEmpty()) {
                palettes[name] = colors
            }
        }
        return palettes
    }

    private fun parsePaletteColors(array: TomlArray?): List<String> {
        if (array == null || array.size() == 0) {
            return emptyList()
        }
        val colors = mutableListOf<String>()
        for (index in 0 until array.size()) {
            val raw = array.get(index) as? String ?: continue
            val normalized = raw.trim()
            if (!HexColorPattern.matches(normalized)) {
                continue
            }
            colors += normalized
        }
        return colors
    }

    private fun resolveDefaultPaletteName(
        preferredName: String?,
        fallbackName: String,
        paletteNames: Set<String>
    ): String {
        if (preferredName != null && preferredName in paletteNames) {
            return preferredName
        }
        if (fallbackName in paletteNames) {
            return fallbackName
        }
        return paletteNames.first()
    }

    fun deriveStylePreference(config: ReportHeatmapTomlConfig): ReportHeatmapStylePreference {
        val availableNames = config.palettes.keys
        if (
            config.defaultLightPalette.isNotBlank() &&
            config.defaultLightPalette == config.defaultDarkPalette &&
            config.defaultLightPalette in availableNames
        ) {
            return ReportHeatmapStylePreference(
                themePolicy = ReportHeatmapThemePolicy.PALETTE,
                paletteName = config.defaultLightPalette
            )
        }
        return ReportHeatmapStylePreference(
            themePolicy = ReportHeatmapThemePolicy.FOLLOW_SYSTEM,
            paletteName = ""
        )
    }

    fun resolveDefaultPalettesForStyle(
        config: ReportHeatmapTomlConfig,
        stylePreference: ReportHeatmapStylePreference
    ): Pair<String, String> {
        val availableNames = config.palettes.keys
        val fallback = defaultReportHeatmapTomlConfig()
        val fallbackLight =
            if (fallback.defaultLightPalette in availableNames) {
                fallback.defaultLightPalette
            } else {
                availableNames.firstOrNull().orEmpty()
            }
        val fallbackDark =
            if (fallback.defaultDarkPalette in availableNames) {
                fallback.defaultDarkPalette
            } else {
                fallbackLight
            }

        return when (stylePreference.themePolicy) {
            ReportHeatmapThemePolicy.PALETTE -> {
                val selected = stylePreference.paletteName.trim()
                val resolved =
                    if (selected in availableNames) {
                        selected
                    } else if (config.defaultLightPalette in availableNames) {
                        config.defaultLightPalette
                    } else {
                        fallbackLight
                    }
                resolved to resolved
            }

            ReportHeatmapThemePolicy.FOLLOW_SYSTEM -> {
                val currentLight = config.defaultLightPalette
                val currentDark = config.defaultDarkPalette
                val canUseCurrent =
                    currentLight in availableNames &&
                        currentDark in availableNames &&
                        currentLight != currentDark
                if (canUseCurrent) {
                    currentLight to currentDark
                } else {
                    fallbackLight to fallbackDark
                }
            }
        }
    }

    fun rewriteDefaults(
        rawToml: String,
        lightPalette: String,
        darkPalette: String
    ): String? {
        val lines = rawToml.split("\n").toMutableList()
        if (lines.isEmpty()) {
            return null
        }

        val defaultsStart = lines.indexOfFirst { line ->
            line.trim().equals("[defaults]", ignoreCase = false)
        }
        if (defaultsStart < 0) {
            return null
        }

        var defaultsEnd = lines.size
        for (index in (defaultsStart + 1) until lines.size) {
            val trimmed = lines[index].trim()
            if (trimmed.isNotEmpty() && SectionHeaderPattern.matches(trimmed)) {
                defaultsEnd = index
                break
            }
        }

        var lightLineIndex = -1
        var darkLineIndex = -1
        var indent = ""
        for (index in (defaultsStart + 1) until defaultsEnd) {
            val line = lines[index]
            val trimmed = line.trim()
            if (trimmed.startsWith("light_palette")) {
                lightLineIndex = index
                indent = line.takeWhile { it == ' ' || it == '\t' }
            } else if (trimmed.startsWith("dark_palette")) {
                darkLineIndex = index
                if (indent.isEmpty()) {
                    indent = line.takeWhile { it == ' ' || it == '\t' }
                }
            }
        }

        val lightLine = "${indent}light_palette = \"$lightPalette\""
        val darkLine = "${indent}dark_palette = \"$darkPalette\""

        if (lightLineIndex >= 0) {
            lines[lightLineIndex] = lightLine
        } else {
            lines.add(defaultsEnd, lightLine)
            defaultsEnd += 1
            if (darkLineIndex >= defaultsEnd) {
                darkLineIndex += 1
            }
        }

        if (darkLineIndex >= 0) {
            lines[darkLineIndex] = darkLine
        } else {
            lines.add(defaultsEnd, darkLine)
        }

        val hasTrailingNewline = rawToml.endsWith("\n")
        val rewritten = lines.joinToString("\n")
        return if (hasTrailingNewline) {
            "$rewritten\n"
        } else {
            rewritten
        }
    }
}
