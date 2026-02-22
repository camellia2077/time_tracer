package com.example.tracer

import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import android.provider.OpenableColumns
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
import java.io.File

@Composable
internal fun rememberTracerSingleTxtImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel
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

            dataViewModel.ingestSingleTxtReplaceMonth(stagedInputPath)
        }
    }

    return {
        dataViewModel.setStatusText("selecting single TXT...")
        importSingleTxtLauncher.launch(arrayOf("text/plain", "text/*", "*/*"))
    }
}

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
                AndroidConfigImportMode.FULL_REPLACE -> {
                    R.string.tracer_import_prepare_android_config_full_replace
                }

                AndroidConfigImportMode.PARTIAL_UPDATE -> {
                    R.string.tracer_import_prepare_android_config_partial_update
                }
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
                AndroidConfigImportMode.FULL_REPLACE -> {
                    configViewModel.importAndroidConfigBundle(importEntries)
                }

                AndroidConfigImportMode.PARTIAL_UPDATE -> {
                    configViewModel.importAndroidConfigPartialUpdate(importEntries)
                }
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

private fun stageSelectedTxtDocument(context: Context, documentUri: Uri): String {
    val stagingRoot = File(context.cacheDir, "time_tracer/single_txt_import")
    if (!stagingRoot.exists()) {
        require(stagingRoot.mkdirs()) {
            "cannot create import staging directory: ${stagingRoot.absolutePath}"
        }
    }

    val displayName = queryDocumentDisplayName(context, documentUri)
    val stagedName = buildStagedFileName(displayName)
    val stagedFile = File(stagingRoot, stagedName)

    val input = context.contentResolver.openInputStream(documentUri)
        ?: throw IllegalArgumentException("cannot open selected document.")
    input.use { source ->
        stagedFile.outputStream().use { output ->
            source.copyTo(output)
        }
    }

    return stagedFile.absolutePath
}

private fun queryDocumentDisplayName(context: Context, documentUri: Uri): String? {
    val projection = arrayOf(OpenableColumns.DISPLAY_NAME)
    context.contentResolver.query(documentUri, projection, null, null, null)?.use { cursor ->
        val nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME)
        if (nameIndex >= 0 && cursor.moveToFirst()) {
            return cursor.getString(nameIndex)
        }
    }
    return null
}

private fun buildStagedFileName(displayName: String?): String {
    val timestampPrefix = System.currentTimeMillis().toString()
    val fallback = "${timestampPrefix}_import.txt"
    val rawName = displayName?.trim().orEmpty()
    if (rawName.isEmpty()) {
        return fallback
    }

    val sanitized = rawName
        .replace(Regex("[^A-Za-z0-9._-]"), "_")
        .trim('_')
        .ifEmpty { "import" }
    val ensuredTxt = if (sanitized.endsWith(".txt", ignoreCase = true)) {
        sanitized
    } else {
        "$sanitized.txt"
    }
    return "${timestampPrefix}_$ensuredTxt"
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
