package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.io.File

@Composable
internal fun rememberTracerTxtFolderImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    recordViewModel: RecordViewModel
): () -> Unit {
    val transferCoordinator = rememberTracerScreenTransferCoordinator(
        context = context,
        coroutineScope = coroutineScope
    )
    val transferUiCallbacks = remember(dataViewModel) {
        TracerTransferUiCallbacks(
            setStatusText = dataViewModel::setStatusText,
            clearCryptoProgress = {},
            startCryptoProgress = {},
            finishCryptoProgress = { _, _ -> }
        )
    }
    val canceledStatusText = context.getString(R.string.tracer_import_txt_folder_canceled)
    val readSelectedDocumentText =
        context.getString(R.string.tracer_import_error_read_selected_document)

    val importTxtFolderLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            transferCoordinator.handleSelectionCanceled(
                uiCallbacks = transferUiCallbacks,
                canceledStatusText = canceledStatusText
            )
            return@rememberLauncherForActivityResult
        }

        transferCoordinator.launchPreparedTransfer(
            uiCallbacks = transferUiCallbacks,
            prepareInput = {
                listTextDocumentsRecursively(
                    contentResolver = context.contentResolver,
                    treeUri = treeUri
                )
            },
            formatPrepareFailure = { error ->
                context.getString(
                    R.string.tracer_import_txt_folder_failed,
                    error.message ?: readSelectedDocumentText
                )
            },
            runTransfer = { documents ->
                val errors = mutableListOf<String>()
                var successCount = 0
                val stagedCandidates = mutableListOf<StagedTxtImportCandidate>()

                for ((index, document) in documents.withIndex()) {
                    dataViewModel.setStatusText(
                        buildTxtFolderImportProgressText(
                            current = index + 1,
                            total = documents.size,
                            relativePath = document.relativePath
                        )
                    )
                    val stagedInputPath = runCatching {
                        withContext(Dispatchers.IO) {
                            stageSelectedTxtDocument(context = context, documentUri = document.documentUri)
                        }
                    }.getOrElse { error ->
                        errors += "${document.relativePath}: ${error.message ?: readSelectedDocumentText}"
                        continue
                    }

                    val candidate = runCatching {
                        withContext(Dispatchers.IO) {
                            inspectStagedTxtImportCandidate(
                                relativePath = document.relativePath,
                                stagedInputPath = stagedInputPath
                            )
                        }
                    }.getOrElse { error ->
                        File(stagedInputPath).delete()
                        errors += "${document.relativePath}: ${error.message ?: readSelectedDocumentText}"
                        continue
                    }
                    stagedCandidates += candidate
                }

                val duplicateMonths = stagedCandidates
                    .groupBy { it.monthKey }
                    .filterValues { it.size > 1 }
                for ((monthKey, candidates) in duplicateMonths) {
                    val paths = candidates.map { it.relativePath }
                    val conflicts = paths.joinToString(", ")
                    candidates.forEach { candidate ->
                        errors += "${candidate.relativePath}: duplicate month $monthKey in selected batch ($conflicts)"
                        File(candidate.stagedInputPath).delete()
                    }
                }

                val importableCandidates = stagedCandidates
                    .filterNot { duplicateMonths.containsKey(it.monthKey) }

                for ((index, candidate) in importableCandidates.withIndex()) {
                    dataViewModel.setStatusText(
                        buildTxtFolderImportProgressText(
                            current = index + 1,
                            total = importableCandidates.size.coerceAtLeast(1),
                            relativePath = candidate.relativePath
                        )
                    )

                    try {
                        val result = dataViewModel
                            .ingestSingleTxtReplaceMonthAndGetOperationResult(candidate.stagedInputPath)
                        if (result.operationOk) {
                            successCount += 1
                        } else {
                            errors += "${candidate.relativePath}: ${extractImportFailureSummary(result.statusText)}"
                        }
                    } finally {
                        File(candidate.stagedInputPath).delete()
                    }
                }

                TracerPreparedTransferResult(
                    statusText = buildTxtFolderImportSummary(
                        context = context,
                        successCount = successCount,
                        totalCount = documents.size,
                        errors = errors
                    )
                )
            },
            formatTransferFailure = { error ->
                TracerPreparedTransferResult(
                    statusText = context.getString(
                        R.string.tracer_import_txt_folder_failed,
                        error.message ?: readSelectedDocumentText
                    )
                )
            },
            afterTransfer = { _, _ ->
                // Keep Data tab export availability in sync with TXT store state,
                // even when ingest returns non-ok (e.g. partial/diagnostic response).
                recordViewModel.refreshHistory()
            }
        )
    }

    return {
        dataViewModel.setStatusText(context.getString(R.string.tracer_import_select_txt_folder))
        importTxtFolderLauncher.launch(null)
    }
}

private fun buildTxtFolderImportProgressText(
    current: Int,
    total: Int,
    relativePath: String
): String {
    return "TXT folder import $current/$total -> $relativePath"
}

private fun extractImportFailureSummary(statusText: String): String {
    val raw = statusText.substringAfter("->", statusText).trim()
    if (raw.isBlank()) {
        return "import failed"
    }
    return runCatching {
        val json = JSONObject(raw)
        json.optString("error_message")
            .takeIf { it.isNotBlank() }
            ?: json.optString("content").takeIf { it.isNotBlank() }
            ?: raw
    }.getOrElse { raw }
}

private data class StagedTxtImportCandidate(
    val relativePath: String,
    val stagedInputPath: String,
    val monthKey: String
)

private fun inspectStagedTxtImportCandidate(
    relativePath: String,
    stagedInputPath: String
): StagedTxtImportCandidate {
    val content = CanonicalTextCodec.readFile(File(stagedInputPath))
    val header = parseTxtMonthHeader(content)
        ?: throw IllegalArgumentException("TXT is missing valid yYYYY + mMM headers.")
    return StagedTxtImportCandidate(
        relativePath = relativePath,
        stagedInputPath = stagedInputPath,
        monthKey = header.monthKey
    )
}
