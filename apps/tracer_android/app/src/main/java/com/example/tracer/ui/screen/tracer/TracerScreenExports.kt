package com.example.tracer

import android.provider.DocumentsContract
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.coroutines.launch

internal data class TracerExportActions(
    val onExportAllMonthsTxt: () -> Unit,
    val isTxtExportInProgress: Boolean,
    val onExportAndroidConfigBundle: () -> Unit
)

@Composable
internal fun rememberTracerExportActions(
    context: android.content.Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    recordUiState: RecordUiState,
    txtStorageGateway: TxtStorageGateway,
    recordViewModel: RecordViewModel,
    configViewModel: ConfigViewModel
): TracerExportActions {
    var pendingConfigExportEntries by remember { mutableStateOf<List<ConfigExportEntry>>(emptyList()) }
    var isTxtExportInProgress by remember { mutableStateOf(false) }

    val exportAllTxtLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_all_canceled))
            isTxtExportInProgress = false
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val message = runCatching {
                withContext(Dispatchers.IO) {
                    val monthKeys = recordUiState.availableMonths.sorted()
                    if (monthKeys.isEmpty()) {
                        return@withContext context.getString(R.string.tracer_export_all_failed_no_months)
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
                    ) ?: return@withContext context.getString(
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

                    if (errors.isEmpty()) {
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
            }.getOrElse { error ->
                context.getString(
                    R.string.tracer_export_all_failed,
                    error.message ?: context.getString(R.string.tracer_export_unknown_error)
                )
            }
            recordViewModel.setStatusText(message)
            isTxtExportInProgress = false
        }
    }
    val onExportAllMonthsTxt: () -> Unit = {
        if (isTxtExportInProgress) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_already_in_progress))
        } else if (recordUiState.availableMonths.isEmpty()) {
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_all_failed_no_months))
        } else {
            isTxtExportInProgress = true
            recordViewModel.setStatusText(context.getString(R.string.tracer_export_select_txt_destination))
            exportAllTxtLauncher.launch(null)
        }
    }

    val exportConfigLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            configViewModel.setStatusText(context.getString(R.string.tracer_export_config_canceled))
            return@rememberLauncherForActivityResult
        }

        val entries = pendingConfigExportEntries
        if (entries.isEmpty()) {
            configViewModel.setStatusText(context.getString(R.string.tracer_export_failed_no_config_entries))
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val message = withContext(Dispatchers.IO) {
                runCatching {
                    val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
                    val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(
                        treeUri,
                        treeDocumentId
                    )
                    var successCount = 0
                    val errors = mutableListOf<String>()

                    for (entry in entries) {
                        val outputUri =
                            resolveOrCreateTextDocumentByRelativePathForOverwrite(
                                contentResolver = context.contentResolver,
                                treeUri = treeUri,
                                rootDocumentUri = rootDocumentUri,
                                relativePath = entry.relativePath
                            )
                        if (outputUri == null) {
                            errors += context.getString(
                                R.string.tracer_export_error_resolve_target,
                                entry.relativePath
                            )
                            continue
                        }

                        val writeOk = runCatching {
                            val output = context.contentResolver.openOutputStream(outputUri, "wt")
                                ?: error(
                                    context.getString(R.string.tracer_export_error_open_destination_stream)
                                )
                            output.use { stream ->
                                writeUtf8Text(
                                    output = stream,
                                    content = entry.content,
                                    includeBom = false
                                )
                            }
                        }.isSuccess

                        if (writeOk) {
                            successCount++
                        } else {
                            errors += context.getString(
                                R.string.tracer_export_error_write_failed_path,
                                entry.relativePath
                            )
                        }
                    }

                    if (errors.isEmpty()) {
                        context.getString(R.string.tracer_export_config_success, successCount)
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
                            R.string.tracer_export_config_completed,
                            successCount,
                            entries.size,
                            "$head$tail"
                        )
                    }
                }.getOrElse { error ->
                    context.getString(
                        R.string.tracer_export_config_failed,
                        error.message ?: context.getString(R.string.tracer_export_unknown_error)
                    )
                }
            }
            configViewModel.setStatusText(message)
        }
    }

    val onExportAndroidConfigBundle: () -> Unit = {
        coroutineScope.launch {
            configViewModel.setStatusText(context.getString(R.string.tracer_export_prepare_android_config))
            val buildResult = withContext(Dispatchers.IO) {
                configViewModel.buildAndroidExportBundle()
            }
            if (!buildResult.ok || buildResult.bundle == null) {
                configViewModel.setStatusText(buildResult.message)
                return@launch
            }
            pendingConfigExportEntries = buildResult.bundle.entries
            configViewModel.setStatusText(
                context.getString(R.string.tracer_export_select_android_config_directory)
            )
            exportConfigLauncher.launch(null)
        }
    }

    return TracerExportActions(
        onExportAllMonthsTxt = onExportAllMonthsTxt,
        isTxtExportInProgress = isTxtExportInProgress,
        onExportAndroidConfigBundle = onExportAndroidConfigBundle
    )
}
