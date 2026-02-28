package com.example.tracer

import org.json.JSONObject
import java.io.File
import java.time.Instant
import java.time.ZoneOffset
import java.time.format.DateTimeFormatter
import java.util.concurrent.atomic.AtomicLong

internal data class RuntimeDiagnosticRecord(
    val timestampEpochMs: Long,
    val operationId: String,
    val stage: String,
    val ok: Boolean,
    val initialized: Boolean?,
    val message: String,
    val errorLogPath: String = ""
) {
    fun toContractEntry(): RuntimeDiagnosticEntry {
        return RuntimeDiagnosticEntry(
            timestampIso = ISO_INSTANT_FORMATTER.format(Instant.ofEpochMilli(timestampEpochMs)),
            operationId = operationId,
            stage = stage,
            ok = ok,
            initialized = initialized,
            message = message,
            errorLogPath = errorLogPath
        )
    }

    fun toJsonLine(): String {
        return JSONObject()
            .put("timestamp_ms", timestampEpochMs)
            .put("timestamp_iso", ISO_INSTANT_FORMATTER.format(Instant.ofEpochMilli(timestampEpochMs)))
            .put("operation_id", operationId)
            .put("stage", stage)
            .put("ok", ok)
            .put("initialized", initialized)
            .put("message", message)
            .put("error_log_path", errorLogPath)
            .toString()
    }
}

internal class RuntimeOperationIdGenerator {
    private val sequence = AtomicLong(0L)

    fun next(stage: String): String {
        val normalizedStage = stage
            .trim()
            .lowercase()
            .replace(Regex("[^a-z0-9]+"), "_")
            .trim('_')
            .ifBlank { "op" }
        val currentMs = System.currentTimeMillis()
        val currentSeq = sequence.incrementAndGet()
        return "$normalizedStage-$currentMs-$currentSeq"
    }
}

internal class RuntimeDiagnosticsRecorder(
    private val runtimePathsProvider: () -> RuntimePaths?,
    private val maxInMemoryEntries: Int = 200
) {
    private val lock = Any()
    private val recentEntries = ArrayDeque<RuntimeDiagnosticRecord>()

    fun record(entry: RuntimeDiagnosticRecord) {
        synchronized(lock) {
            recentEntries.addFirst(entry)
            while (recentEntries.size > maxInMemoryEntries) {
                recentEntries.removeLast()
            }
        }
        appendToDisk(entry)
    }

    fun recent(limit: Int): List<RuntimeDiagnosticRecord> {
        val boundedLimit = limit.coerceIn(1, maxInMemoryEntries)
        return synchronized(lock) {
            recentEntries.take(boundedLimit)
        }
    }

    fun diagnosticsLogPath(): String {
        val paths = runtimePathsProvider() ?: return ""
        return File(paths.outputRoot, "logs/diagnostics.jsonl").absolutePath
    }

    private fun appendToDisk(entry: RuntimeDiagnosticRecord) {
        val logPath = diagnosticsLogPath()
        if (logPath.isBlank()) {
            return
        }

        runCatching {
            val logFile = File(logPath)
            logFile.parentFile?.mkdirs()
            logFile.appendText(entry.toJsonLine() + "\n")
        }
    }
}

private val ISO_INSTANT_FORMATTER: DateTimeFormatter =
    DateTimeFormatter.ISO_INSTANT.withZone(ZoneOffset.UTC)
