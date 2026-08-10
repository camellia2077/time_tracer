package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

@Composable
internal fun rememberTracerTomlFolderImportAction(
    context: Context,
    coroutineScope: kotlinx.coroutines.CoroutineScope,
    dataViewModel: DataViewModel,
    configGateway: ConfigGateway,
    configViewModel: ConfigViewModel
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
    val canceledStatusText = context.getString(R.string.tracer_import_toml_folder_canceled)
    val readSelectedDocumentText =
        context.getString(R.string.tracer_import_error_read_selected_document)

    val importTomlFolderLauncher = rememberLauncherForActivityResult(
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
                listTomlDocumentsRecursively(
                    contentResolver = context.contentResolver,
                    treeUri = treeUri
                )
            },
            formatPrepareFailure = { error ->
                context.getString(
                    R.string.tracer_import_toml_folder_failed,
                    error.message ?: readSelectedDocumentText
                )
            },
            runTransfer = { documents ->
                val errors = mutableListOf<String>()
                var successCount = 0

                for ((index, document) in documents.withIndex()) {
                    if (!isAndroidImportExportUserConfigTomlPath(document.relativePath)) {
                        continue
                    }
                    dataViewModel.setStatusText(
                        buildTomlFolderImportProgressText(
                            current = index + 1,
                            total = documents.size,
                            relativePath = document.relativePath
                        )
                    )
                    val content = runCatching {
                        withContext(Dispatchers.IO) {
                            readSelectedTreeTextDocument(
                                context = context,
                                document = document
                            )
                        }
                    }.getOrElse { error ->
                        errors += "${document.relativePath}: ${error.message ?: readSelectedDocumentText}"
                        continue
                    }

                    val aliasError = if (isAliasConfigFilePath(document.relativePath)) {
                        configViewModel.applyImportedAliasToml(document.relativePath, content)
                    } else {
                        null
                    }
                    val result = if (aliasError == null && !isAliasConfigFilePath(document.relativePath)) {
                        withContext(Dispatchers.IO) {
                            configGateway.saveConfigTomlFile(document.relativePath, content)
                        }
                    } else {
                        null
                    }
                    if (aliasError == null && (result == null || result.ok)) {
                        successCount += 1
                    } else {
                        errors += "${document.relativePath}: ${aliasError ?: result?.message.orEmpty()}"
                    }
                }

                TracerPreparedTransferResult(
                    statusText = buildTomlFolderImportSummary(
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
                        R.string.tracer_import_toml_folder_failed,
                        error.message ?: readSelectedDocumentText
                    )
                )
            },
            afterTransfer = { _, _ ->
                configViewModel.refreshConfigFiles()
            }
        )
    }

    return {
        dataViewModel.setStatusText(context.getString(R.string.tracer_import_select_toml_folder))
        importTomlFolderLauncher.launch(null)
    }
}

private fun readSelectedTreeTextDocument(
    context: Context,
    document: TreeTextDocument
): String {
    return context.contentResolver.openInputStream(document.documentUri)?.use { input ->
        input.reader(Charsets.UTF_8).readText()
    } ?: error(context.getString(R.string.tracer_import_error_read_selected_document))
}

private fun buildTomlFolderImportProgressText(
    current: Int,
    total: Int,
    relativePath: String
): String {
    return "TOML folder import $current/$total -> $relativePath"
}

private fun buildTomlFolderImportSummary(
    context: Context,
    successCount: Int,
    totalCount: Int,
    errors: List<String>
): String {
    if (totalCount <= 0) {
        return context.getString(R.string.tracer_import_toml_folder_no_toml_found)
    }
    if (errors.isEmpty()) {
        return context.getString(
            R.string.tracer_import_toml_folder_completed_success,
            successCount,
            totalCount
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
        R.string.tracer_import_toml_folder_completed_with_errors,
        successCount,
        totalCount,
        "$head$tail"
    )
}
