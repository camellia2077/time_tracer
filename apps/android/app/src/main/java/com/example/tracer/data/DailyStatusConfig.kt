package com.example.tracer.data

import org.tomlj.Toml

data class DailyStatusDefinition(
    val id: String,
    val label: String,
    val parent: String
)

data class DailyStatusConfig(
    val schemaVersion: Int = 1,
    val statuses: List<DailyStatusDefinition> = emptyList()
)

internal object DailyStatusConfigStore {
    const val ConfigPath = "user/report.toml"

    fun idForParent(parent: String): String = parent.trim().replace("/", "__")

    fun parse(rawToml: String): DailyStatusConfig {
        val parsed = Toml.parse(rawToml)
        if (parsed.hasErrors()) return DailyStatusConfig()
        val parentPresent = parsed.getTable("daily_statuses")?.getTable("parent_present")
            ?: return DailyStatusConfig(
                schemaVersion = parsed.getLong("schema_version")?.toInt() ?: 1
            )
        val statuses = parentPresent.keySet().sorted().mapNotNull { id ->
            val table = parentPresent.getTable(id) ?: return@mapNotNull null
            val label = table.getString("label")?.trim().orEmpty()
            val parent = table.getString("parent")?.trim().orEmpty()
            if (id.isBlank() || label.isBlank() || parent.isBlank()) null
            else DailyStatusDefinition(id = id, label = label, parent = parent)
        }
        return DailyStatusConfig(
            schemaVersion = parsed.getLong("schema_version")?.toInt() ?: 1,
            statuses = statuses
        )
    }

    fun serialize(config: DailyStatusConfig): String = buildString {
        appendLine("schema_version = ${config.schemaVersion}")
        if (config.statuses.isNotEmpty()) appendLine()
        config.statuses.forEachIndexed { index, status ->
            appendLine(
                "[daily_statuses.parent_present.${formatTableKey(idForParent(status.parent))}]"
            )
            appendLine("parent = \"${escape(status.parent)}\"")
            appendLine("label = \"${escape(status.label)}\"")
            if (index != config.statuses.lastIndex) appendLine()
        }
    }

    private fun escape(value: String): String =
        value.replace("\\", "\\\\").replace("\"", "\\\"")

    private fun formatTableKey(value: String): String =
        if (value.matches(Regex("[A-Za-z0-9_-]+"))) value else "\"${escape(value)}\""
}
