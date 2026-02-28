package com.example.tracer

import java.time.LocalDate

internal fun mapCorePayloadToDomainModel(payload: ReportChartData): DomainChartModel {
    val normalizedRoots = payload.roots.distinct()
    val normalizedPoints = payload.points
        .map { point ->
            DomainChartPoint(
                date = point.date,
                durationSeconds = point.durationSeconds.coerceAtLeast(0L),
                epochDay = point.epochDay
            )
        }
        .sortedWith(
            compareBy<DomainChartPoint>(
                { it.epochDay ?: parseEpochDayOrNull(it.date) ?: Long.MAX_VALUE },
                { it.date }
            )
        )

    val fallbackTotal = normalizedPoints.sumOf { it.durationSeconds }
    val fallbackActiveDays = normalizedPoints.count { it.durationSeconds > 0L }
    val fallbackRangeDays = if (normalizedPoints.isNotEmpty()) {
        normalizedPoints.size
    } else {
        payload.lookbackDays.coerceAtLeast(0)
    }
    val resolvedTotal = payload.totalDurationSeconds?.coerceAtLeast(0L) ?: fallbackTotal
    val resolvedActiveDays = payload.activeDays?.coerceAtLeast(0) ?: fallbackActiveDays
    val resolvedRangeDays = payload.rangeDays?.coerceAtLeast(0) ?: fallbackRangeDays
    val resolvedAverage = payload.averageDurationSeconds?.coerceAtLeast(0L)
        ?: if (resolvedRangeDays > 0) {
            resolvedTotal / resolvedRangeDays
        } else {
            0L
        }
    val hasFallback = payload.usesLegacyStatsFallback ||
        payload.averageDurationSeconds == null ||
        payload.totalDurationSeconds == null ||
        payload.activeDays == null ||
        payload.rangeDays == null

    return DomainChartModel(
        roots = normalizedRoots,
        selectedRoot = payload.selectedRoot,
        lookbackDays = payload.lookbackDays.coerceAtLeast(0),
        points = normalizedPoints,
        averageDurationSeconds = resolvedAverage,
        totalDurationSeconds = resolvedTotal,
        activeDays = resolvedActiveDays,
        rangeDays = resolvedRangeDays,
        usesLegacyStatsFallback = hasFallback,
        schemaVersion = payload.schemaVersion,
        usesSchemaVersionFallback = payload.usesSchemaVersionFallback
    )
}

internal fun mapDomainModelToRenderModel(
    model: DomainChartModel,
    selectedRootOverride: String?
): ChartRenderModel {
    val roots = model.roots.distinct()
    val requestedRoot = selectedRootOverride?.trim()?.takeIf { it.isNotEmpty() }
    val resolvedRoot = when {
        requestedRoot != null && roots.contains(requestedRoot) -> requestedRoot
        model.selectedRoot.isNotBlank() && roots.contains(model.selectedRoot) -> model.selectedRoot
        requestedRoot != null -> requestedRoot
        else -> ""
    }

    return ChartRenderModel(
        roots = roots,
        selectedRoot = resolvedRoot,
        lookbackDays = model.lookbackDays,
        points = model.points.map { point ->
            ReportChartPoint(
                date = point.date,
                durationSeconds = point.durationSeconds,
                epochDay = point.epochDay
            )
        },
        averageDurationSeconds = model.averageDurationSeconds,
        totalDurationSeconds = model.totalDurationSeconds,
        activeDays = model.activeDays,
        rangeDays = model.rangeDays,
        usesLegacyStatsFallback = model.usesLegacyStatsFallback,
        schemaVersion = model.schemaVersion,
        usesSchemaVersionFallback = model.usesSchemaVersionFallback
    )
}

private fun parseEpochDayOrNull(dateIso: String): Long? =
    try {
        LocalDate.parse(dateIso).toEpochDay()
    } catch (_: Exception) {
        null
    }
