package com.example.tracer

internal class StructuredInsightsModelTranslator {
    fun translate(payload: StructuredInsightsWirePayload): StructuredDailyInsights =
        StructuredDailyInsights(
            date = payload.date,
            totalDurationSeconds = payload.totalDurationSeconds.coerceAtLeast(0L),
            dayRemark = payload.dayRemark,
            statuses = payload.statuses,
            activities = payload.records.map(::translateActivity)
        )

    fun translateActivityDay(payload: StructuredInsightsWireActivityDay): StructuredDailyInsights =
        StructuredDailyInsights(
            date = payload.date,
            totalDurationSeconds = payload.totalDurationSeconds.coerceAtLeast(0L),
            activities = payload.records.map(::translateActivity)
        )

    fun translateProjectNode(
        payload: StructuredInsightsWireProjectNode
    ): StructuredInsightsProjectNode = StructuredInsightsProjectNode(
        name = payload.name,
        durationSeconds = payload.durationSeconds.coerceAtLeast(0L),
        children = payload.children.map(::translateProjectNode)
    )

    private fun translateActivity(record: StructuredInsightsWireRecord): ActivityTimelineItem =
        ActivityTimelineItem(
            logicalId = record.logicalId,
            startTime = record.startTime,
            endTime = record.endTime,
            activityName = record.activityName,
            durationSeconds = record.durationSeconds.coerceAtLeast(0L),
            remark = record.remark,
            kind = ActivityTimelineRecordKind.fromWireValue(record.recordKind),
            parentColor = record.parentColor
        )
}
