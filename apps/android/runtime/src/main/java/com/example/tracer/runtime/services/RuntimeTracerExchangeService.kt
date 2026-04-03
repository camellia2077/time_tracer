package com.example.tracer

internal class RuntimeTracerExchangeService(
    private val responseCodec: NativeResponseCodec,
    private val nativeExportTracerExchange: (
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int
    ) -> String,
    private val nativeExportTracerExchangeFromPayloadJson: (
        requestJson: String,
        outputFd: Int
    ) -> String,
    private val nativeImportTracerExchange: (
        inputPath: String,
        workRoot: String,
        passphrase: String
    ) -> String,
    private val nativeInspectTracerExchange: (inputPath: String, passphrase: String) -> String,
    private val setProgressListener: (((String) -> Unit)?) -> Unit = NativeBridge::setCryptoProgressListener
) {
    private val exportService = RuntimeTracerExchangeExportService(
        responseCodec = responseCodec,
        nativeExportTracerExchange = nativeExportTracerExchange,
        nativeExportTracerExchangeFromPayloadJson = nativeExportTracerExchangeFromPayloadJson,
        setProgressListener = setProgressListener
    )
    private val importService = RuntimeTracerExchangeImportService(
        responseCodec = responseCodec,
        nativeImportTracerExchange = nativeImportTracerExchange,
        setProgressListener = setProgressListener
    )
    private val inspectService = RuntimeTracerExchangeInspectService(
        responseCodec = responseCodec,
        nativeInspectTracerExchange = nativeInspectTracerExchange,
        setProgressListener = setProgressListener
    )

    suspend fun exportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        dateCheckMode: Int = NativeBridge.DATE_CHECK_NONE,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeExportResult = exportService.exportTracerExchange(
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        securityLevel = securityLevel,
        dateCheckMode = dateCheckMode,
        onProgress = onProgress
    )

    suspend fun exportTracerExchangeFromPayload(
        payloads: List<TracerExchangePayloadItem>,
        outputFd: Int,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel = FileCryptoSecurityLevel.INTERACTIVE,
        dateCheckMode: Int = NativeBridge.DATE_CHECK_NONE,
        logicalSourceRootName: String = "data",
        outputDisplayName: String = "data.tracer",
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeExportResult = exportService.exportTracerExchangeFromPayload(
        payloads = payloads,
        outputFd = outputFd,
        passphrase = passphrase,
        securityLevel = securityLevel,
        dateCheckMode = dateCheckMode,
        logicalSourceRootName = logicalSourceRootName,
        outputDisplayName = outputDisplayName,
        onProgress = onProgress
    )

    suspend fun importTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeImportResult = importService.importTracerExchange(
        inputPath = inputPath,
        workRoot = workRoot,
        passphrase = passphrase,
        onProgress = onProgress
    )

    suspend fun inspectTracerExchange(
        inputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)? = null
    ): TracerExchangeInspectResult = inspectService.inspectTracerExchange(
        inputPath = inputPath,
        passphrase = passphrase,
        onProgress = onProgress
    )
}
