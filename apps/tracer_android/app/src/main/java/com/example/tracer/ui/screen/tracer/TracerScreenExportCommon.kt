package com.example.tracer

internal data class MonthExportItem(
    val monthKey: String,
    val exportYear: String,
    val content: String
)

internal data class MonthExportItemsResult(
    val items: List<MonthExportItem?>,
    val errors: List<String>
)

internal suspend fun buildMonthExportItems(
    context: android.content.Context,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway
): MonthExportItemsResult {
    val errors = mutableListOf<String>()
    val normalizedHistoryFiles = recordUiState.historyFiles
        .map { it.replace('\\', '/') }
        .sorted()
    val monthToSourcePath = buildMonthToSourcePathIndex(
        historyFiles = normalizedHistoryFiles,
        readTxtFile = { path -> txtStorageGateway.readTxtFile(path) }
    )

    val items = recordUiState.availableMonths.sorted().map { monthKey ->
        val normalizedMonthKey = normalizeMonthKey(monthKey)
        if (normalizedMonthKey == null) {
            errors += context.getString(
                R.string.tracer_export_error_invalid_month_key,
                monthKey
            )
            return@map null
        }

        val sourcePath = monthToSourcePath[normalizedMonthKey]
        if (sourcePath == null) {
            errors += context.getString(
                R.string.tracer_export_error_source_file_not_found,
                normalizedMonthKey
            )
            return@map null
        }

        val content = if (
            recordUiState.selectedMonth == normalizedMonthKey &&
            recordUiState.selectedHistoryFile.replace('\\', '/') == sourcePath
        ) {
            recordUiState.editableHistoryContent
        } else {
            val readResult = txtStorageGateway.readTxtFile(sourcePath)
            if (!readResult.ok) {
                errors += context.getString(
                    R.string.tracer_export_error_read_failed,
                    normalizedMonthKey,
                    readResult.message
                )
                return@map null
            }
            readResult.content
        }

        val exportYear = extractExportYearFromFirstLine(content)
        if (exportYear == null) {
            errors += context.getString(
                R.string.tracer_export_error_invalid_txt_header,
                normalizedMonthKey
            )
            return@map null
        }

        MonthExportItem(
            monthKey = normalizedMonthKey,
            exportYear = exportYear,
            content = content
        )
    }

    return MonthExportItemsResult(
        items = items,
        errors = errors
    )
}

internal fun buildCryptoOverallText(
    overallProgress: Float,
    completedFiles: Int,
    totalFiles: Int
): String {
    val percent = (overallProgress.coerceIn(0f, 1f) * 100f).toInt()
    return "${percent}% | txt ${completedFiles.coerceAtLeast(0)}/${totalFiles.coerceAtLeast(0)}"
}

internal fun buildCryptoFolderProgressText(
    yearLabel: String,
    folderProgress: Float,
    completedTxt: Int,
    totalTxt: Int
): String {
    val percent = (folderProgress.coerceIn(0f, 1f) * 100f).toInt()
    val boundedTotal = totalTxt.coerceAtLeast(1)
    val boundedDone = completedTxt.coerceIn(0, boundedTotal)
    return "folder $yearLabel ${percent}% | txt $boundedDone/$boundedTotal"
}
