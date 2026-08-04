package com.example.tracer

data class ConfigTomlFileEntry(
    val relativePath: String,
    val displayName: String
)

data class ConfigTomlListResult(
    val ok: Boolean,
    val aliasFiles: List<ConfigTomlFileEntry>,
    val chartFiles: List<ConfigTomlFileEntry>,
    val metaFiles: List<ConfigTomlFileEntry>,
    val reportFiles: List<ConfigTomlFileEntry>,
    val message: String,
    /** All mutable TOML files under the canonical config/user root. */
    val userFiles: List<ConfigTomlFileEntry> = emptyList()
)
