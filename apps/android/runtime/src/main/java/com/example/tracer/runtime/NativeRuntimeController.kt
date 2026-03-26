package com.example.tracer

import android.content.Context

class NativeRuntimeController(context: Context) : RuntimeGateway {
    // RuntimeGateway is the aggregate contract that composes all domain gateways.
    private val runtimeEnvironment = RuntimeEnvironment(context)
    private val rawRecordStore = LiveRawRecordStore()
    private val runtimeSession = RuntimeSession(runtimeEnvironment, rawRecordStore)
    private val runtimeBridge = NativeRuntimeBridge()
    private val errorMapper = RuntimeErrorMapper()
    private val operationIdGenerator = RuntimeOperationIdGenerator()
    private val diagnosticsRecorder = RuntimeDiagnosticsRecorder(
        runtimePathsProvider = runtimeSession::runtimePathsOrNull
    )
    private val responseCodec = NativeResponseCodec()
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
    private val syncUseCase = IngestSyncUseCase(
        autoSyncMaterializer = AutoSyncMaterializer(),
        nativeIngest = runtimeBridge::nativeIngest
    )

    private val recordDelegate = RuntimeRecordDelegate(
        ensureRuntimePaths = runtimeSession::ensureRuntimePaths,
        ensureTextStorage = runtimeSession::ensureTextStorage,
        rawRecordStore = rawRecordStore,
        responseCodec = responseCodec,
        recordTranslator = recordTranslator,
        executeAfterInit = coreAdapter::executeAfterInit,
        nativeValidateStructure = runtimeBridge::nativeValidateStructure,
        nativeValidateLogic = runtimeBridge::nativeValidateLogic,
        nativeIngest = runtimeBridge::nativeIngest,
        syncLiveOperation = { paths -> syncUseCase.run(paths) }
    )
    private val storageDelegate = RuntimeStorageDelegate(
        ensureTextStorage = runtimeSession::ensureTextStorage,
        ensureConfigTomlStorage = runtimeSession::ensureConfigTomlStorage
    )
    private val reportDelegate = RuntimeReportDelegate(
        executeReportAfterInit = coreAdapter::executeReportAfterInit,
        nativeReportSingle = runtimeBridge::nativeReportSingle
    )
    private val queryDelegate = RuntimeQueryDelegate(
        queryTranslator = queryTranslator,
        executeNativeTreeQuery = coreAdapter::executeNativeTreeQuery,
        executeNativeDataQuery = coreAdapter::executeNativeDataQuery
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
        nativeIngest = runtimeBridge::nativeIngest,
        nativeIngestSingleTxtReplaceMonth = runtimeBridge::nativeIngestSingleTxtReplaceMonth
    )
    private val recordService = RuntimeRecordService(
        recordDelegate = recordDelegate,
        storageDelegate = storageDelegate,
        clearTxtData = {
            runtimeSession.ensureRuntimePaths()
            runtimeEnvironment.clearTxtData()
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
        nativeImportTracerExchange = runtimeBridge::nativeImportTracerExchange,
        nativeInspectTracerExchange = runtimeBridge::nativeInspectTracerExchange,
        setProgressListener = runtimeBridge::setCryptoProgressListener
    )

    // init
    override suspend fun initializeRuntime(): NativeCallResult =
        initService.initializeRuntime()

    override suspend fun clearAndReinitialize(): ClearAndInitResult =
        initService.clearAndReinitialize()
    override suspend fun clearDatabase(): ClearDatabaseResult =
        initService.clearDatabase()


    // ingest
    override suspend fun ingestFull(): NativeCallResult =
        ingestService.ingestFull()

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
        preferredTxtPath: String?
    ): RecordActionResult =
        recordService.recordNow(activityName, remark, targetDateIso, preferredTxtPath)

    override suspend fun syncLiveToDatabase(): NativeCallResult =
        recordService.syncLiveToDatabase()

    override suspend fun listTxtFiles(): TxtHistoryListResult =
        recordService.listTxtFiles()

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        recordService.readTxtFile(relativePath)

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        recordService.saveTxtFileAndSync(relativePath, content)

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
    override suspend fun reportDayMarkdown(date: String): ReportCallResult =
        reportService.reportDayMarkdown(date)

    override suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        reportService.reportMonthMarkdown(month)

    override suspend fun reportYearMarkdown(year: String): ReportCallResult =
        reportService.reportYearMarkdown(year)

    override suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        reportService.reportWeekMarkdown(week)

    override suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        reportService.reportRecentMarkdown(days)

    override suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        reportService.reportRange(startDate, endDate)

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

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        queryService.listActivityMappingNames()
}
