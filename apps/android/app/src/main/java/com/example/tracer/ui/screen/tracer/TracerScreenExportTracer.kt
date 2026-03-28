package com.example.tracer

import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking

internal data class TracerBatchCryptoExportResult(
    val message: String,
    val progressStatusText: String
)

private const val TRACER_EXCHANGE_EXPORT_ROOT_NAME = "data"
private const val TRACER_EXCHANGE_EXPORT_FILE_NAME = "data.tracer"
private const val TRACER_EXCHANGE_STAGE_COUNT = 2

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

        val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
        val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(
            treeUri,
            treeDocumentId
        )
        val outputUri = resolveOrCreateDocumentForOverwrite(
            contentResolver = context.contentResolver,
            treeUri = treeUri,
            parentDocumentUri = rootDocumentUri,
            fileName = TRACER_EXCHANGE_EXPORT_FILE_NAME,
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
                    TRACER_EXCHANGE_EXPORT_FILE_NAME
                )
            )
        }

        val detachedOutputFd = runCatching {
            context.contentResolver.openFileDescriptor(outputUri, "w")?.use { descriptor ->
                descriptor.detachFd()
            } ?: error(context.getString(R.string.tracer_export_error_open_output_stream))
        }.getOrElse {
            progressStatusText = failedText
            return@runCatching buildTracerExchangeExportSummary(
                context = context,
                exportedTxtCount = 0,
                totalTxtCount = monthKeys.size,
                converterFileCount = 0,
                manifestFileCount = 0,
                errors = exportItems.errors + context.getString(
                    R.string.tracer_export_error_write_failed,
                    TRACER_EXCHANGE_EXPORT_FILE_NAME
                )
            )
        }

        val exportResult = tracerExchangeGateway.exportTracerExchangeFromPayload(
            payloads = validItems.map { item ->
                TracerExchangePayloadItem(
                    relativePathHint = item.sourceRelativePath,
                    content = item.content
                )
            },
            outputFd = detachedOutputFd,
            passphrase = passphrase,
            securityLevel = tracerSecurityLevel,
            dateCheckMode = NativeBridge.DATE_CHECK_NONE,
            logicalSourceRootName = TRACER_EXCHANGE_EXPORT_ROOT_NAME,
            outputDisplayName = TRACER_EXCHANGE_EXPORT_FILE_NAME,
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
                        overallProgressOverride = 0.2f + (overallProgress * 0.8f),
                        overallTextOverride = buildStageOverallText(
                            context = context,
                            stageIndex = 2,
                            stageCount = TRACER_EXCHANGE_STAGE_COUNT,
                            detail = context.getString(
                                R.string.tracer_progress_detail_package_percent,
                                (overallProgress * 100f).toInt()
                            )
                        ),
                        currentTextOverride = buildStageCurrentText(
                            context = context,
                            label = TRACER_EXCHANGE_EXPORT_FILE_NAME,
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

        progressStatusText = if (exportItems.errors.isEmpty()) {
            completedText
        } else {
            partialText
        }
        if (exportItems.errors.isEmpty()) {
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
                exportedTxtCount = exportResult.payloadFileCount,
                totalTxtCount = monthKeys.size,
                converterFileCount = exportResult.converterFileCount,
                manifestFileCount = if (exportResult.manifestIncluded) 1 else 0,
                errors = exportItems.errors
            )
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
