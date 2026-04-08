package com.example.tracer

import java.io.File

internal class ConfigTomlStorage(private val configRootPath: String) {
    fun listTomlFiles(): ConfigTomlListResult {
        val root = File(configRootPath)
        if (!root.exists()) {
            return ConfigTomlListResult(
                ok = true,
                converterFiles = emptyList(),
                chartFiles = emptyList(),
                metaFiles = emptyList(),
                reportFiles = emptyList(),
                message = "No config directory."
            )
        }

        val allTomlFiles = root.walkTopDown()
            .filter { it.isFile && it.extension.equals("toml", ignoreCase = true) }
            .map { it.relativeTo(root).invariantSeparatorsPath }
            .sorted()
            .toList()

        val converterFiles = mutableListOf<ConfigTomlFileEntry>()
        val chartFiles = mutableListOf<ConfigTomlFileEntry>()
        val metaFiles = mutableListOf<ConfigTomlFileEntry>()
        val reportFiles = mutableListOf<ConfigTomlFileEntry>()
        for (path in allTomlFiles) {
            if (path.startsWith("reports/")) {
                reportFiles += path.toConfigTomlFileEntry(prefixToTrim = "reports/")
            } else if (path.startsWith("charts/")) {
                chartFiles += path.toConfigTomlFileEntry(prefixToTrim = "charts/")
            } else if (path == "config.toml" || path.startsWith("meta/")) {
                metaFiles += path.toConfigTomlFileEntry(prefixToTrim = null)
            } else {
                converterFiles += path.toConfigTomlFileEntry(prefixToTrim = "converter/")
            }
        }

        return ConfigTomlListResult(
            ok = true,
            converterFiles = converterFiles,
            chartFiles = chartFiles,
            metaFiles = metaFiles,
            reportFiles = reportFiles,
            message = "Found ${allTomlFiles.size} TOML file(s)."
        )
    }

    fun readTomlFile(relativePath: String): TxtFileContentResult {
        val requested = normalizeRequestedPath(relativePath)
            ?: return TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = "TOML file path is invalid."
            )

        val root = File(configRootPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TOML path is outside config root."
            )
        }
        if (!target.exists() || !target.isFile) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TOML file not found."
            )
        }

        val canonicalContent = runCatching {
            CanonicalTextCodec.readFile(target)
        }.getOrElse { error ->
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "Cannot read TOML file: ${error.message ?: "unknown read error"}"
            )
        }

        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = canonicalContent,
            message = "Read TOML success."
        )
    }

    fun writeTomlFile(relativePath: String, content: String): TxtFileContentResult {
        val requested = normalizeRequestedPath(relativePath)
            ?: return TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = "TOML file path is invalid."
            )

        val root = File(configRootPath).canonicalFile
        val target = File(root, requested).canonicalFile
        val relative = target.relativeToOrNull(root)
        if (relative == null) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TOML path is outside config root."
            )
        }
        if (!target.exists() || !target.isFile) {
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "TOML file not found."
            )
        }

        val canonicalContent = runCatching {
            CanonicalTextCodec.canonicalizeText(content)
        }.getOrElse { error ->
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "Cannot normalize TOML content: ${error.message ?: "unknown text error"}"
            )
        }
        runCatching {
            CanonicalTextCodec.writeFile(target, canonicalContent)
        }.getOrElse { error ->
            return TxtFileContentResult(
                ok = false,
                filePath = requested,
                content = "",
                message = "Cannot write TOML file: ${error.message ?: "unknown write error"}"
            )
        }
        return TxtFileContentResult(
            ok = true,
            filePath = relative.invariantSeparatorsPath,
            content = canonicalContent,
            message = "Save TOML success."
        )
    }

    private fun normalizeRequestedPath(relativePath: String): String? {
        val trimmed = relativePath.trim()
        if (trimmed.isEmpty()) {
            return null
        }
        if (!trimmed.endsWith(".toml", ignoreCase = true)) {
            return null
        }
        return trimmed
    }

    private fun String.toConfigTomlFileEntry(prefixToTrim: String?): ConfigTomlFileEntry {
        // Read/write must keep the canonical relative path because config references
        // inside tracer_core use these paths as stable identifiers.
        // Only the display label drops the selected top-level category prefix.
        val displayName = if (prefixToTrim != null && startsWith(prefixToTrim)) {
            removePrefix(prefixToTrim)
        } else {
            this
        }
        return ConfigTomlFileEntry(
            relativePath = this,
            displayName = displayName
        )
    }
}
