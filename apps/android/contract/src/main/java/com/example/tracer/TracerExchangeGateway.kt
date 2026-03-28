package com.example.tracer

interface TracerExchangeGateway {
    suspend fun exportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        dateCheckMode: Int = 0,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeExportResult = TracerExchangeExportResult(
        ok = false,
        message = "Complete exchange package export is not supported by current runtime.",
        outputPath = "",
        sourceRootName = "",
        payloadFileCount = 0,
        converterFileCount = 0,
        manifestIncluded = false
    )

    suspend fun exportTracerExchangeFromPayload(
        payloads: List<TracerExchangePayloadItem>,
        outputFd: Int,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        dateCheckMode: Int = 0,
        logicalSourceRootName: String = "data",
        outputDisplayName: String = "data.tracer",
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeExportResult = TracerExchangeExportResult(
        ok = false,
        message = "Complete exchange package export is not supported by current runtime.",
        outputPath = "",
        sourceRootName = "",
        payloadFileCount = 0,
        converterFileCount = 0,
        manifestIncluded = false
    )

    suspend fun importTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeImportResult = TracerExchangeImportResult(
        ok = false,
        message = "Import tracer exchange is not supported by current runtime.",
        sourceRootName = "",
        payloadFileCount = 0
    )

    suspend fun inspectTracerExchange(
        inputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeInspectResult = TracerExchangeInspectResult(
        ok = false,
        message = "Inspect tracer exchange is not supported by current runtime.",
        renderedText = "",
        inputPath = "",
        sourceRootName = "",
        payloadFileCount = 0,
        producerPlatform = "",
        producerApp = "",
        createdAtUtc = ""
    )
}
