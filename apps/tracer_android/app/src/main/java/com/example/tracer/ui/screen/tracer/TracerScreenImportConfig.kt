package com.example.tracer

import android.content.Context
import android.net.Uri
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
internal fun rememberTracerAndroidConfigImportActions(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    configViewModel: ConfigViewModel
): TracerConfigImportActions {
    var pendingMode by remember { mutableStateOf(AndroidConfigImportMode.FULL_REPLACE) }
    val importConfigLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            configViewModel.setStatusText(context.getString(R.string.tracer_import_config_canceled))
            return@rememberLauncherForActivityResult
        }

        coroutineScope.launch {
            val prepareMessageResId = when (pendingMode) {
                AndroidConfigImportMode.FULL_REPLACE ->
                    R.string.tracer_import_prepare_android_config_full_replace

                AndroidConfigImportMode.PARTIAL_UPDATE ->
                    R.string.tracer_import_prepare_android_config_partial_update
            }
            configViewModel.setStatusText(context.getString(prepareMessageResId))
            val importEntries = runCatching {
                withContext(Dispatchers.IO) {
                    loadTomlEntriesFromSelectedDirectory(context, treeUri)
                }
            }.getOrElse { error ->
                configViewModel.setStatusText(
                    context.getString(
                        R.string.tracer_import_config_failed,
                        error.message ?: context.getString(R.string.tracer_export_unknown_error)
                    )
                )
                return@launch
            }

            val result = when (pendingMode) {
                AndroidConfigImportMode.FULL_REPLACE ->
                    configViewModel.importAndroidConfigBundle(importEntries)

                AndroidConfigImportMode.PARTIAL_UPDATE ->
                    configViewModel.importAndroidConfigPartialUpdate(importEntries)
            }
            configViewModel.setStatusText(result.message)
            if (result.ok) {
                configViewModel.refreshConfigFiles()
            }
        }
    }

    return TracerConfigImportActions(
        onImportAndroidConfigFullReplace = {
            pendingMode = AndroidConfigImportMode.FULL_REPLACE
            configViewModel.setStatusText(
                context.getString(R.string.tracer_import_select_android_config_directory_full_replace)
            )
            importConfigLauncher.launch(null)
        },
        onImportAndroidConfigPartialUpdate = {
            pendingMode = AndroidConfigImportMode.PARTIAL_UPDATE
            configViewModel.setStatusText(
                context.getString(R.string.tracer_import_select_android_config_directory_partial_update)
            )
            importConfigLauncher.launch(null)
        }
    )
}

internal data class TracerConfigImportActions(
    val onImportAndroidConfigFullReplace: () -> Unit,
    val onImportAndroidConfigPartialUpdate: () -> Unit
)

private enum class AndroidConfigImportMode {
    FULL_REPLACE,
    PARTIAL_UPDATE
}

private fun loadTomlEntriesFromSelectedDirectory(
    context: Context,
    treeUri: Uri
): List<ConfigImportEntry> {
    val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
    val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, treeDocumentId)
    val rawEntries = readTextFilesFromDocumentTree(
        contentResolver = context.contentResolver,
        treeUri = treeUri,
        rootDocumentUri = rootDocumentUri
    ) { fileName, mimeType ->
        val byName = fileName.endsWith(".toml", ignoreCase = true)
        val byMime = mimeType.equals("application/toml", ignoreCase = true)
        byName || byMime
    }
    return rawEntries.map { entry ->
        ConfigImportEntry(
            relativePath = entry.relativePath,
            content = entry.content
        )
    }
}
