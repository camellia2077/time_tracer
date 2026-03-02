package com.example.tracer

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
}
