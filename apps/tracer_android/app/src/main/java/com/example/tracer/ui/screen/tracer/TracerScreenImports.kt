package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.runBlocking
import kotlinx.coroutines.withContext
import java.io.File

@Composable
internal fun rememberTracerSingleTxtImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel
): () -> Unit {
    val importSingleTxtLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { documentUri ->
        if (documentUri == null) {
            dataViewModel.setStatusText("single TXT import canceled.")
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val stagedInputPath = runCatching {
                withContext(Dispatchers.IO) {
                    stageSelectedTxtDocument(context = context, documentUri = documentUri)
                }
            }.getOrElse { error ->
                dataViewModel.setStatusText(
                    "single TXT import failed: ${error.message ?: "cannot read selected document."}"
                )
                return@launch
            }

            val ingestOk = dataViewModel.ingestSingleTxtReplaceMonthAndGetResult(stagedInputPath)
            // Keep Data tab export availability in sync with TXT store state,
            // even when ingest returns non-ok (e.g. partial/diagnostic response).
            recordViewModel.refreshHistory()
            if (!ingestOk) {
                return@launch
            }
        }
    }

    return {
        dataViewModel.setStatusText("selecting single TXT...")
        importSingleTxtLauncher.launch(arrayOf("text/plain", "text/*", "*/*"))
    }
}

