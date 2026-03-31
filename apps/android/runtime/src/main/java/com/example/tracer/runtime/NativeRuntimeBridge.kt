package com.example.tracer

/**
 * Thin Kotlin-side adapter over `NativeBridge`.
 *
 * Android runtime flows should depend on this adapter instead of calling raw
 * `external fun` JNI methods directly.
 */
internal class NativeRuntimeBridge {
    fun nativeInit(paths: RuntimePaths): String =
        NativeBridge.nativeInit(
            dbPath = paths.dbPath,
            outputRoot = paths.outputRoot,
            converterConfigTomlPath = paths.configTomlPath
        )

    fun nativeIngest(
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ): String = NativeBridge.nativeIngest(
        inputPath = inputPath,
        dateCheckMode = dateCheckMode,
        saveProcessedOutput = saveProcessedOutput
    )

    fun nativeIngestSingleTxtReplaceMonth(
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ): String = NativeBridge.nativeIngestSingleTxtReplaceMonth(
        inputPath = inputPath,
        dateCheckMode = dateCheckMode,
        saveProcessedOutput = saveProcessedOutput
    )

    fun nativeListTxtIngestSyncStatus(requestJson: String): String =
        NativeBridge.nativeListTxtIngestSyncStatus(requestJson)

    fun nativeClearTxtIngestSyncStatus(): String =
        NativeBridge.nativeClearTxtIngestSyncStatus()

    fun nativeEncryptFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel
    ): String = NativeBridge.nativeEncryptFile(
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        securityLevel = securityLevel.wireValue
    )

    fun nativeDecryptFile(
        inputPath: String,
        outputPath: String,
        passphrase: String
    ): String = NativeBridge.nativeDecryptFile(
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase
    )

    fun nativeExportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int
    ): String = NativeBridge.nativeExportTracerExchange(
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        securityLevel = securityLevel.wireValue,
        dateCheckMode = dateCheckMode
    )

    fun nativeExportTracerExchangeFromPayloadJson(
        requestJson: String,
        outputFd: Int
    ): String = NativeBridge.nativeExportTracerExchangeFromPayloadJson(
        requestJson = requestJson,
        outputFd = outputFd
    )

    fun nativeImportTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String
    ): String = NativeBridge.nativeImportTracerExchange(
        inputPath = inputPath,
        workRoot = workRoot,
        passphrase = passphrase
    )

    fun nativeInspectTracerExchange(
        inputPath: String,
        passphrase: String
    ): String = NativeBridge.nativeInspectTracerExchange(
        inputPath = inputPath,
        passphrase = passphrase
    )

    fun nativeValidateStructure(inputPath: String): String =
        NativeBridge.nativeValidateStructure(
            inputPath = inputPath
        )

    fun nativeValidateLogic(
        inputPath: String,
        dateCheckMode: Int
    ): String = NativeBridge.nativeValidateLogic(
        inputPath = inputPath,
        dateCheckMode = dateCheckMode
    )

    fun nativeRecordActivityAtomically(
        targetDateIso: String,
        rawActivityName: String,
        remark: String,
        preferredTxtPath: String?,
        dateCheckMode: Int,
        timeOrderMode: RecordTimeOrderMode
    ): String = NativeBridge.nativeRecordActivityAtomically(
        targetDateIso = targetDateIso,
        rawActivityName = rawActivityName,
        remark = remark,
        preferredTxtPath = preferredTxtPath.orEmpty(),
        dateCheckMode = dateCheckMode,
        timeOrderMode = when (timeOrderMode) {
            RecordTimeOrderMode.STRICT_CALENDAR ->
                NativeBridge.RECORD_TIME_ORDER_STRICT_CALENDAR
            RecordTimeOrderMode.LOGICAL_DAY_0600 ->
                NativeBridge.RECORD_TIME_ORDER_LOGICAL_DAY_0600
        }
    )

    fun nativeQuery(request: DataQueryRequest): String = NativeBridge.nativeQuery(
        action = request.action,
        year = request.year ?: 0,
        month = request.month ?: 0,
        fromDate = request.fromDateIso.orEmpty(),
        toDate = request.toDateIso.orEmpty(),
        remark = request.remark.orEmpty(),
        dayRemark = request.dayRemark.orEmpty(),
        project = request.project.orEmpty(),
        root = request.root.orEmpty(),
        exercise = request.exercise ?: NativeBridge.UNSET_INT,
        status = request.status ?: NativeBridge.UNSET_INT,
        overnight = request.overnight,
        reverse = request.reverse,
        limit = request.limit ?: NativeBridge.UNSET_INT,
        topN = request.topN ?: NativeBridge.UNSET_INT,
        lookbackDays = request.lookbackDays ?: NativeBridge.UNSET_INT,
        scoreByDuration = request.scoreByDuration,
        treePeriod = request.treePeriod.orEmpty(),
        treePeriodArgument = request.treePeriodArgument.orEmpty(),
        treeMaxDepth = request.treeMaxDepth ?: NativeBridge.UNSET_INT,
        outputMode = request.outputMode.orEmpty()
    )

    fun nativeTree(params: DataTreeQueryParams): String = NativeBridge.nativeTree(
        listRoots = false,
        rootPattern = "",
        maxDepth = params.level,
        period = params.period.wireValue,
        periodArgument = params.periodArgument,
        root = ""
    )

    fun nativeReportSingle(
        reportType: Int,
        argument: String,
        format: Int = NativeBridge.REPORT_FORMAT_MARKDOWN
    ): String = NativeBridge.nativeReport(
        mode = NativeBridge.REPORT_MODE_SINGLE,
        reportType = reportType,
        argument = argument,
        format = format,
        daysList = null
    )

    fun setCryptoProgressListener(listener: ((String) -> Unit)?) {
        NativeBridge.setCryptoProgressListener(listener)
    }
}
