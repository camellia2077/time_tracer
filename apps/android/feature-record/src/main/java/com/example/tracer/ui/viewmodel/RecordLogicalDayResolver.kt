package com.example.tracer

import java.time.Clock
import java.time.Instant
import java.time.LocalDate
import java.time.LocalTime
import java.time.ZoneId

// Shared logical-day resolver for both Record and TXT flows.
// Design intent:
// - keep one source of truth for yesterday/today date resolution;
// - keep this as session state only (no local persistence), so app restarts recompute from clock.
internal val RECORD_LOGICAL_DAY_CUTOFF: LocalTime = LocalTime.of(6, 0)

internal fun defaultLogicalDayTarget(
    currentTimeMillis: Long,
    zoneId: ZoneId
): RecordLogicalDayTarget {
    val localTime = Instant.ofEpochMilli(currentTimeMillis).atZone(zoneId).toLocalTime()
    return if (localTime.isBefore(RECORD_LOGICAL_DAY_CUTOFF)) {
        RecordLogicalDayTarget.YESTERDAY
    } else {
        RecordLogicalDayTarget.TODAY
    }
}

internal fun defaultLogicalDayTarget(clock: Clock): RecordLogicalDayTarget =
    defaultLogicalDayTarget(
        currentTimeMillis = clock.millis(),
        zoneId = clock.zone
    )

internal fun resolveLogicalDayTargetDate(
    logicalDayTarget: RecordLogicalDayTarget,
    clock: Clock
): LocalDate {
    val today = LocalDate.now(clock)
    return when (logicalDayTarget) {
        RecordLogicalDayTarget.YESTERDAY -> today.minusDays(1)
        RecordLogicalDayTarget.TODAY -> today
    }
}
