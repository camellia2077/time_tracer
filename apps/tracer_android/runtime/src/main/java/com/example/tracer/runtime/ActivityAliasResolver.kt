package com.example.tracer

import java.io.File

internal class ActivityAliasResolver {
    fun loadAliasMapping(paths: RuntimePaths?): Map<String, String> {
        if (paths == null) {
            return emptyMap()
        }
        val mappingFile = resolveMappingConfigFile(paths) ?: return emptyMap()
        return try {
            parseTextMappings(mappingFile)
        } catch (_: Exception) {
            emptyMap()
        }
    }

    fun buildActivityNameSet(aliasMapping: Map<String, String>): Set<String> {
        val names = linkedSetOf<String>()
        for ((alias, fullName) in aliasMapping) {
            val normalizedAlias = alias.trim()
            val normalizedFullName = fullName.trim()
            if (normalizedAlias.isNotEmpty()) {
                names += normalizedAlias
            }
            if (normalizedFullName.isNotEmpty()) {
                names += normalizedFullName
            }
        }
        return names
    }

    fun normalizeSuggestedActivities(
        activities: List<String>,
        aliasMapping: Map<String, String>,
        validActivityNames: Set<String>,
        maxItems: Int
    ): List<String> {
        val unique = linkedSetOf<String>()
        for (activity in activities) {
            val raw = activity.trim()
            if (raw.isEmpty()) {
                continue
            }
            val normalized = (aliasMapping[raw] ?: raw).trim()
            if (normalized.isEmpty()) {
                continue
            }
            if (validActivityNames.isNotEmpty() &&
                !validActivityNames.contains(raw) &&
                !validActivityNames.contains(normalized)
            ) {
                continue
            }
            unique += normalized
            if (unique.size >= maxItems) {
                break
            }
        }
        return unique.toList()
    }

    private fun resolveMappingConfigFile(paths: RuntimePaths): File? {
        val intervalConfigFile = File(paths.configTomlPath)
        if (!intervalConfigFile.exists() || !intervalConfigFile.isFile) {
            return null
        }

        val mappingPath = parseTomlStringValue(
            file = intervalConfigFile,
            key = "mappings_config_path"
        ) ?: "mapping_config.toml"

        val resolved = if (File(mappingPath).isAbsolute) {
            File(mappingPath)
        } else {
            File(intervalConfigFile.parentFile, mappingPath)
        }

        if (!resolved.exists() || !resolved.isFile) {
            return null
        }
        return resolved
    }

    private fun parseTomlStringValue(file: File, key: String): String? {
        val pattern = Regex("""^\s*""" + Regex.escape(key) + """\s*=\s*"([^"]+)"\s*(?:#.*)?$""")
        for (rawLine in file.readLines()) {
            val line = rawLine.trim()
            if (line.isEmpty() || line.startsWith("#")) {
                continue
            }
            val match = pattern.matchEntire(line) ?: continue
            val value = match.groupValues[1].trim()
            if (value.isNotEmpty()) {
                return value
            }
        }
        return null
    }

    private fun parseTextMappings(file: File): Map<String, String> {
        val mapping = linkedMapOf<String, String>()
        val pattern = Regex("""^"([^"]+)"\s*=\s*"([^"]+)"\s*(?:#.*)?$""")
        var inTextMappings = false

        for (rawLine in file.readLines()) {
            val line = rawLine.trim()
            if (line.isEmpty() || line.startsWith("#")) {
                continue
            }

            if (line.startsWith("[") && line.endsWith("]")) {
                inTextMappings = line == "[text_mappings]"
                continue
            }

            if (!inTextMappings) {
                continue
            }

            val match = pattern.matchEntire(line) ?: continue
            val alias = match.groupValues[1].trim()
            val fullName = match.groupValues[2].trim()
            if (alias.isNotEmpty() && fullName.isNotEmpty()) {
                mapping[alias] = fullName
            }
        }

        return mapping
    }
}
