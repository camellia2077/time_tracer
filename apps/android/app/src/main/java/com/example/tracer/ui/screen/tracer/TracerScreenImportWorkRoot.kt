package com.example.tracer

import android.content.Context
import java.io.File

internal fun buildTracerExchangeImportWorkRoot(
    context: Context,
    stagingPrefix: String,
    sourceLabel: String
): File {
    val sanitizedLabel = sourceLabel
        .replace('\\', '_')
        .replace('/', '_')
        .replace(Regex("[^A-Za-z0-9._-]"), "_")
        .ifBlank { "tracer" }
    val root = File(context.cacheDir, "time_tracer/$stagingPrefix")
    if (!root.exists()) {
        require(root.mkdirs()) {
            "cannot create tracer exchange import staging directory: ${root.absolutePath}"
        }
    }
    return File(root, "${System.currentTimeMillis()}_$sanitizedLabel")
}
