package com.example.tracer

import java.time.Clock
import java.time.LocalDate
import java.time.YearMonth
import java.time.format.DateTimeFormatter

internal class RecordUseCaseDatePolicy(
    private val clock: Clock
) {
    fun currentMonthKey(): String {
        return YearMonth.now(clock).format(MONTH_FORMATTER)
    }

    fun resolvePreferredMonthForRecord(targetDateIso: String?): String {
        if (targetDateIso.isNullOrBlank()) {
            return currentMonthKey()
        }

        return try {
            YearMonth.from(LocalDate.parse(targetDateIso)).format(MONTH_FORMATTER)
        } catch (_: Exception) {
            currentMonthKey()
        }
    }

    fun resolveTargetDateIso(logicalDayTarget: RecordLogicalDayTarget): String {
        // Record and TXT must share the same logical-day resolver so "yesterday/today" means
        // exactly the same date in both tabs within the current session.
        val targetDate = resolveLogicalDayTargetDate(
            logicalDayTarget = logicalDayTarget,
            clock = clock
        )
        return targetDate.format(DateTimeFormatter.ISO_LOCAL_DATE)
    }

    fun resolveRecordTimeOrderMode(
        logicalDayTarget: RecordLogicalDayTarget
    ): RecordTimeOrderMode {
        // Explicit mode contract:
        // - record to yesterday enables logical_day_0600 so 00:xx belongs to the same
        //   logical day axis as prior late-night entries.
        // - record to today stays strict calendar ordering and does not apply rollover mapping.
        return when (logicalDayTarget) {
            RecordLogicalDayTarget.YESTERDAY -> RecordTimeOrderMode.LOGICAL_DAY_0600
            RecordLogicalDayTarget.TODAY -> RecordTimeOrderMode.STRICT_CALENDAR
        }
    }

    fun defaultLogicalDayTarget(): RecordLogicalDayTarget {
        return com.example.tracer.defaultLogicalDayTarget(clock.millis(), clock.zone)
    }

    private companion object {
        private val MONTH_FORMATTER: DateTimeFormatter = DateTimeFormatter.ofPattern("yyyy-MM")
    }
}

