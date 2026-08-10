package com.example.tracer.data

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
    fun idForParent(parent: String): String = parent.trim().replace("/", "__")
}
