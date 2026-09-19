package com.example.tracer

import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import android.util.Base64
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

internal data class TracerBatchCryptoExportResult(
    val message: String,
    val progressStatusText: String
)

private const val TRACER_EXCHANGE_EXPORT_ROOT_NAME = "data"
private const val TRACER_EXCHANGE_STAGE_COUNT = 2
private val TRACER_EXCHANGE_EXPORT_NAME_FORMATTER =
    DateTimeFormatter.ofPattern("yyyy-MM-dd_HH-mm-ss")

internal fun buildTimestampedDataExportName(now: LocalDateTime = LocalDateTime.now()): String =
    "data_${now.format(TRACER_EXCHANGE_EXPORT_NAME_FORMATTER)}"

// Complete exchange export is an encrypted standard ZIP for easy human-side
// backup and sharing.
internal suspend fun exportAllMonthsTracerToTree(
    context: Context,
    treeUri: Uri,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    tracerExchangeGateway: TracerExchangeGateway,
    recordViewModel: RecordViewModel,
    passphrase: String,
    tracerSecurityLevel: TracerExchangeSecurityLevel
): TracerBatchCryptoExportResult {
    val completedText = context.getString(R.string.tracer_progress_status_completed)
    val failedText = context.getString(R.string.tracer_progress_status_failed)
    val partialText = context.getString(R.string.tracer_progress_status_partial)
    var progressStatusText = failedText
    val message = runCatching {
        // Complete exchange export must include every valid TXT month from
        // storage, even when the Record tab currently has only one month open.
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
                    )
                )
            }
        )
        val validItems = exportItems.items.filterNotNull()
        if (validItems.isEmpty()) {
            val errors = if (exportItems.errors.isEmpty()) {
                listOf(context.getString(R.string.tracer_export_all_failed_no_months))
            } else {
                exportItems.errors
            }
            progressStatusText = failedText
            return@runCatching buildTracerExchangeExportSummary(
                context = context,
                exportedTxtCount = 0,
                totalTxtCount = exportItems.totalCount,
                converterFileCount = 0,
                manifestFileCount = 0,
                errors = errors
            )
        }

        val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
        val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(
            treeUri,
            treeDocumentId
        )
        val outputFileName = "${buildTimestampedDataExportName()}.zip"
        val outputUri = resolveOrCreateDocumentForOverwrite(
            contentResolver = context.contentResolver,
            treeUri = treeUri,
            parentDocumentUri = rootDocumentUri,
            fileName = outputFileName,
            mimeType = "application/zip"
        )
        if (outputUri == null) {
            progressStatusText = failedText
            return@runCatching buildTracerExchangeExportSummary(
                context = context,
                exportedTxtCount = 0,
                totalTxtCount = exportItems.totalCount,
                converterFileCount = 0,
                manifestFileCount = 0,
                errors = exportItems.errors + context.getString(
                    R.string.tracer_export_error_create_target_file,
                    outputFileName
                )
            )
        }

        val detachedOutputFd = runCatching {
            // The timestamped ZIP may already exist. `wt` is required here so a
            // shorter replacement cannot leave stale ciphertext bytes at the
            // end of the SAF document.
            context.contentResolver.openFileDescriptor(outputUri, "wt")?.use { descriptor ->
                descriptor.detachFd()
            } ?: error(context.getString(R.string.tracer_export_error_open_output_stream))
        }.getOrElse {
            progressStatusText = failedText
            return@runCatching buildTracerExchangeExportSummary(
                context = context,
                exportedTxtCount = 0,
                totalTxtCount = exportItems.totalCount,
                converterFileCount = 0,
                manifestFileCount = 0,
                errors = exportItems.errors + context.getString(
                    R.string.tracer_export_error_write_failed,
                    outputFileName
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
            outputDisplayName = outputFileName,
            onProgress = { event ->
                val overallProgress = event.overallProgressFraction.coerceIn(0f, 1f)
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
                        )
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
                totalTxtCount = exportItems.totalCount,
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

internal suspend fun exportCurrentTxtExchangeDirectoryToTree(
    context: Context,
    treeUri: Uri,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    tracerExchangeGateway: TracerExchangeGateway
): String {
    return runCatching {
        val txtListResult = txtStorageGateway.listTxtFiles()
        if (!txtListResult.ok) {
            return@runCatching context.getString(
                R.string.tracer_export_current_txt_failed,
                txtListResult.message
            )
        }
        val txtPaths = txtListResult.files
            .map { it.replace('\\', '/') }
            .distinct()
            .sorted()

        if (txtPaths.isEmpty()) {
            return@runCatching context.getString(R.string.tracer_export_current_txt_failed_no_selection)
        }
        val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
        val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(
            treeUri,
            treeDocumentId
        )
        val exportRootDocumentUri = resolveOrCreateDirectoryPath(
            contentResolver = context.contentResolver,
            treeUri = treeUri,
            rootDocumentUri = rootDocumentUri,
            relativeDirectoryPath = buildTimestampedDataExportName()
        ) ?: error("Failed to create timestamped data export directory")

        val payloads = txtPaths.map { relativePath ->
            val content = if (recordUiState.selectedHistoryFile.replace('\\', '/') == relativePath) {
                recordUiState.editableHistoryContent
            } else {
                val readResult = txtStorageGateway.readTxtFile(relativePath)
                if (!readResult.ok) {
                    error(
                        context.getString(
                            R.string.tracer_export_error_read_failed,
                            relativePath,
                            readResult.message
                        )
                    )
                }
                readResult.content
            }
            TracerExchangePayloadItem(relativePathHint = relativePath, content = content)
        }

        val contentResult = tracerExchangeGateway.buildTracerExchangeContentFromPayload(
            payloads = payloads,
            logicalSourceRootName = TRACER_EXCHANGE_EXPORT_ROOT_NAME,
            dateCheckMode = NativeBridge.DATE_CHECK_NONE
        )
        if (!contentResult.ok) {
            return@runCatching context.getString(
                R.string.tracer_export_current_txt_failed,
                contentResult.message
            )
        }
        writeTextDocumentToTree(
            context = context,
            treeUri = treeUri,
            rootDocumentUri = exportRootDocumentUri,
            relativePath = "manifest.toml",
            content = contentResult.manifestText,
            mimeType = "application/toml"
        )
        contentResult.entries.forEach { entry ->
            val mimeType = if (entry.relativePath.endsWith(".toml", ignoreCase = true)) {
                "application/toml"
            } else {
                "text/plain"
            }
            if (entry.contentBase64.isBlank()) {
                writeTextDocumentToTree(
                    context = context,
                    treeUri = treeUri,
                    rootDocumentUri = exportRootDocumentUri,
                    relativePath = entry.relativePath,
                    content = entry.content,
                    mimeType = mimeType
                )
            } else {
                writeBytesDocumentToTree(
                    context = context,
                    treeUri = treeUri,
                    rootDocumentUri = exportRootDocumentUri,
                    relativePath = entry.relativePath,
                    bytes = Base64.decode(entry.contentBase64, Base64.DEFAULT),
                    mimeType = "application/octet-stream"
                )
            }
        }

        context.getString(
            R.string.tracer_export_current_txt_completed,
            contentResult.entries.count { it.relativePath.startsWith("payload/") },
            contentResult.entries.count { it.relativePath.startsWith("config/user/") }
        )
    }.getOrElse { error ->
        context.getString(
            R.string.tracer_export_current_txt_failed,
            error.message ?: context.getString(R.string.tracer_export_unknown_error)
        )
    }
}

private fun writeTextDocumentToTree(
    context: Context,
    treeUri: Uri,
    rootDocumentUri: Uri,
    relativePath: String,
    content: String,
    mimeType: String = "text/plain"
) {
    writeBytesDocumentToTree(
        context = context,
        treeUri = treeUri,
        rootDocumentUri = rootDocumentUri,
        relativePath = relativePath,
        bytes = content.toByteArray(Charsets.UTF_8),
        mimeType = mimeType
    )
}

private fun writeBytesDocumentToTree(
    context: Context,
    treeUri: Uri,
    rootDocumentUri: Uri,
    relativePath: String,
    bytes: ByteArray,
    mimeType: String
) {
    val normalizedPath = relativePath.replace('\\', '/').trim('/')
    val parentRelativePath = normalizedPath.substringBeforeLast('/', "")
    val fileName = normalizedPath.substringAfterLast('/').ifBlank {
        error("Invalid export path: $relativePath")
    }
    val parentDocumentUri = resolveOrCreateDirectoryPath(
        contentResolver = context.contentResolver,
        treeUri = treeUri,
        rootDocumentUri = rootDocumentUri,
        relativeDirectoryPath = parentRelativePath
    ) ?: error("Failed to create export directory for $relativePath")
    val outputUri = resolveOrCreateDocumentForOverwrite(
        contentResolver = context.contentResolver,
        treeUri = treeUri,
        parentDocumentUri = parentDocumentUri,
        fileName = fileName,
        mimeType = mimeType
    ) ?: error(context.getString(R.string.tracer_export_error_create_target_file, fileName))
    context.contentResolver.openOutputStream(outputUri, "wt")?.use { output ->
        output.write(bytes)
        output.flush()
    } ?: error(context.getString(R.string.tracer_export_error_write_failed, fileName))
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
    overallText: String
) {
    runBlocking(Dispatchers.Main) {
        recordViewModel.updateCryptoProgress(
            event = TracerExchangeProgressEvent(),
            operationTextOverride = context.getString(R.string.tracer_progress_operation_export_tracer),
            phaseTextOverride = phaseText,
            overallProgressOverride = overallProgress,
            overallTextOverride = overallText
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
