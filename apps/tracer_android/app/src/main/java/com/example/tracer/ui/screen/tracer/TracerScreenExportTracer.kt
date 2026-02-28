package com.example.tracer

import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import java.io.File

internal data class TracerBatchCryptoExportResult(
    val message: String,
    val progressStatusText: String
)

internal suspend fun exportAllMonthsTracerToTree(
    context: Context,
    treeUri: Uri,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    cryptoGateway: FileCryptoGateway,
    recordViewModel: RecordViewModel,
    passphrase: String,
    tracerSecurityLevel: FileCryptoSecurityLevel
): TracerBatchCryptoExportResult {
    var progressStatusText = "失败"
    val message = runCatching {
        val monthKeys = recordUiState.availableMonths.sorted()
        if (monthKeys.isEmpty()) {
            return@runCatching context.getString(R.string.tracer_export_all_failed_no_months)
        }

        val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
        val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(
            treeUri,
            treeDocumentId
        )
        val datesRootUri = ensureDirectoryChild(
            contentResolver = context.contentResolver,
            treeUri = treeUri,
            parentDocumentUri = rootDocumentUri,
            directoryName = "dates"
        ) ?: return@runCatching context.getString(
            R.string.tracer_export_all_failed_dates_folder
        )

        var successCount = 0
        val errors = mutableListOf<String>()
        val exportItems = buildMonthExportItems(
            context = context,
            recordUiState = recordUiState,
            txtStorageGateway = txtStorageGateway
        )
        val validItems = exportItems.items.filterNotNull()
        val totalCryptoFiles = validItems.size
        val yearTotals = validItems.groupingBy { it.exportYear }.eachCount()
        val yearCompleted = mutableMapOf<String, Int>()
        var completedCryptoFiles = 0

        if (totalCryptoFiles == 0) {
            progressStatusText = "失败"
        }
        for (item in exportItems.items) {
            if (item == null) {
                continue
            }
            val normalizedMonthKey = item.monthKey
            val exportYear = item.exportYear
            val yearDoneBefore = yearCompleted[exportYear] ?: 0
            val yearTotal = yearTotals[exportYear] ?: 1
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
                if (totalCryptoFiles > 0) {
                    val completedAfterFailure =
                        (completedCryptoFiles + 1).coerceAtMost(totalCryptoFiles)
                    val yearDoneAfterFailure = (yearDoneBefore + 1).coerceAtMost(yearTotal)
                    val syntheticProgress = if (totalCryptoFiles == 0) {
                        1f
                    } else {
                        completedAfterFailure.toFloat() / totalCryptoFiles.toFloat()
                    }
                    val folderProgress = if (yearTotal <= 0) {
                        1f
                    } else {
                        yearDoneAfterFailure.toFloat() / yearTotal.toFloat()
                    }
                    runBlocking(Dispatchers.Main) {
                        recordViewModel.updateCryptoProgress(
                            event = FileCryptoProgressEvent(
                                operation = FileCryptoOperation.ENCRYPT,
                                phase = FileCryptoPhase.FAILED,
                                currentGroupLabel = exportYear,
                                fileIndexInGroup = yearDoneAfterFailure,
                                fileCountInGroup = yearTotal,
                                currentFileIndex = completedAfterFailure,
                                totalFiles = totalCryptoFiles,
                                currentFileDoneBytes = 1L,
                                currentFileTotalBytes = 1L,
                                overallDoneBytes = completedAfterFailure.toLong(),
                                overallTotalBytes = totalCryptoFiles.toLong(),
                                remainingBytes = 0L,
                                etaSeconds = 0L
                            ),
                            operationTextOverride = "导出 TRACER",
                            overallProgressOverride = syntheticProgress,
                            overallTextOverride = buildCryptoOverallText(
                                overallProgress = syntheticProgress,
                                completedFiles = completedAfterFailure,
                                totalFiles = totalCryptoFiles
                            ),
                            currentTextOverride = buildCryptoFolderProgressText(
                                yearLabel = exportYear,
                                folderProgress = folderProgress,
                                completedTxt = yearDoneAfterFailure,
                                totalTxt = yearTotal
                            ),
                            currentProgressOverride = folderProgress
                        )
                    }
                    completedCryptoFiles = completedAfterFailure
                    yearCompleted[exportYear] = yearDoneAfterFailure
                }
                continue
            }

            val tmpRoot = File(context.cacheDir, "time_tracer/export_tracer_tmp")
            if (!tmpRoot.exists()) {
                tmpRoot.mkdirs()
            }
            val tmpTxt = File(tmpRoot, "${normalizedMonthKey}_plain.txt")
            val tmpTracer = File(tmpRoot, "${normalizedMonthKey}.tracer")
            tmpTxt.writeText(item.content.removePrefix("\uFEFF"), Charsets.UTF_8)
            val encryptResult = cryptoGateway.encryptTxtFile(
                inputPath = tmpTxt.absolutePath,
                outputPath = tmpTracer.absolutePath,
                passphrase = passphrase,
                securityLevel = tracerSecurityLevel,
                onProgress = { event ->
                    if (totalCryptoFiles <= 0) {
                        return@encryptTxtFile
                    }
                    val currentProgress =
                        event.currentFileProgressFraction.coerceIn(0f, 1f)
                    val overallProgress =
                        (completedCryptoFiles.toFloat() + currentProgress) / totalCryptoFiles.toFloat()
                    val folderProgress = if (yearTotal <= 0) {
                        1f
                    } else {
                        (yearDoneBefore.toFloat() + currentProgress) / yearTotal.toFloat()
                    }
                    val folderCompletedTxt =
                        (yearDoneBefore + if (currentProgress >= 1f) 1 else 0)
                            .coerceAtMost(yearTotal)
                    runBlocking(Dispatchers.Main) {
                        recordViewModel.updateCryptoProgress(
                            event = event,
                            operationTextOverride = "导出 TRACER",
                            overallProgressOverride = overallProgress,
                            overallTextOverride = buildCryptoOverallText(
                                overallProgress = overallProgress,
                                completedFiles = completedCryptoFiles,
                                totalFiles = totalCryptoFiles
                            ),
                            currentTextOverride = buildCryptoFolderProgressText(
                                yearLabel = exportYear,
                                folderProgress = folderProgress,
                                completedTxt = folderCompletedTxt,
                                totalTxt = yearTotal
                            ),
                            currentProgressOverride = folderProgress
                        )
                    }
                }
            )
            completedCryptoFiles = (completedCryptoFiles + 1).coerceAtMost(totalCryptoFiles)
            yearCompleted[exportYear] = (yearDoneBefore + 1).coerceAtMost(yearTotal)
            if (!encryptResult.ok) {
                errors += context.getString(
                    R.string.tracer_export_error_encrypt_failed,
                    normalizedMonthKey,
                    encryptResult.message
                )
                tmpTxt.delete()
                tmpTracer.delete()
                continue
            }

            val exportName = "$normalizedMonthKey.tracer"
            val outputUri = resolveOrCreateDocumentForOverwrite(
                contentResolver = context.contentResolver,
                treeUri = treeUri,
                parentDocumentUri = yearDirUri,
                fileName = exportName,
                mimeType = "application/octet-stream"
            )
            if (outputUri == null) {
                errors += context.getString(
                    R.string.tracer_export_error_create_target_file,
                    normalizedMonthKey
                )
                tmpTxt.delete()
                tmpTracer.delete()
                continue
            }

            val writeOk = runCatching {
                val output = context.contentResolver.openOutputStream(outputUri, "w")
                    ?: error(context.getString(R.string.tracer_export_error_open_output_stream))
                output.use { stream ->
                    tmpTracer.inputStream().use { input ->
                        input.copyTo(stream)
                    }
                }
            }.isSuccess
            tmpTxt.delete()
            tmpTracer.delete()

            if (writeOk) {
                successCount++
            } else {
                errors += context.getString(
                    R.string.tracer_export_error_write_failed,
                    normalizedMonthKey
                )
            }
        }
        errors += exportItems.errors

        if (errors.isEmpty()) {
            progressStatusText = "完成"
            context.resources.getQuantityString(
                R.plurals.tracer_export_all_tracer_success,
                successCount,
                successCount
            )
        } else {
            progressStatusText = if (successCount > 0) "部分失败" else "失败"
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
                R.string.tracer_export_all_tracer_completed,
                successCount,
                monthKeys.size,
                "$head$tail"
            )
        }
    }.getOrElse { error ->
        progressStatusText = "失败"
        context.getString(
            R.string.tracer_export_all_tracer_failed,
            error.message ?: context.getString(R.string.tracer_export_unknown_error)
        )
    }

    return TracerBatchCryptoExportResult(
        message = message,
        progressStatusText = progressStatusText
    )
}
