package com.example.tracer

internal class FakeQuickActivitiesPreferenceGateway(
    initialQuickActivities: List<String> = emptyList()
) : QuickActivitiesPreferenceGateway {
    var quickActivities: List<String> = initialQuickActivities

    override suspend fun getQuickActivities(): List<String> = quickActivities

    override suspend fun setQuickActivities(values: List<String>) {
        quickActivities = values
    }
}
