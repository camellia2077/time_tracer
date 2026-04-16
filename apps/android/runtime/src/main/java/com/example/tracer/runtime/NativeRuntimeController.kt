package com.example.tracer

import android.content.Context

class NativeRuntimeController(context: Context) : RuntimeGateway {
    // RuntimeGateway is the aggregate contract that composes all domain gateways.
    private val runtimeEnvironment = RuntimeEnvironment(context)
    private val inputRecordStore = InputRecordStore()
    private val runtimeSession = RuntimeSession(runtimeEnvironment, inputRecordStore)
    private val runtimeBridge = NativeRuntimeBridge()
    private val errorMapper = RuntimeErrorMapper()
    private val operationIdGenerator = RuntimeOperationIdGenerator()
    private val diagnosticsRecorder = RuntimeDiagnosticsRecorder(
        runtimePathsProvider = runtimeSession::runtimePathsOrNull
    )
    private val responseCodec = NativeResponseCodec()
    private val atomicRecordCodec = NativeAtomicRecordCodec()
    private val ingestSyncStatusCodec = NativeIngestSyncStatusCodec()
    private val txtRuntimeCodec = NativeTxtRuntimeCodec()
    private val reportTranslator = NativeReportTranslator(responseCodec)
    private val queryTranslator = NativeQueryTranslator(responseCodec)
    private val recordTranslator = NativeRecordTranslator(responseCodec)
    private val coreAdapter = RuntimeCoreAdapter(
        ensureRuntimePaths = runtimeSession::ensureRuntimePaths,
        runtimePathsProvider = runtimeSession::runtimePathsOrNull,
        nativeInit = runtimeBridge::nativeInit,
        nativeQuery = runtimeBridge::nativeQuery,
        nativeTree = runtimeBridge::nativeTree,
        responseCodec = responseCodec,
        reportTranslator = reportTranslator,
        diagnosticsRecorder = diagnosticsRecorder,
        nextOperationId = operationIdGenerator::next,
        errorMapper = errorMapper
    )
    private val txtInspectionService = TxtInspectionService(
        ensureTextStorage = runtimeSession::ensureTextStorage,
        executeAfterInit = coreAdapter::executeAfterInit,
        nativeListTxtIngestSyncStatus = runtimeBridge::nativeListTxtIngestSyncStatus,
        responseCodec = responseCodec,
        statusCodec = ingestSyncStatusCodec
    )
    private val queryDelegate = RuntimeQueryDelegate(
        queryTranslator = queryTranslator,
        executeNativeTreeQuery = coreAdapter::executeNativeTreeQuery,
        executeNativeDataQuery = coreAdapter::executeNativeDataQuery
    )
    private val recordDelegate = RuntimeRecordDelegate(
        ensureRuntimePaths = runtimeSession::ensureRuntimePaths,
        ensureTextStorage = runtimeSession::ensureTextStorage,
        rawRecordStore = inputRecordStore,
        loadWakeKeywords = queryDelegate::listWakeKeywords,
        responseCodec = responseCodec,
        atomicRecordCodec = atomicRecordCodec,
        recordTranslator = recordTranslator,
        inspectTxtFilesInternal = txtInspectionService::inspectTxtFiles,
        executeAfterInit = coreAdapter::executeAfterInit,
        nativeValidateStructure = runtimeBridge::nativeValidateStructure,
        nativeValidateLogic = runtimeBridge::nativeValidateLogic,
        nativeRecordActivityAtomically = runtimeBridge::nativeRecordActivityAtomically,
        nativeIngestSingleTxtReplaceMonth = runtimeBridge::nativeIngestSingleTxtReplaceMonth,
        nativeClearTxtIngestSyncStatus = runtimeBridge::nativeClearTxtIngestSyncStatus
    )
    private val storageDelegate = RuntimeStorageDelegate(
        ensureTextStorage = runtimeSession::ensureTextStorage,
        ensureConfigTomlStorage = runtimeSession::ensureConfigTomlStorage,
        inspectTxtFilesInternal = txtInspectionService::inspectTxtFiles
    )
    private val reportDelegate = RuntimeReportDelegate(
        executeReportAfterInit = coreAdapter::executeReportAfterInit,
        nativeReportJson = runtimeBridge::nativeReportJson
    )

