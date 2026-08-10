package com.example.tracer.data

import com.example.tracer.InsightsMode

data class DailyStatusDefinition(
    val id: String,
    val label: String,
    val parent: String
)

data class DailyStatusConfig(
    val schemaVersion: Int = 1,
    val statuses: List<DailyStatusDefinition> = emptyList()
)

data class InsightsStatusConfigs(
    val day: DailyStatusConfig = DailyStatusConfig(),
    val week: DailyStatusConfig = DailyStatusConfig(),
    val month: DailyStatusConfig = DailyStatusConfig(),
    val year: DailyStatusConfig = DailyStatusConfig(),
    val recent: DailyStatusConfig = DailyStatusConfig(),
    val range: DailyStatusConfig = DailyStatusConfig()
) {
    operator fun get(mode: InsightsMode): DailyStatusConfig = when (mode) {
        InsightsMode.DAY -> day
        InsightsMode.WEEK -> week
        InsightsMode.MONTH -> month
        InsightsMode.YEAR -> year
        InsightsMode.RECENT -> recent
        InsightsMode.RANGE -> range
    }
}

internal object DailyStatusConfigStore {
    fun idForParent(parent: String): String = parent.trim().replace("/", "__")
}
