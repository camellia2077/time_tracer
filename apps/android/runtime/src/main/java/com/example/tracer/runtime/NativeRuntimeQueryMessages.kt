package com.example.tracer

internal fun buildInsightsChartResultMessage(
    pointCount: Int
): String {
    return if (pointCount <= 0) {
        "No chart points."
    } else {
        "Loaded $pointCount chart point(s)."
    }
}

internal fun buildInsightsCompositionResultMessage(
    sliceCount: Int
): String {
    return if (sliceCount <= 0) {
        "No composition slices."
    } else {
        "Loaded $sliceCount composition slice(s)."
    }
}

internal fun buildTreeResultMessage(
    found: Boolean,
    roots: List<String>,
    nodes: List<TreeNode>
): String {
    if (!found) {
        return if (roots.isEmpty()) {
            "No matching tree nodes."
        } else {
            "No matching tree nodes. Available roots: ${roots.size}."
        }
    }
    val nodeCount = countTreeNodes(nodes)
    return if (nodeCount > 0) {
        "Loaded $nodeCount tree node(s)."
    } else {
        "Tree query completed with empty result."
    }
}

internal fun buildFrequentActivitiesResultMessage(
    frequentActivities: List<String>,
    lookbackDays: Int
): String {
    if (lookbackDays == 0) {
        return "Frequent query skipped because lookbackDays=0."
    }
    return if (frequentActivities.isEmpty()) {
        "No frequent activities in the last $lookbackDays days."
    } else {
        "Loaded ${frequentActivities.size} frequent activities."
    }
}