    private val initService = RuntimeInitService(
        initializeRuntimeInternal = coreAdapter::initializeRuntimeInternal,
        clearRuntimeData = runtimeEnvironment::clearRuntimeData,
        clearDatabaseData = runtimeEnvironment::clearDatabaseData,
        resetRuntimeCaches = runtimeSession::reset
    )
    private val ingestService = RuntimeIngestService(
        ensureRuntimePaths = runtimeSession::ensureRuntimePaths,
        executeAfterInit = coreAdapter::executeAfterInit,
        nativeValidateStructure = runtimeBridge::nativeValidateStructure,
        nativeValidateLogic = runtimeBridge::nativeValidateLogic,
        nativeIngestSingleTxtReplaceMonth = runtimeBridge::nativeIngestSingleTxtReplaceMonth
    )
    private val recordService = RuntimeRecordService(
        recordDelegate = recordDelegate,
        storageDelegate = storageDelegate,
        clearTxtData = {
            runtimeSession.ensureRuntimePaths()
            runtimeEnvironment.clearTxtData()
        },
        clearTxtIngestSyncStatus = {
            coreAdapter.executeAfterInit("native_clear_txt_ingest_sync_status") {
                runtimeBridge.nativeClearTxtIngestSyncStatus()
            }
        }
    )
    private val queryService = RuntimeQueryService(queryDelegate)
    private val reportService = RuntimeReportService(reportDelegate)
    private val configService = RuntimeConfigService(storageDelegate)
    private val diagnosticsService = RuntimeDiagnosticsService(
        ensureRuntimePaths = runtimeSession::ensureRuntimePaths,
        bundleStatusProvider = runtimeEnvironment::lastConfigBundleStatus,
        diagnosticsRecorder = diagnosticsRecorder,
        nextOperationId = operationIdGenerator::next,
        errorMapper = errorMapper
    )
    private val cryptoService = RuntimeCryptoService(
        responseCodec = responseCodec,
        nativeEncryptFile = runtimeBridge::nativeEncryptFile,
        nativeDecryptFile = runtimeBridge::nativeDecryptFile,
        setProgressListener = runtimeBridge::setCryptoProgressListener
    )
    private val tracerExchangeService = RuntimeTracerExchangeService(
        responseCodec = responseCodec,
        nativeExportTracerExchange = runtimeBridge::nativeExportTracerExchange,
        nativeExportTracerExchangeFromPayloadJson = runtimeBridge::nativeExportTracerExchangeFromPayloadJson,
        nativeImportTracerExchange = runtimeBridge::nativeImportTracerExchange,
        nativeInspectTracerExchange = runtimeBridge::nativeInspectTracerExchange,
        setProgressListener = runtimeBridge::setCryptoProgressListener
    )
    private val txtDayBlockService = RuntimeTxtDayBlockService(
        initializeRuntimeInternal = coreAdapter::initializeRuntimeInternal,
        nativeTxt = runtimeBridge::nativeTxt,
        codec = txtRuntimeCodec
    )

    // init
    override suspend fun initializeRuntime(): NativeCallResult =
        initService.initializeRuntime()

    override suspend fun clearAndReinitialize(): ClearAndInitResult =
        initService.clearAndReinitialize()
    override suspend fun clearDatabase(): ClearDatabaseResult =
        initService.clearDatabase()


    // ingest
    override suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult =
        ingestService.ingestSingleTxtReplaceMonth(inputPath)

    // record
    override suspend fun createCurrentMonthTxt(): RecordActionResult =
        recordService.createCurrentMonthTxt()

