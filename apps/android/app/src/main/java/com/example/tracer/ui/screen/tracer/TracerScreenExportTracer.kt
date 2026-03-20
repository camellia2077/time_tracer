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

private const val TRACER_EXCHANGE_EXPORT_ROOT_NAME = "data"
private const val TRACER_EXCHANGE_STAGE_COUNT = 5
private const val TRACER_EXCHANGE_CONVERTER_FILE_COUNT = 3
private const val TRACER_EXCHANGE_MANIFEST_FILE_COUNT = 1

// Android only exposes complete exchange package export to users.
// Do not reintroduce TXT-only or TOML-only export flows here.
internal suspend fun exportAllMonthsTracerToTree(
    context: Context,
    treeUri: Uri,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    tracerExchangeGateway: TracerExchangeGateway,
    recordViewModel: RecordViewModel,
    passphrase: String,
    tracerSecurityLevel: FileCryptoSecurityLevel
): TracerBatchCryptoExportResult {
    val completedText = context.getString(R.string.tracer_progress_status_completed)
    val failedText = context.getString(R.string.tracer_progress_status_failed)
    val partialText = context.getString(R.string.tracer_progress_status_partial)
    var progressStatusText = failedText
    val message = runCatching {
        val monthKeys = recordUiState.availableMonths.sorted()
        if (monthKeys.isEmpty()) {
            return@runCatching context.getString(R.string.tracer_export_all_failed_no_months)
        }

        val exportItems = buildMonthExportItems(
            context = context,
            recordUiState = recordUiState,
            txtStorageGateway = txtStorageGateway,
            onProgress = { processedCount, totalCount ->
                updateTracerExchangeStageProgress(
                    context = context,
                    recordViewModel = recordViewModel,
                    phaseText = context.getString(R.string.tracer_progress_phase_collect_records),
                    overallProgress = (processedCount.toFloat() / totalCount.toFloat()) * 0.2f,
                    overallText = buildStageOverallText(
                        context = context,
                        stageIndex = 1,
                        stageCount = TRACER_EXCHANGE_STAGE_COUNT,
                        detail = context.getString(
                            R.string.tracer_progress_detail_collect_records,
                            processedCount,
                            totalCount
                        )
                    ),
                    currentText = buildStageCurrentText(
                        context = context,
                        label = context.getString(R.string.tracer_progress_phase_collect_records),
                        progress = processedCount.toFloat() / totalCount.toFloat()
                    ),
                    currentProgress = processedCount.toFloat() / totalCount.toFloat()
                )
            }
        )
        val validItems = exportItems.items.filterNotNull()
        if (validItems.isEmpty()) {
            progressStatusText = failedText
            return@runCatching buildTracerExchangeExportSummary(
                context = context,
                exportedTxtCount = 0,
                totalTxtCount = monthKeys.size,
                converterFileCount = 0,
                manifestFileCount = 0,
                errors = exportItems.errors
            )
        }

        val stagingSessionRoot = buildTracerExchangeExportSessionRoot(context)
        try {
            val stagedInputRoot = File(stagingSessionRoot, TRACER_EXCHANGE_EXPORT_ROOT_NAME)
            updateTracerExchangeStageProgress(
                context = context,
                recordViewModel = recordViewModel,
                phaseText = context.getString(R.string.tracer_progress_phase_collect_config),
                overallProgress = 0.4f,
                overallText = buildStageOverallText(
                    context = context,
                    stageIndex = 2,
                    stageCount = TRACER_EXCHANGE_STAGE_COUNT,
                    detail = context.getString(
                        R.string.tracer_progress_detail_collect_config,
                        TRACER_EXCHANGE_CONVERTER_FILE_COUNT,
                        TRACER_EXCHANGE_CONVERTER_FILE_COUNT
                    )
                ),
                currentText = buildStageCurrentText(
                    context = context,
                    label = context.getString(R.string.tracer_progress_phase_collect_config),
                    progress = 1f
                ),
                currentProgress = 1f
            )
            updateTracerExchangeStageProgress(
                context = context,
                recordViewModel = recordViewModel,
                phaseText = context.getString(R.string.tracer_progress_phase_generate_manifest),
                overallProgress = 0.6f,
                overallText = buildStageOverallText(
                    context = context,
                    stageIndex = 3,
                    stageCount = TRACER_EXCHANGE_STAGE_COUNT,
                    detail = context.getString(
                        R.string.tracer_progress_detail_generate_manifest,
                        TRACER_EXCHANGE_MANIFEST_FILE_COUNT,
                        TRACER_EXCHANGE_MANIFEST_FILE_COUNT
                    )
                ),
                currentText = buildStageCurrentText(
                    context = context,
                    label = context.getString(R.string.tracer_progress_phase_generate_manifest),
                    progress = 1f
                ),
                currentProgress = 1f
            )
            stageTracerExchangePayloads(stagedInputRoot, validItems)
            val stagedOutput = File(stagingSessionRoot, "${stagedInputRoot.name}.tracer")

            val exportResult = tracerExchangeGateway.exportTracerExchange(
                inputPath = stagedInputRoot.absolutePath,
                outputPath = stagedOutput.absolutePath,
                passphrase = passphrase,
                securityLevel = tracerSecurityLevel,
                dateCheckMode = NativeBridge.DATE_CHECK_NONE,
                onProgress = { event ->
                    val overallProgress = event.overallProgressFraction.coerceIn(0f, 1f)
                    val currentProgress = event.currentFileProgressFraction.coerceIn(0f, 1f)
                    runBlocking(Dispatchers.Main) {
                        recordViewModel.updateCryptoProgress(
                            event = event,
                            operationTextOverride = context.getString(
                                R.string.tracer_progress_operation_export_tracer
                            ),
                            phaseTextOverride = context.getString(
                                R.string.tracer_progress_phase_package_and_encrypt
                            ),
                            overallProgressOverride = 0.6f + (overallProgress * 0.3f),
                            overallTextOverride = buildStageOverallText(
                                context = context,
                                stageIndex = 4,
                                stageCount = TRACER_EXCHANGE_STAGE_COUNT,
                                detail = context.getString(
                                    R.string.tracer_progress_detail_package_percent,
                                    (overallProgress * 100f).toInt()
                                )
                            ),
                            currentTextOverride = buildStageCurrentText(
                                context = context,
                                label = stagedOutput.name,
                                progress = currentProgress
                            ),
                            currentProgressOverride = currentProgress
                        )
                    }
                }
            )
            if (!exportResult.ok) {
                progressStatusText = failedText
                return@runCatching context.getString(
                    R.string.tracer_export_all_tracer_failed,
                    exportResult.message
                )
            }

            val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
            val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(
                treeUri,
                treeDocumentId
            )
            val exportName = File(exportResult.outputPath).name.ifBlank {
                "${TRACER_EXCHANGE_EXPORT_ROOT_NAME}.tracer"
            }
            val outputUri = resolveOrCreateDocumentForOverwrite(
                contentResolver = context.contentResolver,
                treeUri = treeUri,
                parentDocumentUri = rootDocumentUri,
                fileName = exportName,
                mimeType = "application/octet-stream"
            )
            if (outputUri == null) {
                progressStatusText = failedText
                return@runCatching buildTracerExchangeExportSummary(
                    context = context,
                    exportedTxtCount = 0,
                    totalTxtCount = monthKeys.size,
                    converterFileCount = 0,
                    manifestFileCount = 0,
                    errors = exportItems.errors + context.getString(
                        R.string.tracer_export_error_create_target_file,
                        exportName
                    )
                )
            }

            updateTracerExchangeStageProgress(
                context = context,
                recordViewModel = recordViewModel,
                phaseText = context.getString(R.string.tracer_progress_phase_write_package_output),
                overallProgress = 0.9f,
                overallText = buildStageOverallText(
                    context = context,
                    stageIndex = 5,
                    stageCount = TRACER_EXCHANGE_STAGE_COUNT,
                    detail = context.getString(R.string.tracer_progress_detail_write_percent, 0)
                ),
                currentText = buildStageCurrentText(
                    context = context,
                    label = exportName,
                    progress = 0f
                ),
                currentProgress = 0f
            )
            val writeOk = runCatching {
                val output = context.contentResolver.openOutputStream(outputUri, "w")
                    ?: error(context.getString(R.string.tracer_export_error_open_output_stream))
                output.use { stream ->
                    stagedOutput.inputStream().use { input ->
                        input.copyTo(stream)
                    }
                }
            }.isSuccess
            val summaryErrors = exportItems.errors.toMutableList()
            if (!writeOk) {
                summaryErrors += context.getString(
                    R.string.tracer_export_error_write_failed,
                    exportName
                )
            } else {
                updateTracerExchangeStageProgress(
                    context = context,
                    recordViewModel = recordViewModel,
                    phaseText = context.getString(R.string.tracer_progress_status_completed),
                    overallProgress = 1f,
                    overallText = buildStageOverallText(
                        context = context,
                        stageIndex = 5,
                        stageCount = TRACER_EXCHANGE_STAGE_COUNT,
                        detail = context.getString(R.string.tracer_progress_detail_write_percent, 100)
                    ),
                    currentText = buildStageCurrentText(
                        context = context,
                        label = exportName,
                        progress = 1f
                    ),
                    currentProgress = 1f
                )
            }

            progressStatusText = when {
                !writeOk -> failedText
                summaryErrors.isEmpty() -> completedText
                else -> partialText
            }
            if (writeOk && summaryErrors.isEmpty()) {
                context.resources.getQuantityString(
                    R.plurals.tracer_export_all_tracer_success,
                    exportResult.payloadFileCount,
                    exportResult.payloadFileCount,
                    exportResult.converterFileCount,
                    if (exportResult.manifestIncluded) 1 else 0
                )
            } else {
                buildTracerExchangeExportSummary(
                    context = context,
                    exportedTxtCount = if (writeOk) exportResult.payloadFileCount else 0,
                    totalTxtCount = monthKeys.size,
                    converterFileCount = if (writeOk) exportResult.converterFileCount else 0,
                    manifestFileCount = if (writeOk && exportResult.manifestIncluded) 1 else 0,
                    errors = summaryErrors
                )
            }
        } finally {
            stagingSessionRoot.deleteRecursively()
        }
    }.getOrElse { error ->
        progressStatusText = failedText
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

private fun buildTracerExchangeExportSessionRoot(context: Context): File {
    val sessionRoot = File(
        context.cacheDir,
        "time_tracer/export_tracer_exchange/${System.currentTimeMillis()}"
    )
    require(sessionRoot.mkdirs()) {
        "cannot create tracer exchange export staging directory: ${sessionRoot.absolutePath}"
    }
    return sessionRoot
}

private fun stageTracerExchangePayloads(
    stagedInputRoot: File,
    items: List<MonthExportItem>
) {
    for (item in items) {
        val yearDir = File(stagedInputRoot, item.exportYear)
        if (!yearDir.exists()) {
            require(yearDir.mkdirs()) {
                "cannot create tracer exchange year directory: ${yearDir.absolutePath}"
            }
        }

        val targetFile = File(yearDir, "${item.monthKey}.txt")
        targetFile.outputStream().use { output ->
            CanonicalTextCodec.writeOutputStream(
                output = output,
                content = item.content
            )
        }
    }
}

private fun buildTracerExchangeExportSummary(
    context: Context,
    exportedTxtCount: Int,
    totalTxtCount: Int,
    converterFileCount: Int,
    manifestFileCount: Int,
    errors: List<String>
): String {
    if (errors.isEmpty()) {
        return context.resources.getQuantityString(
            R.plurals.tracer_export_all_tracer_success,
            exportedTxtCount,
            exportedTxtCount,
            converterFileCount,
            manifestFileCount
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
        R.string.tracer_export_all_tracer_completed,
        exportedTxtCount,
        totalTxtCount,
        converterFileCount,
        manifestFileCount,
        "$head$tail"
    )
}

private fun updateTracerExchangeStageProgress(
    context: Context,
    recordViewModel: RecordViewModel,
    phaseText: String,
    overallProgress: Float,
    overallText: String,
    currentText: String,
    currentProgress: Float
) {
    runBlocking(Dispatchers.Main) {
        recordViewModel.updateCryptoProgress(
            event = FileCryptoProgressEvent(),
            operationTextOverride = context.getString(R.string.tracer_progress_operation_export_tracer),
            phaseTextOverride = phaseText,
            overallProgressOverride = overallProgress,
            overallTextOverride = overallText,
            currentTextOverride = currentText,
            currentProgressOverride = currentProgress
        )
    }
}

private fun buildStageOverallText(
    context: Context,
    stageIndex: Int,
    stageCount: Int,
    detail: String
): String = context.getString(
    R.string.tracer_progress_overall_stage_detail,
    stageIndex,
    stageCount,
    detail
)

private fun buildStageCurrentText(
    context: Context,
    label: String,
    progress: Float
): String {
    val percent = (progress.coerceIn(0f, 1f) * 100f).toInt()
    return context.getString(R.string.tracer_progress_current_stage_detail, label, percent)
}
