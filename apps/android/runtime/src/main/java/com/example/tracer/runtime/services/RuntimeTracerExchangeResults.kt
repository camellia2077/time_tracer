package com.example.tracer

import org.json.JSONObject

internal object RuntimeTracerExchangeResults {
    fun parseContentObject(content: String): JSONObject {
        if (content.isBlank()) {
            return JSONObject()
        }
        return runCatching { JSONObject(content) }.getOrDefault(JSONObject())
    }

    fun exportFailure(message: String): TracerExchangeExportResult =
        TracerExchangeExportResult(
            ok = false,
            message = message,
            outputPath = "",
            sourceRootName = "",
            payloadFileCount = 0,
            converterFileCount = 0,
            manifestIncluded = false
        )

    fun importFailure(message: String): TracerExchangeImportResult =
        TracerExchangeImportResult(
            ok = false,
            message = message,
            sourceRootName = "",
            payloadFileCount = 0
        )

    fun inspectFailure(message: String): TracerExchangeInspectResult =
        TracerExchangeInspectResult(
            ok = false,
            message = message,
            renderedText = "",
            inputPath = "",
            sourceRootName = "",
            payloadFileCount = 0,
            producerPlatform = "",
            producerApp = "",
            createdAtUtc = ""
        )
}

