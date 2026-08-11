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
    configGateway: ConfigGateway,
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
                    txtDocuments = listTextDocumentsInSubdirectory(
                        contentResolver = context.contentResolver,
                        treeUri = treeUri,
                        directoryName = "txt"
                    ),
                    tomlDocuments = listTomlDocumentsInSubdirectory(
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
                    configGateway = configGateway
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
    val txtDocuments: List<TreeTextDocument>,
    val tomlDocuments: List<TreeTextDocument>
)

private suspend fun importDataFolder(
    context: Context,
    input: DataFolderImportInput,
    configGateway: ConfigGateway
): TracerPreparedTransferResult = withContext(Dispatchers.IO) {
    if (input.txtDocuments.isEmpty() && input.tomlDocuments.isEmpty()) {
        return@withContext TracerPreparedTransferResult(
            statusText = "Data folder import skipped: no TXT or TOML files found under txt/ or config/."
        )
    }

    val snapshotGateway = configGateway as? DataFolderSnapshotGateway
        ?: return@withContext TracerPreparedTransferResult(
            statusText = "Data folder import failed: snapshot runtime is unavailable."
        )
    val stagedRoot = File(
        context.cacheDir,
        "time_tracer/data_folder_import/${UUID.randomUUID()}"
    )
    try {
        stageDataFolderSnapshot(context, input, stagedRoot)
        val result = snapshotGateway.replaceDataFolderSnapshot(stagedRoot.absolutePath)
        TracerPreparedTransferResult(
            statusText = if (result.ok) {
                "Data folder replaced: TXT ${result.txtFileCount}, TOML ${result.tomlFileCount}."
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

private fun stageDataFolderSnapshot(
    context: Context,
    input: DataFolderImportInput,
    stagedRoot: File
) {
    val stagedConfig = File(stagedRoot, "config").apply { mkdirs() }
    val stagedInput = File(stagedRoot, "input").apply { mkdirs() }
    for (document in input.tomlDocuments) {
        copyTreeDocument(context, document, stagedConfig)
    }
    for (document in input.txtDocuments) {
        copyTreeDocument(context, document, stagedInput)
    }
}

private fun copyTreeDocument(
    context: Context,
    document: TreeTextDocument,
    targetRoot: File
) {
    val target = safeSnapshotTarget(targetRoot, document.relativePath)
    target.parentFile?.mkdirs()
    context.contentResolver.openInputStream(document.documentUri)?.use { input ->
        target.outputStream().use { output -> input.copyTo(output) }
    } ?: error("unable to read selected document: ${document.relativePath}")
}

private fun safeSnapshotTarget(root: File, relativePath: String): File {
    val canonicalRoot = root.canonicalFile
    val target = File(canonicalRoot, relativePath).canonicalFile
    require(target.toPath().startsWith(canonicalRoot.toPath())) {
        "invalid imported relative path: $relativePath"
    }
    return target
}
