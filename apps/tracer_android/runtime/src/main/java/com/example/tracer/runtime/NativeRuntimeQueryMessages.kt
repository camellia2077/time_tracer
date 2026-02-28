package com.example.tracer

internal fun buildReportChartResultMessage(
    pointCount: Int
): String {
    return if (pointCount <= 0) {
        "No chart points."
    } else {
        "Loaded $pointCount chart point(s)."
    }
}

internal fun buildTreeResultMessage(
    found: Boolean,
    roots: List<String>,
    nodes: List<TreeNode>,
    usesTextFallback: Boolean = false
): String {
    if (usesTextFallback) {
        return "Loaded tree via legacy text fallback."
    }
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

internal fun buildSuggestionResultMessage(
    suggestions: List<String>,
    lookbackDays: Int
): String {
    return if (suggestions.isEmpty()) {
        "No activity suggestions in recent $lookbackDays days."
    } else {
        "Loaded ${suggestions.size} activity suggestion(s)."
    }
}
