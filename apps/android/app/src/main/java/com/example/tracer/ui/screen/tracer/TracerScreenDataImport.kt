package com.example.tracer

import android.content.Context
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.tomlj.Toml
import java.io.File

@Composable
internal fun rememberTracerDataFolderImportAction(
    context: Context,
    coroutineScope: CoroutineScope,
    dataViewModel: DataViewModel,
    configGateway: ConfigGateway,
    configViewModel: ConfigViewModel,
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
                    dataViewModel = dataViewModel,
                    configGateway = configGateway,
                    configViewModel = configViewModel
                )
            },
            formatTransferFailure = { error ->
                TracerPreparedTransferResult(
                    statusText = "Data folder import failed: ${error.message ?: "unknown error"}"
                )
            },
            afterTransfer = { _, _ ->
                recordViewModel.refreshHistory()
                configViewModel.refreshConfigFiles()
            }
        )
    }

    return {
        dataViewModel.setStatusText("Select a folder containing txt/ and config/.")
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
    dataViewModel: DataViewModel,
    configGateway: ConfigGateway,
    configViewModel: ConfigViewModel
): TracerPreparedTransferResult = withContext(Dispatchers.IO) {
    if (input.txtDocuments.isEmpty() && input.tomlDocuments.isEmpty()) {
        return@withContext TracerPreparedTransferResult(
            statusText = "Data folder import skipped: no TXT or TOML files found under txt/ or config/."
        )
    }

    val errors = mutableListOf<String>()
    val tomlContents = mutableListOf<Pair<TreeTextDocument, String>>()
    val aliasDocuments = mutableListOf<AliasHierarchyDocumentInput>()
    val aliasHierarchyGateway = configGateway as? AliasHierarchyGateway
    for (document in input.tomlDocuments) {
        val content = runCatching {
            readTreeTextDocument(context, document)
        }.getOrElse { error ->
            errors += "config/${document.relativePath}: ${error.message ?: "read failed"}"
            continue
        }
        val syntax = Toml.parse(content)
        if (syntax.hasErrors()) {
            errors += "config/${document.relativePath}: ${syntax.errors().joinToString("; ")}"
            continue
        }
        if (document.relativePath.startsWith("aliases/") &&
            document.relativePath != "aliases/_system.toml"
        ) {
            val hierarchy = aliasHierarchyGateway?.describeAliasHierarchy(content)
            if (hierarchy?.ok != true) {
                errors += "config/${document.relativePath}: ${hierarchy?.message ?: "Alias hierarchy runtime is unavailable."}"
                continue
            }
            aliasDocuments += AliasHierarchyDocumentInput(
                sourceName = document.relativePath,
                tomlContent = content
            )
        }
        tomlContents += document to content
    }
    if (errors.isEmpty() && aliasDocuments.isNotEmpty()) {
        val validation = aliasHierarchyGateway?.validateAliasHierarchyDocuments(aliasDocuments)
        if (validation?.ok != true) {
            errors += validation?.message ?: "Alias hierarchy runtime is unavailable."
        }
    }

    val stagedTxt = mutableListOf<StagedDataTxtImportCandidate>()
    for (document in input.txtDocuments) {
        val stagedPath = runCatching {
            stageSelectedTxtDocument(context, document.documentUri)
        }.getOrElse { error ->
            errors += "txt/${document.relativePath}: ${error.message ?: "read failed"}"
            continue
        }
        val candidate = runCatching {
            val content = CanonicalTextCodec.readFile(File(stagedPath))
            val header = parseTxtMonthHeader(content)
                ?: error("TXT is missing valid yYYYY + mMM headers.")
            StagedDataTxtImportCandidate(document.relativePath, stagedPath, header.monthKey)
        }.getOrElse { error ->
            File(stagedPath).delete()
            errors += "txt/${document.relativePath}: ${error.message ?: "invalid TXT"}"
            continue
        }
        stagedTxt += candidate
    }

    val duplicateMonths = stagedTxt.groupBy { it.monthKey }.filterValues { it.size > 1 }
    for ((monthKey, candidates) in duplicateMonths) {
        val paths = candidates.joinToString(", ") { it.relativePath }
        errors += "TXT month $monthKey appears more than once: $paths"
        candidates.forEach { File(it.stagedPath).delete() }
    }

    if (errors.isNotEmpty()) {
        stagedTxt.filterNot { duplicateMonths.containsKey(it.monthKey) }
            .forEach { File(it.stagedPath).delete() }
        return@withContext buildDataFolderImportResult(0, 0, errors)
    }

    var configSuccess = 0
    for ((document, content) in tomlContents) {
        if (isAliasConfigFilePath(document.relativePath)) {
            val error = configViewModel.applyImportedAliasToml(document.relativePath, content)
            if (error == null) configSuccess++ else errors += "config/${document.relativePath}: $error"
        } else {
            val result = configGateway.saveConfigTomlFile(document.relativePath, content)
            if (result.ok) configSuccess++ else errors += "config/${document.relativePath}: ${result.message}"
        }
    }
    var txtSuccess = 0
    for (candidate in stagedTxt) {
        try {
            val result = dataViewModel.ingestSingleTxtReplaceMonthAndGetOperationResult(candidate.stagedPath)
            if (result.operationOk) txtSuccess++ else errors += "txt/${candidate.relativePath}: ${extractImportFailureSummary(result.statusText)}"
        } finally {
            File(candidate.stagedPath).delete()
        }
    }
    buildDataFolderImportResult(txtSuccess, configSuccess, errors)
}

private data class StagedDataTxtImportCandidate(
    val relativePath: String,
    val stagedPath: String,
    val monthKey: String
)

private fun readTreeTextDocument(context: Context, document: TreeTextDocument): String =
    context.contentResolver.openInputStream(document.documentUri)?.use { input ->
        input.reader(Charsets.UTF_8).readText()
    } ?: error("unable to read selected document")

private fun buildDataFolderImportResult(
    txtSuccess: Int,
    configSuccess: Int,
    errors: List<String>
): TracerPreparedTransferResult {
    val summary = "Data folder import: TXT $txtSuccess, TOML $configSuccess" +
        if (errors.isEmpty()) " completed." else ". Errors: ${errors.take(3).joinToString(" | ")}"
    return TracerPreparedTransferResult(statusText = summary)
}
