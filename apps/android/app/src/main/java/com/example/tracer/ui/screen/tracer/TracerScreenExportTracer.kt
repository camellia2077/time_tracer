package com.example.tracer

import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.runBlocking
import java.io.OutputStreamWriter

internal data class TracerBatchCryptoExportResult(
    val message: String,
    val progressStatusText: String
)

internal data class ConfigTomlExportEntry(
    val sourcePath: String,
    val exportPath: String
)

/** ConfigTomlStorage returns canonical paths under config/user. */
internal fun buildConfigTomlExportEntries(relativePaths: Iterable<String>): List<ConfigTomlExportEntry> {
    return relativePaths
        .map { it.replace('\\', '/').trim('/') }
        .filter { it.isNotBlank() }
        .mapNotNull { sourcePath ->
            if (sourcePath.startsWith("user/") && sourcePath.endsWith(".toml")) {
                ConfigTomlExportEntry(sourcePath = sourcePath, exportPath = sourcePath)
            } else {
                null
            }
        }
        .groupBy { it.exportPath }
        .values
        .map { entries -> entries.minBy { it.sourcePath } }
        .sortedBy { it.exportPath }
}

private const val TRACER_EXCHANGE_EXPORT_ROOT_NAME = "data"
private const val TRACER_EXCHANGE_EXPORT_FILE_NAME = "data.tracer"
private const val TRACER_EXCHANGE_STAGE_COUNT = 2

// Complete exchange export remains encrypted .tracer; current TXT export is a
// plain ZIP for easy human-side backup and sharing.
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
                totalTxtCount = exportItems.totalCount,
                converterFileCount = 0,
                manifestFileCount = 0,
                errors = exportItems.errors + context.getString(
                    R.string.tracer_export_error_create_target_file,
                    TRACER_EXCHANGE_EXPORT_FILE_NAME
                )
            )
        }

        val detachedOutputFd = runCatching {
            // `data.tracer` may already exist. `wt` is required here so a
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

internal suspend fun exportCurrentTxtZipToTree(
    context: Context,
    treeUri: Uri,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    configGateway: ConfigGateway
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

        val configListResult = configGateway.listConfigTomlFiles()
        if (!configListResult.ok) {
            return@runCatching context.getString(
                R.string.tracer_export_current_txt_failed,
                configListResult.message
            )
        }
        val configPaths = buildConfigTomlExportEntries(
            configListResult.userFiles.map { it.relativePath }
        )

        if (txtPaths.isEmpty() && configPaths.isEmpty()) {
            return@runCatching context.getString(R.string.tracer_export_current_txt_failed_no_selection)
        }
        val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
        val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(
            treeUri,
            treeDocumentId
        )

        txtPaths.forEach { relativePath ->
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
            writeTextDocumentToTree(
                context = context,
                treeUri = treeUri,
                rootDocumentUri = rootDocumentUri,
                relativePath = "txt/$relativePath",
                content = content
            )
        }

        configPaths.forEach { configEntry ->
            val readResult = configGateway.readConfigTomlFile(configEntry.sourcePath)
            if (!readResult.ok) {
                error(
                    context.getString(
                        R.string.tracer_export_error_read_failed,
                        configEntry.sourcePath,
                        readResult.message
                    )
                )
            }
            writeTextDocumentToTree(
                context = context,
                treeUri = treeUri,
                rootDocumentUri = rootDocumentUri,
                relativePath = "config/${configEntry.exportPath}",
                content = readResult.content,
                // text/plain makes some SAF providers append .txt to .toml.
                // Keep the TOML MIME so the exported filename remains .toml.
                mimeType = "application/toml"
            )
        }

        context.getString(
            R.string.tracer_export_current_txt_completed,
            txtPaths.size,
            configPaths.size
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
        OutputStreamWriter(output, Charsets.UTF_8).use { writer ->
            writer.write(content)
            writer.flush()
        }
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
