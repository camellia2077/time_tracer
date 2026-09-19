package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.util.UUID

@Composable
internal fun rememberTracerDataFolderImportAction(
    context: Context,
    coroutineScope: CoroutineScope,
    dataViewModel: DataViewModel,
    tracerExchangeGateway: TracerExchangeGateway,
    activityHierarchyEditorViewModel: ActivityHierarchyEditorViewModel,
    recordViewModel: RecordViewModel,
    onQuickAccessReload: suspend () -> Unit
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
    val launcher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.OpenDocumentTree()
    ) { treeUri ->
        if (treeUri == null) {
            transferCoordinator.handleSelectionCanceled(
                uiCallbacks = transferUiCallbacks,
                canceledStatusText = "Data folder import canceled."
            )
            return@rememberLauncherForActivityResult
        }

        transferCoordinator.launchPreparedTransfer(
            uiCallbacks = transferUiCallbacks,
            prepareInput = {
                DataFolderImportInput(
                    manifestDocument = listTomlDocumentsRecursively(
                        contentResolver = context.contentResolver,
                        treeUri = treeUri
                    ).firstOrNull { it.relativePath == "manifest.toml" },
                    txtDocuments = listTextDocumentsInSubdirectory(
                        contentResolver = context.contentResolver,
                        treeUri = treeUri,
                        directoryName = "payload"
                    ),
                    configDocuments = listAllDocumentsInSubdirectory(
                        contentResolver = context.contentResolver,
                        treeUri = treeUri,
                        directoryName = "config"
                    )
                )
            },
            formatPrepareFailure = { error ->
                "Data folder import failed: ${error.message ?: "unable to read selected folder"}"
            },
            runTransfer = { input ->
                importDataFolder(
                    context = context,
                    input = input,
                    tracerExchangeGateway = tracerExchangeGateway
                )
            },
            formatTransferFailure = { error ->
                TracerPreparedTransferResult(
                    statusText = "Data folder import failed: ${error.message ?: "unknown error"}"
                )
            },
            afterTransfer = { _, result ->
                // Wait for the asynchronous history refresh before reloading Quick Access.
                // The refresh publishes a full RecordUiState snapshot; if it finishes after
                // Quick Access is updated, its stale pre-import snapshot can overwrite the
                // newly imported aliases. Keeping this order makes the final state consistent.
                recordViewModel.refreshHistory().join()
                activityHierarchyEditorViewModel.openActivityCategories()
                if (result.succeeded) {
                    onQuickAccessReload()
                }
            }
        )
    }

    return {
        launcher.launch(null)
    }
}

private data class DataFolderImportInput(
    val manifestDocument: TreeTextDocument?,
    val txtDocuments: List<TreeTextDocument>,
    val configDocuments: List<TreeTextDocument>
)

@Suppress("TooGenericExceptionCaught")
private suspend fun importDataFolder(
    context: Context,
    input: DataFolderImportInput,
    tracerExchangeGateway: TracerExchangeGateway
): TracerPreparedTransferResult = withContext(Dispatchers.IO) {
    if (input.txtDocuments.isEmpty() && input.configDocuments.isEmpty()) {
        return@withContext TracerPreparedTransferResult(
            statusText = "Data folder import skipped: package files are required."
        )
    }

    val stagedRoot = File(
        context.cacheDir,
        "time_tracer/data_folder_import/${UUID.randomUUID()}"
    )
    try {
        stageTracerExchangeDirectory(context, input, stagedRoot)
        val result = tracerExchangeGateway.importTracerExchange(
            inputPath = stagedRoot.absolutePath,
            workRoot = stagedRoot.parentFile?.absolutePath ?: stagedRoot.absolutePath,
            passphrase = ""
        )
        TracerPreparedTransferResult(
            statusText = if (result.ok) {
                "Data folder imported: TXT ${result.payloadFileCount}."
            } else {
                "Data folder import failed: ${result.message}"
            },
            succeeded = result.ok
        )
    } catch (error: Exception) {
        TracerPreparedTransferResult(
            statusText = "Data folder import failed: ${error.message ?: "unknown error"}"
        )
    } finally {
        stagedRoot.deleteRecursively()
    }
}

private fun stageTracerExchangeDirectory(
    context: Context,
    importInput: DataFolderImportInput,
    stagedRoot: File
) {
    val stagedConfig = File(stagedRoot, "config").apply { mkdirs() }
    val stagedPayload = File(stagedRoot, "payload").apply { mkdirs() }
    importInput.manifestDocument?.let { copyTreeDocument(context, it, stagedRoot) }
    for (document in importInput.configDocuments) {
        copyTreeDocument(context, document, stagedConfig)
    }
    for (document in importInput.txtDocuments) {
        copyTreeDocument(context, document, stagedPayload)
    }
}

private fun copyTreeDocument(
    context: Context,
    document: TreeTextDocument,
    targetRoot: File
) {
    val target = safeExchangeDirectoryTarget(targetRoot, document.relativePath)
    target.parentFile?.mkdirs()
    context.contentResolver.openInputStream(document.documentUri)?.use { sourceStream ->
        target.outputStream().use { targetStream -> sourceStream.copyTo(targetStream) }
    } ?: error("unable to read selected document: ${document.relativePath}")
}

private fun safeExchangeDirectoryTarget(targetRoot: File, relativePath: String): File {
    val canonicalTargetRoot = targetRoot.canonicalFile
    val targetFile = File(canonicalTargetRoot, relativePath).canonicalFile
    require(targetFile.toPath().startsWith(canonicalTargetRoot.toPath())) {
        "invalid imported relative path: $relativePath"
    }
    return targetFile
}
