package com.example.tracer

import java.io.File

internal data class RuntimeConfigBundleStatus(
    val ok: Boolean,
    val schemaVersion: Int?,
    val profile: String,
    val bundleName: String,
    val requiredFiles: List<String>,
    val missingFiles: List<String>,
    val bundlePath: String,
    val message: String,
    val validatedAtEpochMs: Long
)

internal fun unknownConfigBundleStatus(nowMs: Long = System.currentTimeMillis()): RuntimeConfigBundleStatus {
    return RuntimeConfigBundleStatus(
        ok = false,
        schemaVersion = null,
        profile = "",
        bundleName = "",
        requiredFiles = emptyList(),
        missingFiles = emptyList(),
        bundlePath = "",
        message = "Config bundle has not been validated yet.",
        validatedAtEpochMs = nowMs
    )
}

internal fun validateRuntimeConfigBundle(
    configRoot: File,
    expectedProfile: String = "android",
    nowMs: Long = System.currentTimeMillis()
): RuntimeConfigBundleStatus {
    val bundleFile = File(configRoot, "meta/bundle.toml")
    if (!bundleFile.exists() || !bundleFile.isFile) {
        return RuntimeConfigBundleStatus(
            ok = false,
            schemaVersion = null,
            profile = "",
            bundleName = "",
            requiredFiles = emptyList(),
            missingFiles = listOf("meta/bundle.toml"),
            bundlePath = bundleFile.absolutePath,
            message = "Missing config bundle file: ${bundleFile.absolutePath}",
            validatedAtEpochMs = nowMs
        )
    }

    val rawToml = runCatching {
        bundleFile.readText()
    }.getOrElse { error ->
        return RuntimeConfigBundleStatus(
            ok = false,
            schemaVersion = null,
            profile = "",
            bundleName = "",
            requiredFiles = emptyList(),
            missingFiles = emptyList(),
            bundlePath = bundleFile.absolutePath,
            message = "Cannot read config bundle: ${error.message ?: "unknown read error"}",
            validatedAtEpochMs = nowMs
        )
    }

    val parsed = parseBundleToml(rawToml)
    val schemaVersion = parsed.schemaVersion
    if (schemaVersion == null || schemaVersion <= 0) {
        return RuntimeConfigBundleStatus(
            ok = false,
            schemaVersion = schemaVersion,
            profile = parsed.profile,
            bundleName = parsed.bundleName,
            requiredFiles = parsed.requiredFiles,
            missingFiles = emptyList(),
            bundlePath = bundleFile.absolutePath,
            message = "Invalid bundle schema_version in ${bundleFile.absolutePath}",
            validatedAtEpochMs = nowMs
        )
    }

    if (parsed.profile != expectedProfile) {
        return RuntimeConfigBundleStatus(
            ok = false,
            schemaVersion = schemaVersion,
            profile = parsed.profile,
            bundleName = parsed.bundleName,
            requiredFiles = parsed.requiredFiles,
            missingFiles = emptyList(),
            bundlePath = bundleFile.absolutePath,
            message = "Config bundle profile mismatch: expected=$expectedProfile actual=${parsed.profile}",
            validatedAtEpochMs = nowMs
        )
    }

    if (parsed.requiredFiles.isEmpty()) {
        return RuntimeConfigBundleStatus(
            ok = false,
            schemaVersion = schemaVersion,
            profile = parsed.profile,
            bundleName = parsed.bundleName,
            requiredFiles = emptyList(),
            missingFiles = emptyList(),
            bundlePath = bundleFile.absolutePath,
            message = "Config bundle required file list is empty.",
            validatedAtEpochMs = nowMs
        )
    }

    val missingFiles = parsed.requiredFiles.filter { relativePath ->
        !File(configRoot, relativePath).exists()
    }
    if (missingFiles.isNotEmpty()) {
        return RuntimeConfigBundleStatus(
            ok = false,
            schemaVersion = schemaVersion,
            profile = parsed.profile,
            bundleName = parsed.bundleName,
            requiredFiles = parsed.requiredFiles,
            missingFiles = missingFiles,
            bundlePath = bundleFile.absolutePath,
            message = "Config bundle missing required file(s): ${missingFiles.joinToString(", ")}",
            validatedAtEpochMs = nowMs
        )
    }

    return RuntimeConfigBundleStatus(
        ok = true,
        schemaVersion = schemaVersion,
        profile = parsed.profile,
        bundleName = parsed.bundleName,
        requiredFiles = parsed.requiredFiles,
        missingFiles = emptyList(),
        bundlePath = bundleFile.absolutePath,
        message = "Config bundle validated: schema=$schemaVersion profile=${parsed.profile}",
        validatedAtEpochMs = nowMs
    )
}

private data class ParsedBundleToml(
    val schemaVersion: Int?,
    val profile: String,
    val bundleName: String,
    val requiredFiles: List<String>
)

private fun parseBundleToml(content: String): ParsedBundleToml {
    val schemaVersion = SCHEMA_VERSION_PATTERN
        .find(content)
        ?.groupValues
        ?.getOrNull(1)
        ?.toIntOrNull()
    val profile = PROFILE_PATTERN
        .find(content)
        ?.groupValues
        ?.getOrNull(1)
        ?.trim()
        .orEmpty()
    val bundleName = BUNDLE_NAME_PATTERN
        .find(content)
        ?.groupValues
        ?.getOrNull(1)
        ?.trim()
        .orEmpty()
    val requiredBlock = REQUIRED_LIST_BLOCK_PATTERN
        .find(content)
        ?.groupValues
        ?.getOrNull(1)
        .orEmpty()
    val requiredFiles = QUOTED_STRING_PATTERN
        .findAll(requiredBlock)
        .map { it.groupValues[1].trim() }
        .filter { it.isNotEmpty() }
        .toList()

    return ParsedBundleToml(
        schemaVersion = schemaVersion,
        profile = profile,
        bundleName = bundleName,
        requiredFiles = requiredFiles
    )
}

private val SCHEMA_VERSION_PATTERN = Regex("""(?m)^\s*schema_version\s*=\s*(\d+)\s*$""")
private val PROFILE_PATTERN = Regex("""(?m)^\s*profile\s*=\s*"([^"]+)"\s*$""")
private val BUNDLE_NAME_PATTERN = Regex("""(?m)^\s*bundle_name\s*=\s*"([^"]+)"\s*$""")
private val REQUIRED_LIST_BLOCK_PATTERN = Regex("""(?s)\[file_list\].*?required\s*=\s*\[(.*?)]""")
private val QUOTED_STRING_PATTERN = Regex("\"([^\"]+)\"")
