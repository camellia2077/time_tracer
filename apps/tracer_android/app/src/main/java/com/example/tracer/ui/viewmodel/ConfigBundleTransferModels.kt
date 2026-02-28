package com.example.tracer

data class ConfigExportEntry(
    val relativePath: String,
    val content: String
)

data class ConfigExportBundle(
    val entries: List<ConfigExportEntry>
)

data class ConfigExportBuildResult(
    val ok: Boolean,
    val message: String,
    val bundle: ConfigExportBundle? = null
)

data class ConfigImportEntry(
    val relativePath: String,
    val content: String
)

data class ConfigImportApplyResult(
    val ok: Boolean,
    val message: String
)
