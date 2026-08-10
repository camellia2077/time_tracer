package com.example.tracer

internal class StructuredInsightsModelTranslator {
    fun translate(payload: StructuredInsightsWirePayload): StructuredDailyInsights =
        StructuredDailyInsights(
            date = payload.date,
            totalDurationSeconds = payload.totalDurationSeconds.coerceAtLeast(0L),
            dayRemark = payload.dayRemark,
            statuses = payload.statuses,
            activities = payload.records.map { record ->
                ActivityTimelineItem(
                    logicalId = record.logicalId,
                    startTime = record.startTime,
                    endTime = record.endTime,
                    activityName = record.activityName,
                    durationSeconds = record.durationSeconds.coerceAtLeast(0L),
                    remark = record.remark,
                    kind = ActivityTimelineRecordKind.fromWireValue(record.recordKind)
                )
            }
        )
}
