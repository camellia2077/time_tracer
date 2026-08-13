package com.example.tracer

internal fun StructuredInsightsCallResult.activityDaysForComparison(): List<StructuredDailyInsights> =
    listOfNotNull(insights).ifEmpty { activityDays }
