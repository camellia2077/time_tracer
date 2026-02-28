package com.example.tracer

import android.content.ContentResolver
import android.net.Uri
import android.provider.DocumentsContract

private data class DocumentChildEntry(
    val documentId: String,
    val displayName: String,
    val mimeType: String
)

internal data class DocumentTreeTextFile(
    val relativePath: String,
    val content: String
)

internal data class DocumentTreeBinaryFile(
    val relativePath: String,
    val content: ByteArray
)

private fun listDirectChildDocuments(
    contentResolver: ContentResolver,
    treeUri: Uri,
    parentDocumentUri: Uri
): List<DocumentChildEntry> {
    val parentDocumentId = runCatching {
        DocumentsContract.getDocumentId(parentDocumentUri)
    }.getOrNull() ?: return emptyList()
    val childrenUri = DocumentsContract.buildChildDocumentsUriUsingTree(treeUri, parentDocumentId)
    val projection = arrayOf(
        DocumentsContract.Document.COLUMN_DOCUMENT_ID,
        DocumentsContract.Document.COLUMN_DISPLAY_NAME,
        DocumentsContract.Document.COLUMN_MIME_TYPE
    )

    return runCatching {
        contentResolver.query(childrenUri, projection, null, null, null)?.use { cursor ->
            val idIndex = cursor.getColumnIndex(DocumentsContract.Document.COLUMN_DOCUMENT_ID)
            val nameIndex = cursor.getColumnIndex(DocumentsContract.Document.COLUMN_DISPLAY_NAME)
            val mimeIndex = cursor.getColumnIndex(DocumentsContract.Document.COLUMN_MIME_TYPE)
            if (idIndex < 0 || nameIndex < 0 || mimeIndex < 0) {
                return@use emptyList()
            }

            val output = mutableListOf<DocumentChildEntry>()
            while (cursor.moveToNext()) {
                val documentId = cursor.getString(idIndex) ?: continue
                val displayName = cursor.getString(nameIndex) ?: continue
                val mimeType = cursor.getString(mimeIndex) ?: "application/octet-stream"
                output += DocumentChildEntry(
                    documentId = documentId,
                    displayName = displayName,
                    mimeType = mimeType
                )
            }
            output
        } ?: emptyList()
    }.getOrElse { error ->
        throw IllegalStateException(
            "Failed to list child documents under ${parentDocumentUri}. ${error.message.orEmpty()}"
        )
    }
}

private fun findDirectChildDocument(
    contentResolver: ContentResolver,
    treeUri: Uri,
    parentDocumentUri: Uri,
    childName: String
): DocumentChildEntry? {
    if (childName.isBlank()) {
        return null
    }

    val children = listDirectChildDocuments(
        contentResolver = contentResolver,
        treeUri = treeUri,
        parentDocumentUri = parentDocumentUri
    )
    return children.firstOrNull { it.displayName == childName }
}

internal fun readTextFilesFromDocumentTree(
    contentResolver: ContentResolver,
    treeUri: Uri,
    rootDocumentUri: Uri,
    fileSelector: (fileName: String, mimeType: String) -> Boolean
): List<DocumentTreeTextFile> {
    fun readTextDocument(documentUri: Uri): String {
        val input = contentResolver.openInputStream(documentUri)
            ?: throw IllegalStateException("Cannot open input stream for $documentUri")
        return input.bufferedReader(Charsets.UTF_8).use { it.readText() }
    }

    val output = mutableListOf<DocumentTreeTextFile>()

    fun walk(parentUri: Uri, relativeDir: String) {
        val children = listDirectChildDocuments(
            contentResolver = contentResolver,
            treeUri = treeUri,
            parentDocumentUri = parentUri
        )
        for (child in children) {
            val childUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, child.documentId)
            val relativePath = if (relativeDir.isEmpty()) {
                child.displayName
            } else {
                "$relativeDir/${child.displayName}"
            }
            if (child.mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                walk(childUri, relativePath)
                continue
            }
            if (!fileSelector(child.displayName, child.mimeType)) {
                continue
            }
            output += DocumentTreeTextFile(
                relativePath = relativePath,
                content = readTextDocument(childUri)
            )
        }
    }

    walk(rootDocumentUri, "")
    return output.sortedBy { it.relativePath }
}

