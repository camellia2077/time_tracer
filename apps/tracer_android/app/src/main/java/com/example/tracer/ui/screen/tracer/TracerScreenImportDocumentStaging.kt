package com.example.tracer

import android.content.Context
import android.net.Uri
import android.provider.DocumentsContract
import android.provider.OpenableColumns
import java.io.File

internal fun stageSelectedTxtDocument(context: Context, documentUri: Uri): String {
    return stageSelectedDocument(
        context = context,
        documentUri = documentUri,
        expectedExtension = ".txt"
    )
}

internal fun stageSelectedDocument(
    context: Context,
    documentUri: Uri,
    expectedExtension: String
): String {
    val stagingRoot = File(context.cacheDir, "time_tracer/single_doc_import")
    if (!stagingRoot.exists()) {
        require(stagingRoot.mkdirs()) {
            "cannot create import staging directory: ${stagingRoot.absolutePath}"
        }
    }

    val displayName = queryDocumentDisplayName(context, documentUri)
    val stagedName = buildStagedFileName(
        displayName = displayName,
        expectedExtension = expectedExtension
    )
    if (!stagedName.endsWith(expectedExtension, ignoreCase = true)) {
        throw IllegalArgumentException("selected file must end with $expectedExtension.")
    }
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

private fun buildStagedFileName(displayName: String?, expectedExtension: String): String {
    val timestampPrefix = System.currentTimeMillis().toString()
    val fallback = "${timestampPrefix}_import$expectedExtension"
    val rawName = displayName?.trim().orEmpty()
    if (rawName.isEmpty()) {
        return fallback
    }

    val sanitized = rawName
        .replace(Regex("[^A-Za-z0-9._-]"), "_")
        .trim('_')
        .ifEmpty { "import" }
    val ensuredExtension = if (sanitized.endsWith(expectedExtension, ignoreCase = true)) {
        sanitized
    } else {
        "$sanitized$expectedExtension"
    }
    return "${timestampPrefix}_$ensuredExtension"
}

internal fun loadTracerEntriesFromSelectedDirectory(
    context: Context,
    treeUri: Uri
): List<DocumentTreeBinaryFile> {
    val treeDocumentId = DocumentsContract.getTreeDocumentId(treeUri)
    val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, treeDocumentId)
    return readBinaryFilesFromDocumentTree(
        contentResolver = context.contentResolver,
        treeUri = treeUri,
        rootDocumentUri = rootDocumentUri
    ) { fileName, _ ->
        fileName.endsWith(".tracer", ignoreCase = true)
    }
}

internal fun stageTracerContentForBatchImport(
    context: Context,
    relativePath: String,
    content: ByteArray
): String {
    val stagingRoot = File(context.cacheDir, "time_tracer/folder_tracer_import_input")
    if (!stagingRoot.exists()) {
        require(stagingRoot.mkdirs()) {
            "cannot create folder TRACER import staging directory: ${stagingRoot.absolutePath}"
        }
    }

    val leafName = relativePath
        .substringAfterLast('/')
        .substringAfterLast('\\')
    val baseName = leafName
        .removeSuffix(".tracer")
        .replace(Regex("[^A-Za-z0-9._-]"), "_")
        .ifBlank { "import" }
    val stagedFile = File.createTempFile(
        "folder_${baseName}_",
        ".tracer",
        stagingRoot
    )
    stagedFile.writeBytes(content)
    return stagedFile.absolutePath
}
