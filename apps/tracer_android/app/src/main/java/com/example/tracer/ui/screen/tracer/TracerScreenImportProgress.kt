package com.example.tracer

import android.content.Context

internal fun buildSingleImportOverallText(overallProgress: Float, isTerminal: Boolean): String {
    val percent = (overallProgress.coerceIn(0f, 1f) * 100f).toInt()
    val doneCount = if (isTerminal) 1 else 0
    return "${percent}% | txt $doneCount/1"
}

internal fun buildSingleImportCurrentText(sourceFileName: String, progress: Float): String {
    val percent = (progress.coerceIn(0f, 1f) * 100f).toInt()
    return "$sourceFileName $percent%"
}

internal fun buildFolderTracerImportSummary(
    context: Context,
    successCount: Int,
    totalCount: Int,
    errors: List<String>
): String {
    if (errors.isEmpty()) {
        return context.getString(
            R.string.tracer_import_folder_tracer_completed_success,
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
        R.string.tracer_import_folder_tracer_completed_with_errors,
        successCount,
        totalCount,
        "$head$tail"
    )
}

internal fun buildBatchTracerImportOverallText(
    overallProgress: Float,
    completedFiles: Int,
    totalFiles: Int
): String {
    val percent = (overallProgress.coerceIn(0f, 1f) * 100f).toInt()
    return "${percent}% | file ${completedFiles.coerceAtLeast(0)}/${totalFiles.coerceAtLeast(0)}"
}

internal fun buildBatchTracerImportFolderProgressText(
    folderLabel: String,
    folderProgress: Float,
    completedTxt: Int,
    totalTxt: Int
): String {
    val percent = (folderProgress.coerceIn(0f, 1f) * 100f).toInt()
    val boundedTotal = totalTxt.coerceAtLeast(1)
    val boundedDone = completedTxt.coerceIn(0, boundedTotal)
    return "folder $folderLabel ${percent}% | txt $boundedDone/$boundedTotal"
}

internal fun resolveBatchImportFolderLabel(relativePath: String): String {
    val normalized = relativePath
        .replace('\\', '/')
        .trim('/')
    if (normalized.isBlank()) {
        return "(root)"
    }
    val segments = normalized.split('/').filter { it.isNotBlank() }
    if (segments.size > 1) {
        return segments.first()
    }
    val fileName = segments.first()
    val yearFromFileName = Regex(
        pattern = "^(\\d{4})-\\d{2}\\.tracer$",
        options = setOf(RegexOption.IGNORE_CASE)
    ).find(fileName)?.groupValues?.getOrNull(1)
    return yearFromFileName ?: "(root)"
}