internal fun readBinaryFilesFromDocumentTree(
    contentResolver: ContentResolver,
    treeUri: Uri,
    rootDocumentUri: Uri,
    fileSelector: (fileName: String, mimeType: String) -> Boolean
): List<DocumentTreeBinaryFile> {
    fun readBinaryDocument(documentUri: Uri): ByteArray {
        val input = contentResolver.openInputStream(documentUri)
            ?: throw IllegalStateException("Cannot open input stream for $documentUri")
        return input.use { it.readBytes() }
    }

    val output = mutableListOf<DocumentTreeBinaryFile>()

    fun walk(parentUri: Uri, relativeDir: String) {
        val children = listDirectChildDocuments(
            contentResolver = contentResolver,
            treeUri = treeUri,
            parentDocumentUri = parentUri
        )
        for (child in children) {
            val childUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, child.documentId)
            val relativePath = if (relativeDir.isEmpty()) {
                child.displayName
            } else {
                "$relativeDir/${child.displayName}"
            }
            if (child.mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
                walk(childUri, relativePath)
                continue
            }
            if (!fileSelector(child.displayName, child.mimeType)) {
                continue
            }
            output += DocumentTreeBinaryFile(
                relativePath = relativePath,
                content = readBinaryDocument(childUri)
            )
        }
    }

    walk(rootDocumentUri, "")
    return output.sortedBy { it.relativePath }
}

internal fun ensureDirectoryChild(
    contentResolver: ContentResolver,
    treeUri: Uri,
    parentDocumentUri: Uri,
    directoryName: String
): Uri? {
    val existing = findDirectChildDocument(
        contentResolver = contentResolver,
        treeUri = treeUri,
        parentDocumentUri = parentDocumentUri,
        childName = directoryName
    )
    if (existing != null) {
        if (existing.mimeType != DocumentsContract.Document.MIME_TYPE_DIR) {
            return null
        }
        return DocumentsContract.buildDocumentUriUsingTree(treeUri, existing.documentId)
    }

    return runCatching {
        DocumentsContract.createDocument(
            contentResolver,
            parentDocumentUri,
            DocumentsContract.Document.MIME_TYPE_DIR,
            directoryName
        )
    }.getOrNull()
}

internal fun resolveOrCreateTextDocumentForOverwrite(
    contentResolver: ContentResolver,
    treeUri: Uri,
    parentDocumentUri: Uri,
    fileName: String
): Uri? {
    return resolveOrCreateDocumentForOverwrite(
        contentResolver = contentResolver,
        treeUri = treeUri,
        parentDocumentUri = parentDocumentUri,
        fileName = fileName,
        mimeType = "text/plain"
    )
}

internal fun resolveOrCreateDocumentForOverwrite(
    contentResolver: ContentResolver,
    treeUri: Uri,
    parentDocumentUri: Uri,
    fileName: String,
    mimeType: String
): Uri? {
    val existing = findDirectChildDocument(
        contentResolver = contentResolver,
        treeUri = treeUri,
        parentDocumentUri = parentDocumentUri,
        childName = fileName
    )
    if (existing != null) {
        if (existing.mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
            return null
        }
        return DocumentsContract.buildDocumentUriUsingTree(treeUri, existing.documentId)
    }

    return runCatching {
        DocumentsContract.createDocument(
            contentResolver,
            parentDocumentUri,
            mimeType,
            fileName
        )
    }.getOrNull()
}

internal fun resolveOrCreateTextDocumentByRelativePathForOverwrite(
    contentResolver: ContentResolver,
    treeUri: Uri,
    rootDocumentUri: Uri,
    relativePath: String
): Uri? {
    val normalizedPath = relativePath
        .trim()
        .replace('\\', '/')
        .trim('/')
    if (normalizedPath.isEmpty()) {
        return null
    }

    val parts = normalizedPath.split('/').filter { it.isNotBlank() }
    if (parts.isEmpty()) {
        return null
    }

    var parentUri = rootDocumentUri
    for (directoryName in parts.dropLast(1)) {
        parentUri = ensureDirectoryChild(
            contentResolver = contentResolver,
            treeUri = treeUri,
            parentDocumentUri = parentUri,
            directoryName = directoryName
        ) ?: return null
    }

    val fileName = parts.last()
    return resolveOrCreateTextDocumentForOverwrite(
        contentResolver = contentResolver,
        treeUri = treeUri,
        parentDocumentUri = parentUri,
        fileName = fileName
    )
}
