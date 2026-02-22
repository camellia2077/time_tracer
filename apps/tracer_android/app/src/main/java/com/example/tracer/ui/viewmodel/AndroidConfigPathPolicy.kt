package com.example.tracer

internal object AndroidConfigPathPolicy {
    private val requiredImportPathsInternal = listOf(
        "config.toml",
        "meta/bundle.toml",
        "converter/interval_processor_config.toml",
        "converter/alias_mapping.toml",
        "converter/duration_rules.toml",
        "reports/markdown/day.toml",
        "reports/markdown/month.toml",
        "reports/markdown/period.toml",
        "reports/markdown/week.toml",
        "reports/markdown/year.toml"
    )
    private val requiredImportPathSetInternal = requiredImportPathsInternal.toSet()
    private val canonicalPathByFileNameLower = requiredImportPathsInternal.associateBy {
        it.substringAfterLast('/').lowercase()
    }

    val requiredImportPaths: List<String>
        get() = requiredImportPathsInternal

    fun normalizeRelativePath(path: String): String? {
        val normalized = path
            .trim()
            .replace('\\', '/')
            .trim('/')
        if (normalized.isEmpty()) {
            return null
        }
        if (!normalized.endsWith(".toml", ignoreCase = true)) {
            return null
        }
        val parts = normalized.split('/')
        if (parts.any { it.isBlank() || it == "." || it == ".." }) {
            return null
        }
        return normalized
    }

    fun isAllowedImportPath(path: String): Boolean = path in requiredImportPathSetInternal

    fun resolveCanonicalImportPathForPartial(path: String): String? {
        val normalized = normalizeRelativePath(path) ?: return null
        if (isAllowedImportPath(normalized)) {
            return normalized
        }
        val fileNameLower = normalized.substringAfterLast('/').lowercase()
        return canonicalPathByFileNameLower[fileNameLower]
    }
}
