package com.example.tracer

internal suspend fun exportAllMonthsTxtToTree(
    context: android.content.Context,
    treeUri: android.net.Uri,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway
): String {
    val monthKeys = recordUiState.availableMonths.sorted()
    if (monthKeys.isEmpty()) {
        return context.getString(R.string.tracer_export_all_failed_no_months)
    }

    val treeDocumentId = android.provider.DocumentsContract.getTreeDocumentId(treeUri)
    val rootDocumentUri = android.provider.DocumentsContract.buildDocumentUriUsingTree(
        treeUri,
        treeDocumentId
    )
    val datesRootUri = ensureDirectoryChild(
        contentResolver = context.contentResolver,
        treeUri = treeUri,
        parentDocumentUri = rootDocumentUri,
        directoryName = "dates"
    ) ?: return context.getString(
        R.string.tracer_export_all_failed_dates_folder
    )

    var successCount = 0
    val errors = mutableListOf<String>()
    val normalizedHistoryFiles = recordUiState.historyFiles
        .map { it.replace('\\', '/') }
        .sorted()
    val monthToSourcePath = buildMonthToSourcePathIndex(
        historyFiles = normalizedHistoryFiles,
        readTxtFile = { path -> txtStorageGateway.readTxtFile(path) }
    )

    for (monthKey in monthKeys) {
        val normalizedMonthKey = normalizeMonthKey(monthKey)
        if (normalizedMonthKey == null) {
            errors += context.getString(
                R.string.tracer_export_error_invalid_month_key,
                monthKey
            )
            continue
        }

        val sourcePath = monthToSourcePath[normalizedMonthKey]
        if (sourcePath == null) {
            errors += context.getString(
                R.string.tracer_export_error_source_file_not_found,
                normalizedMonthKey
            )
            continue
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
                continue
            }
            readResult.content
        }

        val exportYear = extractExportYearFromFirstLine(content)
        if (exportYear == null) {
            errors += context.getString(
                R.string.tracer_export_error_invalid_txt_header,
                normalizedMonthKey
            )
            continue
        }

        val yearDirUri = ensureDirectoryChild(
            contentResolver = context.contentResolver,
            treeUri = treeUri,
            parentDocumentUri = datesRootUri,
            directoryName = exportYear
        )
        if (yearDirUri == null) {
            errors += context.getString(
                R.string.tracer_export_error_year_folder,
                normalizedMonthKey,
                exportYear
            )
            continue
        }

        val exportName = "$normalizedMonthKey.txt"
        val outputUri = resolveOrCreateTextDocumentForOverwrite(
            contentResolver = context.contentResolver,
            treeUri = treeUri,
            parentDocumentUri = yearDirUri,
            fileName = exportName
        )
        if (outputUri == null) {
            errors += context.getString(
                R.string.tracer_export_error_create_target_file,
                normalizedMonthKey
            )
            continue
        }

        val writeOk = runCatching {
            val output = context.contentResolver.openOutputStream(outputUri, "wt")
                ?: error(context.getString(R.string.tracer_export_error_open_output_stream))
            output.use { stream ->
                writeUtf8Text(
                    output = stream,
                    content = content,
                    includeBom = true
                )
            }
        }.isSuccess

        if (writeOk) {
            successCount++
        } else {
            errors += context.getString(
                R.string.tracer_export_error_write_failed,
                normalizedMonthKey
            )
        }
    }

    return if (errors.isEmpty()) {
        context.resources.getQuantityString(
            R.plurals.tracer_export_all_success,
            successCount,
            successCount
        )
    } else {
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
        context.getString(
            R.string.tracer_export_all_completed,
            successCount,
            monthKeys.size,
            "$head$tail"
        )
    }
}
