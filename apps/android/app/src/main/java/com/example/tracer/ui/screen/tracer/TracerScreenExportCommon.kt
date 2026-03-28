package com.example.tracer

internal data class MonthExportItem(
    val monthKey: String,
    val sourceRelativePath: String,
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
    txtStorageGateway: TxtStorageGateway,
    onProgress: ((processedCount: Int, totalCount: Int) -> Unit)? = null
): MonthExportItemsResult {
    val errors = mutableListOf<String>()
    val inspection = txtStorageGateway.inspectTxtFiles()
    if (!inspection.ok) {
        return MonthExportItemsResult(
            items = emptyList(),
            errors = listOf(inspection.message)
        )
    }
    val monthToSourcePath = inspection.entries
        .filter { it.canOpen && !it.headerMonth.isNullOrBlank() }
        .sortedBy { it.relativePath }
        .associateBy(
            keySelector = { it.headerMonth!! },
            valueTransform = { it.relativePath.replace('\\', '/') }
        )

    val totalCount = recordUiState.availableMonths.size.coerceAtLeast(1)
    val items = recordUiState.availableMonths.sorted().mapIndexed { index, monthKey ->
        val normalizedMonthKey = normalizeMonthKey(monthKey)
        if (normalizedMonthKey == null) {
            errors += context.getString(
                R.string.tracer_export_error_invalid_month_key,
                monthKey
            )
            onProgress?.invoke(index + 1, totalCount)
            return@mapIndexed null
        }

        val sourcePath = monthToSourcePath[normalizedMonthKey]
        if (sourcePath == null) {
            errors += context.getString(
                R.string.tracer_export_error_source_file_not_found,
                normalizedMonthKey
            )
            onProgress?.invoke(index + 1, totalCount)
            return@mapIndexed null
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
                onProgress?.invoke(index + 1, totalCount)
                return@mapIndexed null
            }
            readResult.content
        }

        val exportYear = extractExportYearFromFirstLine(content)
        if (exportYear == null) {
            errors += context.getString(
                R.string.tracer_export_error_invalid_txt_header,
                normalizedMonthKey
            )
            onProgress?.invoke(index + 1, totalCount)
            return@mapIndexed null
        }

        onProgress?.invoke(index + 1, totalCount)
        MonthExportItem(
            monthKey = normalizedMonthKey,
            sourceRelativePath = sourcePath,
            exportYear = exportYear,
            content = content
        )
    }

    return MonthExportItemsResult(
        items = items,
        errors = errors
    )
}
