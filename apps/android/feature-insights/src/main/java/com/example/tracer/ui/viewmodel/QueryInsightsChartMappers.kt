package com.example.tracer

import java.time.LocalDate

internal fun mapCorePayloadToDomainModel(payload: InsightsChartData): DomainChartModel {
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

    return DomainChartModel(
        roots = normalizedRoots,
        rootTree = payload.rootTree,
        selectedRoot = payload.selectedRoot,
        lookbackDays = payload.lookbackDays.coerceAtLeast(0),
        points = normalizedPoints,
        averageDurationSeconds = payload.averageDurationSeconds.coerceAtLeast(0L),
        totalOccurrenceCount = payload.totalOccurrenceCount.coerceAtLeast(0L),
        averageDurationPerOccurrenceSeconds =
            payload.averageDurationPerOccurrenceSeconds.coerceAtLeast(0L),
        modeDurationSeconds = payload.modeDurationSeconds,
        medianDurationSeconds = payload.medianDurationSeconds,
        minimumDurationSeconds = payload.minimumDurationSeconds,
        maximumDurationSeconds = payload.maximumDurationSeconds,
        lowerQuartileDurationSeconds = payload.lowerQuartileDurationSeconds,
        upperQuartileDurationSeconds = payload.upperQuartileDurationSeconds,
        coefficientOfVariation = payload.coefficientOfVariation,
        meanAbsoluteDeviationSeconds = payload.meanAbsoluteDeviationSeconds,
        totalDurationSeconds = payload.totalDurationSeconds.coerceAtLeast(0L),
        activeDays = payload.activeDays.coerceAtLeast(0),
        rangeDays = payload.rangeDays.coerceAtLeast(0),
        schemaVersion = payload.schemaVersion,
        usesSchemaVersionFallback = payload.usesSchemaVersionFallback
    )
}

internal fun mapDomainModelToRenderModel(
    model: DomainChartModel,
    selectedRootOverride: String?,
    fromDateIso: String? = null,
    toDateIso: String? = null
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
        rootTree = model.rootTree,
        selectedRoot = resolvedRoot,
        lookbackDays = model.lookbackDays,
        points = model.points.map { point ->
            InsightsChartPoint(
                date = point.date,
                durationSeconds = point.durationSeconds,
                epochDay = point.epochDay
            )
        },
        averageDurationSeconds = model.averageDurationSeconds,
        totalOccurrenceCount = model.totalOccurrenceCount,
        averageDurationPerOccurrenceSeconds =
            model.averageDurationPerOccurrenceSeconds,
        modeDurationSeconds = model.modeDurationSeconds,
        medianDurationSeconds = model.medianDurationSeconds,
        minimumDurationSeconds = model.minimumDurationSeconds,
        maximumDurationSeconds = model.maximumDurationSeconds,
        lowerQuartileDurationSeconds = model.lowerQuartileDurationSeconds,
        upperQuartileDurationSeconds = model.upperQuartileDurationSeconds,
        coefficientOfVariation = model.coefficientOfVariation,
        meanAbsoluteDeviationSeconds = model.meanAbsoluteDeviationSeconds,
        totalDurationSeconds = model.totalDurationSeconds,
        activeDays = model.activeDays,
        rangeDays = model.rangeDays,
        schemaVersion = model.schemaVersion,
        usesSchemaVersionFallback = model.usesSchemaVersionFallback,
        fromDateIso = fromDateIso,
        toDateIso = toDateIso
    )
}

private fun parseEpochDayOrNull(dateIso: String): Long? =
    try {
        LocalDate.parse(dateIso).toEpochDay()
    } catch (_: Exception) {
        null
    }

internal fun mapCorePayloadToCompositionRenderModel(
    payload: InsightsCompositionData
): CompositionChartRenderModel {
    return CompositionChartRenderModel(
        totalDurationSeconds = payload.totalDurationSeconds.coerceAtLeast(0L),
        activeRootCount = payload.activeRootCount.coerceAtLeast(0),
        activeDays = payload.activeDays.coerceAtLeast(0),
        rangeDays = payload.rangeDays.coerceAtLeast(0),
        averageDayBasis = payload.averageDayBasis,
        averageDenominatorDays = payload.averageDenominatorDays.coerceAtLeast(0),
        displayLevel = payload.displayLevel.coerceAtLeast(0),
        displayPath = payload.displayPath.map(String::trim).filter(String::isNotEmpty),
        tree = normalizeCompositionTree(payload.tree)
    )
}

private fun normalizeCompositionTree(nodes: List<TreeNode>): List<TreeNode> = nodes
    .mapNotNull { node ->
        val name = node.name.trim()
        val durationSeconds = node.durationSeconds?.coerceAtLeast(0L) ?: return@mapNotNull null
        if (name.isEmpty() || durationSeconds <= 0L) {
            return@mapNotNull null
        }
        TreeNode(
            name = name,
            path = node.path.trim(),
            durationSeconds = durationSeconds,
            occurrenceCount = node.occurrenceCount?.coerceAtLeast(0L),
            averageDurationSeconds = node.averageDurationSeconds?.coerceAtLeast(0L),
            averageDurationPerOccurrenceSeconds = node.averageDurationPerOccurrenceSeconds
                ?.coerceAtLeast(0L),
            averageOccurrenceCount = node.averageOccurrenceCount
                ?.takeIf { it.isFinite() }
                ?.coerceAtLeast(0.0),
            averageOccurrenceRatio = node.averageOccurrenceRatio
                ?.coerceIn(0.0, 1.0),
            children = normalizeCompositionTree(node.children)
        )
    }
    .sortedWith(compareByDescending<TreeNode> { it.durationSeconds ?: 0L }.thenBy(TreeNode::name))
