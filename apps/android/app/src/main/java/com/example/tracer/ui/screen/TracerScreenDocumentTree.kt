package com.example.tracer

import android.content.ContentResolver
import android.net.Uri
import android.provider.DocumentsContract

private data class DocumentChildEntry(
    val documentId: String,
    val displayName: String,
    val mimeType: String
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

internal data class TreeTextDocument(
    val documentUri: Uri,
    val relativePath: String
)

internal fun listTextDocumentsRecursively(
    contentResolver: ContentResolver,
    treeUri: Uri
): List<TreeTextDocument> {
    val rootDocumentId = runCatching {
        DocumentsContract.getTreeDocumentId(treeUri)
    }.getOrNull() ?: return emptyList()
    val rootDocumentUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, rootDocumentId)
    val output = mutableListOf<TreeTextDocument>()
    collectTextDocumentsRecursively(
        contentResolver = contentResolver,
        treeUri = treeUri,
        parentDocumentUri = rootDocumentUri,
        currentRelativeDir = "",
        output = output
    )
    return output.sortedBy { it.relativePath }
}

private fun collectTextDocumentsRecursively(
    contentResolver: ContentResolver,
    treeUri: Uri,
    parentDocumentUri: Uri,
    currentRelativeDir: String,
    output: MutableList<TreeTextDocument>
) {
    val children = listDirectChildDocuments(
        contentResolver = contentResolver,
        treeUri = treeUri,
        parentDocumentUri = parentDocumentUri
    )
    for (child in children) {
        val childDocumentUri = DocumentsContract.buildDocumentUriUsingTree(treeUri, child.documentId)
        if (child.mimeType == DocumentsContract.Document.MIME_TYPE_DIR) {
            val nextRelativeDir = if (currentRelativeDir.isBlank()) {
                child.displayName
            } else {
                "$currentRelativeDir/${child.displayName}"
            }
            collectTextDocumentsRecursively(
                contentResolver = contentResolver,
                treeUri = treeUri,
                parentDocumentUri = childDocumentUri,
                currentRelativeDir = nextRelativeDir,
                output = output
            )
            continue
        }

        if (!child.displayName.endsWith(".txt", ignoreCase = true)) {
            continue
        }

        val relativePath = if (currentRelativeDir.isBlank()) {
            child.displayName
        } else {
            "$currentRelativeDir/${child.displayName}"
        }
        output += TreeTextDocument(
            documentUri = childDocumentUri,
            relativePath = relativePath
        )
    }
}