@Composable
internal fun rememberTracerFolderTracerImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel,
    cryptoGateway: FileCryptoGateway
): () -> Unit {
    val importFolderTracerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            recordViewModel.clearCryptoProgress()
            dataViewModel.setStatusText(context.getString(R.string.tracer_import_folder_tracer_canceled))
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val tracerEntries = runCatching {
                withContext(Dispatchers.IO) {
                    loadTracerEntriesFromSelectedDirectory(context, treeUri)
                }
            }.getOrElse { error ->
                dataViewModel.setStatusText(
                    context.getString(
                        R.string.tracer_import_folder_tracer_failed,
                        error.message ?: context.getString(R.string.tracer_export_unknown_error)
                    )
                )
                return@launch
            }
            if (tracerEntries.isEmpty()) {
                dataViewModel.setStatusText(context.getString(R.string.tracer_import_folder_tracer_no_files))
                return@launch
            }

            val passphrase = promptPassphrase(
                context = context,
                title = context.getString(R.string.tracer_crypto_passphrase_decrypt_title),
                firstHint = context.getString(R.string.tracer_crypto_passphrase_hint),
                secondHint = null,
                requiredMessage = context.getString(R.string.tracer_crypto_passphrase_required),
                mismatchMessage = context.getString(R.string.tracer_crypto_passphrase_mismatch)
            )
            if (passphrase.isNullOrBlank()) {
                recordViewModel.clearCryptoProgress()
                dataViewModel.setStatusText(context.getString(R.string.tracer_import_folder_tracer_canceled))
                return@launch
            }

            recordViewModel.startCryptoProgress("导入目录 TRACER")
            var progressStatusText = "失败"
            var successCount = 0
            var completedFiles = 0
            val errors = mutableListOf<String>()
            val totalFiles = tracerEntries.size
            val folderTotals = tracerEntries
                .groupingBy { resolveBatchImportFolderLabel(it.relativePath) }
                .eachCount()
            val folderCompleted = mutableMapOf<String, Int>()
            for (entry in tracerEntries) {
                val folderLabel = resolveBatchImportFolderLabel(entry.relativePath)
                val folderDoneBefore = folderCompleted[folderLabel] ?: 0
                val folderTotal = folderTotals[folderLabel] ?: 1
                val stagedTracerPath = runCatching {
                    withContext(Dispatchers.IO) {
                        stageTracerContentForBatchImport(
                            context = context,
                            relativePath = entry.relativePath,
                            content = entry.content
                        )
                    }
                }.getOrElse { error ->
                    errors += "${entry.relativePath}: ${error.message ?: context.getString(R.string.tracer_export_unknown_error)}"
                    completedFiles = (completedFiles + 1).coerceAtMost(totalFiles)
                    folderCompleted[folderLabel] = (folderDoneBefore + 1).coerceAtMost(folderTotal)
                    null
                } ?: continue
                val stagedTxtPath = withContext(Dispatchers.IO) {
                    val outputRoot = File(context.cacheDir, "time_tracer/folder_tracer_import")
                    if (!outputRoot.exists()) {
                        outputRoot.mkdirs()
                    }
                    File(
                        outputRoot,
                        "${System.currentTimeMillis()}_${completedFiles}_import.txt"
                    ).absolutePath
                }

                val decryptResult = withContext(Dispatchers.IO) {
                    cryptoGateway.decryptTracerFile(
                        inputPath = stagedTracerPath,
                        outputPath = stagedTxtPath,
                        passphrase = passphrase,
                        onProgress = { event ->
                            if (totalFiles <= 0) {
                                return@decryptTracerFile
                            }
                            val currentProgress =
                                event.currentFileProgressFraction.coerceIn(0f, 1f)
                            val overallProgress =
                                (completedFiles.toFloat() + currentProgress) / totalFiles.toFloat()
                            val folderProgress = if (folderTotal <= 0) {
                                1f
                            } else {
                                (folderDoneBefore.toFloat() + currentProgress) / folderTotal.toFloat()
                            }
                            val folderCompletedTxt =
                                (folderDoneBefore + if (currentProgress >= 1f) 1 else 0)
                                    .coerceAtMost(folderTotal)
                            runBlocking(Dispatchers.Main) {
                                recordViewModel.updateCryptoProgress(
                                    event = event,
                                    operationTextOverride = "导入目录 TRACER",
                                    overallProgressOverride = overallProgress,
                                    overallTextOverride = buildBatchTracerImportOverallText(
                                        overallProgress = overallProgress,
                                        completedFiles = completedFiles,
                                        totalFiles = totalFiles
                                    ),
                                    currentTextOverride = buildBatchTracerImportFolderProgressText(
                                        folderLabel = folderLabel,
                                        folderProgress = folderProgress,
                                        completedTxt = folderCompletedTxt,
                                        totalTxt = folderTotal
                                    ),
                                    currentProgressOverride = folderProgress
                                )
                            }
                        }
                    )
                }
                completedFiles = (completedFiles + 1).coerceAtMost(totalFiles)
                folderCompleted[folderLabel] = (folderDoneBefore + 1).coerceAtMost(folderTotal)
                if (!decryptResult.ok) {
                    errors += "${entry.relativePath}: ${decryptResult.message}"
                    continue
                }

                val ingestOk = dataViewModel.ingestSingleTxtReplaceMonthAndGetResult(stagedTxtPath)
                if (ingestOk) {
                    successCount += 1
                } else {
                    val detail = dataViewModel.uiState.statusText.ifBlank {
                        context.getString(R.string.tracer_export_unknown_error)
                    }
                    errors += "${entry.relativePath}: $detail"
                }
            }

            // Keep Data tab export availability in sync with TXT store state,
            // even when ingest returns non-ok (e.g. partial/diagnostic response).
            recordViewModel.refreshHistory()
            progressStatusText = when {
                errors.isEmpty() -> "完成"
                successCount > 0 -> "部分失败"
                else -> "失败"
            }
            recordViewModel.finishCryptoProgress(
                statusText = progressStatusText,
                keepVisible = true
            )
            dataViewModel.setStatusText(
                buildFolderTracerImportSummary(
                    context = context,
                    successCount = successCount,
                    totalCount = totalFiles,
                    errors = errors
                )
            )
        }
    }

    return {
        dataViewModel.setStatusText(context.getString(R.string.tracer_import_select_tracer_folder))
        importFolderTracerLauncher.launch(null)
    }
}

