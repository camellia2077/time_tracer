package com.example.tracer

import android.content.Context

internal fun buildSingleImportOverallText(overallProgress: Float, isTerminal: Boolean): String {
    val percent = (overallProgress.coerceIn(0f, 1f) * 100f).toInt()
    val doneCount = if (isTerminal) 1 else 0
    return "${percent}% | bundle $doneCount/1"
}

internal fun buildSingleImportCurrentText(sourceFileName: String, progress: Float): String {
    val percent = (progress.coerceIn(0f, 1f) * 100f).toInt()
    return "$sourceFileName $percent%"
}

internal fun buildSingleTracerImportSummary(
    context: Context,
    successCount: Int,
    totalCount: Int,
    errors: List<String>
): String {
    if (errors.isEmpty()) {
        return context.getString(
            R.string.tracer_import_single_tracer_completed_success,
            successCount,
            totalCount
        )
    }

    val head = errors.take(3).joinToString(" | ")
    val tail = if (errors.size > 3) {
        context.resources.getQuantityString(
            R.plurals.tracer_export_error_tail,
            errors.size,
            errors.size
        )
    } else {
        ""
    }
    return context.getString(
        R.string.tracer_import_single_tracer_completed_with_errors,
        successCount,
        totalCount,
        "$head$tail"
    )
}

internal fun buildTxtFolderImportSummary(
    context: Context,
    successCount: Int,
    totalCount: Int,
    errors: List<String>
): String {
    if (totalCount <= 0) {
        return context.getString(R.string.tracer_import_txt_folder_no_txt_found)
    }
    if (errors.isEmpty()) {
        return context.getString(
            R.string.tracer_import_txt_folder_completed_success,
            successCount,
            totalCount
        )
    }

    val head = errors.take(3).joinToString(" | ")
    val tail = if (errors.size > 3) {
        context.resources.getQuantityString(
            R.plurals.tracer_export_error_tail,
            errors.size,
            errors.size
        )
    } else {
        ""
    }
    return context.getString(
        R.string.tracer_import_txt_folder_completed_with_errors,
        successCount,
        totalCount,
        "$head$tail"
    )
}