    override suspend fun createMonthTxt(month: String): RecordActionResult =
        recordService.createMonthTxt(month)

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?,
        timeOrderMode: RecordTimeOrderMode
    ): RecordActionResult =
        recordService.recordNow(activityName, remark, targetDateIso, preferredTxtPath, timeOrderMode)

    override suspend fun syncLiveToDatabase(): NativeCallResult =
        recordService.syncLiveToDatabase()

    override suspend fun listTxtFiles(): TxtHistoryListResult =
        recordService.listTxtFiles()

    override suspend fun inspectTxtFiles(): TxtInspectionResult =
        recordService.inspectTxtFiles()

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        recordService.readTxtFile(relativePath)

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        recordService.saveTxtFileAndSync(relativePath, content)

    override suspend fun defaultTxtDayMarker(
        selectedMonth: String,
        targetDateIso: String
    ): TxtDayMarkerResult = txtDayBlockService.defaultTxtDayMarker(
        selectedMonth = selectedMonth,
        targetDateIso = targetDateIso
    )

    override suspend fun resolveTxtDayBlock(
        content: String,
        dayMarker: String,
        selectedMonth: String
    ): TxtDayBlockResolveResult = txtDayBlockService.resolveTxtDayBlock(
        content = content,
        dayMarker = dayMarker,
        selectedMonth = selectedMonth
    )

    override suspend fun replaceTxtDayBlock(
        content: String,
        dayMarker: String,
        editedDayBody: String
    ): TxtDayBlockReplaceResult = txtDayBlockService.replaceTxtDayBlock(
        content = content,
        dayMarker = dayMarker,
        editedDayBody = editedDayBody
    )

    override suspend fun clearTxt(): ClearTxtResult =
        recordService.clearTxt()

    // storage
    override suspend fun listConfigTomlFiles(): ConfigTomlListResult =
        configService.listConfigTomlFiles()

    override suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult =
        configService.readConfigTomlFile(relativePath)

    override suspend fun saveConfigTomlFile(relativePath: String, content: String): TxtFileContentResult =
        configService.saveConfigTomlFile(relativePath, content)

    // file crypto
    override suspend fun encryptTxtFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): RecordActionResult = cryptoService.encryptTxtFile(
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        securityLevel = securityLevel,
        onProgress = onProgress
    )

    override suspend fun decryptTracerFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): RecordActionResult = cryptoService.decryptTracerFile(
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        onProgress = onProgress
    )

    override suspend fun exportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeExportResult = tracerExchangeService.exportTracerExchange(
        inputPath = inputPath,
        outputPath = outputPath,
        passphrase = passphrase,
        securityLevel = securityLevel,
        dateCheckMode = dateCheckMode,
        onProgress = onProgress
    )

    override suspend fun exportTracerExchangeFromPayload(
        payloads: List<TracerExchangePayloadItem>,
        outputFd: Int,
        passphrase: String,
        securityLevel: FileCryptoSecurityLevel,
        dateCheckMode: Int,
        logicalSourceRootName: String,
        outputDisplayName: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeExportResult = tracerExchangeService.exportTracerExchangeFromPayload(
        payloads = payloads,
        outputFd = outputFd,
        passphrase = passphrase,
        securityLevel = securityLevel,
        dateCheckMode = dateCheckMode,
        logicalSourceRootName = logicalSourceRootName,
        outputDisplayName = outputDisplayName,
        onProgress = onProgress
    )

    override suspend fun importTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeImportResult = tracerExchangeService.importTracerExchange(
        inputPath = inputPath,
        workRoot = workRoot,
        passphrase = passphrase,
        onProgress = onProgress
    )

    override suspend fun inspectTracerExchange(
        inputPath: String,
        passphrase: String,
        onProgress: ((FileCryptoProgressEvent) -> Unit)?
    ): TracerExchangeInspectResult = tracerExchangeService.inspectTracerExchange(
        inputPath = inputPath,
        passphrase = passphrase,
        onProgress = onProgress
    )

    override suspend fun listRecentDiagnostics(limit: Int): RuntimeDiagnosticsListResult =
        diagnosticsService.listRecentDiagnostics(limit)

    override suspend fun buildDiagnosticsPayload(maxEntries: Int): RuntimeDiagnosticsPayloadResult =
        diagnosticsService.buildDiagnosticsPayload(maxEntries)

    // report
    override suspend fun reportMarkdown(request: TemporalReportQueryRequest): ReportCallResult =
        reportService.reportMarkdown(request)

    // query
    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult =
        queryService.queryActivitySuggestions(lookbackDays, topN)

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        queryService.queryDayDurations(params)

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        queryService.queryDayDurationStats(params)

    override suspend fun queryProjectTree(params: DataTreeQueryParams): TreeQueryResult =
        queryService.queryProjectTree(params)

    override suspend fun queryProjectTreeText(params: DataTreeQueryParams): DataQueryTextResult =
        queryService.queryProjectTreeText(params)

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        queryService.queryReportChart(params)

    override suspend fun queryReportComposition(
        params: ReportCompositionQueryParams
    ): ReportCompositionQueryResult =
        queryService.queryReportComposition(params)

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        queryService.listActivityMappingNames()

    override suspend fun listActivityAliasKeys(): ActivityMappingNamesResult =
        queryService.listActivityAliasKeys()

    override suspend fun listWakeKeywords(): ActivityMappingNamesResult =
        queryService.listWakeKeywords()

    override suspend fun listAuthorableEventTokens(): ActivityMappingNamesResult =
        queryService.listAuthorableEventTokens()
}
