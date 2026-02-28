package com.example.tracer

import android.content.Context
import android.provider.DocumentsContract
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

@Composable
internal fun rememberTracerAndroidConfigExportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    configViewModel: ConfigViewModel
): () -> Unit {
    var pendingConfigExportEntries by remember { mutableStateOf<List<ConfigExportEntry>>(emptyList()) }
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

    return {
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
}
