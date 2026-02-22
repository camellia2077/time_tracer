package com.example.tracer

internal interface TextStorage {
    fun listTxtFiles(): TxtHistoryListResult
    fun readTxtFile(relativePath: String): TxtFileContentResult
    fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult
    fun resolveIngestInputPath(relativePath: String): String?
    fun resolveSourceTarget(relativePath: String): Pair<String, String>?
}

internal class MultiInputTextStorage(
    private val fullInputPath: String,
    private val smokeInputPath: String,
    private val liveRawInputPath: String,
    private val recordStore: LiveRawRecordStore
) : TextStorage {
    private data class TxtSource(val key: String, val rootPath: String)
    private data class ResolvedSource(val source: TxtSource, val innerPath: String)

    private val sources = listOf(
        TxtSource(key = "full", rootPath = fullInputPath),
        TxtSource(key = "smoke", rootPath = smokeInputPath),
        TxtSource(key = "live_raw", rootPath = liveRawInputPath)
    )

    override fun listTxtFiles(): TxtHistoryListResult {
        val files = mutableListOf<String>()
        for (source in sources) {
            val result = recordStore.listTxtFiles(source.rootPath)
            if (!result.ok) {
                continue
            }
            for (path in result.files) {
                files += withSourcePrefix(source, path)
            }
        }
        val uniqueSortedFiles = files.distinct().sorted()
        return TxtHistoryListResult(
            ok = true,
            files = uniqueSortedFiles,
            message = "Found ${uniqueSortedFiles.size} TXT file(s)."
        )
    }

    override fun readTxtFile(relativePath: String): TxtFileContentResult {
        val resolved = resolveSource(relativePath)
            ?: return TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = "TXT file path is invalid. Use full/... or smoke/... or live_raw/..."
            )

        val result = recordStore.readTxtFile(resolved.source.rootPath, resolved.innerPath)
        return result.copy(
            filePath = withSourcePrefix(resolved.source, result.filePath)
        )
    }

    override fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult {
        val resolved = resolveSource(relativePath)
            ?: return TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = "TXT file path is invalid. Use full/... or smoke/... or live_raw/..."
            )

        val result = recordStore.writeTxtFile(
            liveRawInputPath = resolved.source.rootPath,
            relativePath = resolved.innerPath,
            content = content
        )
        return result.copy(
            filePath = withSourcePrefix(resolved.source, result.filePath)
        )
    }

    override fun resolveIngestInputPath(relativePath: String): String? {
        return resolveSource(relativePath)?.source?.rootPath
    }

    override fun resolveSourceTarget(relativePath: String): Pair<String, String>? {
        val resolved = resolveSource(relativePath) ?: return null
        return resolved.source.rootPath to resolved.innerPath
    }

    private fun resolveSource(relativePath: String): ResolvedSource? {
        val normalized = relativePath.trim().replace('\\', '/')
        if (normalized.isEmpty()) {
            return null
        }

        val slashIndex = normalized.indexOf('/')
        if (slashIndex > 0) {
            val sourceKey = normalized.substring(0, slashIndex)
            val innerPath = normalized.substring(slashIndex + 1)
            if (innerPath.isNotBlank()) {
                val source = sources.firstOrNull { it.key.equals(sourceKey, ignoreCase = true) }
                if (source != null) {
                    return ResolvedSource(source = source, innerPath = innerPath)
                }
            }
        }

        for (source in sources) {
            val result = recordStore.readTxtFile(source.rootPath, normalized)
            if (result.ok) {
                return ResolvedSource(source = source, innerPath = normalized)
            }
        }

        return null
    }

    private fun withSourcePrefix(source: TxtSource, path: String): String {
        val normalized = path.replace('\\', '/').trimStart('/')
        return "${source.key}/$normalized"
    }
}