@Composable
internal fun rememberTracerSingleTracerImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel,
    cryptoGateway: FileCryptoGateway
): () -> Unit {
    val importSingleTracerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocument()
    ) { documentUri ->
        if (documentUri == null) {
            recordViewModel.clearCryptoProgress()
            dataViewModel.setStatusText(context.getString(R.string.tracer_import_single_tracer_canceled))
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val stagedTracerPath = runCatching {
                withContext(Dispatchers.IO) {
                    stageSelectedDocument(
                        context = context,
                        documentUri = documentUri,
                        expectedExtension = ".tracer"
                    )
                }
            }.getOrElse { error ->
                dataViewModel.setStatusText(
                    context.getString(
                        R.string.tracer_import_single_tracer_failed,
                        error.message ?: context.getString(R.string.tracer_export_unknown_error)
                    )
                )
                return@launch
            }

            val passphrase = promptPassphrase(
                context = context,
                title = context.getString(R.string.tracer_crypto_passphrase_decrypt_title),
                firstHint = context.getString(R.string.tracer_crypto_passphrase_hint),
                secondHint = null,
                requiredMessage = context.getString(R.string.tracer_crypto_passphrase_required),
                mismatchMessage = context.getString(R.string.tracer_crypto_passphrase_mismatch)
            )
            if (passphrase.isNullOrBlank()) {
                recordViewModel.clearCryptoProgress()
                dataViewModel.setStatusText(context.getString(R.string.tracer_import_single_tracer_canceled))
                return@launch
            }

            recordViewModel.startCryptoProgress("导入 TRACER")
            val sourceFileName = File(stagedTracerPath).name
            val stagedTxtPath = withContext(Dispatchers.IO) {
                val outputRoot = File(context.cacheDir, "time_tracer/single_tracer_import")
                if (!outputRoot.exists()) {
                    outputRoot.mkdirs()
                }
                File(
                    outputRoot,
                    "${System.currentTimeMillis()}_import.txt"
                ).absolutePath
            }
            val decryptResult = withContext(Dispatchers.IO) {
                cryptoGateway.decryptTracerFile(
                    inputPath = stagedTracerPath,
                    outputPath = stagedTxtPath,
                    passphrase = passphrase,
                    onProgress = { event ->
                        val overallProgress = event.currentFileProgressFraction.coerceIn(0f, 1f)
                        runBlocking(Dispatchers.Main) {
                            recordViewModel.updateCryptoProgress(
                                event = event,
                                operationTextOverride = "导入 TRACER",
                                overallProgressOverride = overallProgress,
                                overallTextOverride = buildSingleImportOverallText(
                                    overallProgress = overallProgress,
                                    isTerminal = event.phase.isTerminal
                                ),
                                currentTextOverride = buildSingleImportCurrentText(
                                    sourceFileName = sourceFileName,
                                    progress = overallProgress
                                )
                            )
                        }
                    }
                )
            }
            if (!decryptResult.ok) {
                recordViewModel.finishCryptoProgress(statusText = "失败", keepVisible = true)
                dataViewModel.setStatusText(
                    context.getString(
                        R.string.tracer_import_single_tracer_failed,
                        decryptResult.message
                    )
                )
                return@launch
            }

            val ingestOk = dataViewModel.ingestSingleTxtReplaceMonthAndGetResult(stagedTxtPath)
            // Keep Data tab export availability in sync with TXT store state,
            // even when ingest returns non-ok (e.g. partial/diagnostic response).
            recordViewModel.refreshHistory()
            if (ingestOk) {
                recordViewModel.finishCryptoProgress(statusText = "完成", keepVisible = true)
            } else {
                recordViewModel.finishCryptoProgress(statusText = "失败", keepVisible = true)
            }
        }
    }

    return {
        dataViewModel.setStatusText(context.getString(R.string.tracer_import_select_single_tracer))
        importSingleTracerLauncher.launch(arrayOf("*/*"))
    }
}
