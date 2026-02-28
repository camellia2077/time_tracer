package com.example.tracer

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.time.Instant
import java.time.ZoneOffset
import java.time.format.DateTimeFormatter

class NativeRuntimeController(context: Context) : RuntimeGateway {
    // RuntimeGateway is the aggregate contract that composes all domain gateways.
    private var runtimePaths: RuntimePaths? = null
    private var textStorage: TextStorage? = null
    private var configTomlStorage: ConfigTomlStorage? = null
    private val runtimeEnvironment = RuntimeEnvironment(context)
    private val operationIdGenerator = RuntimeOperationIdGenerator()
    private val diagnosticsRecorder = RuntimeDiagnosticsRecorder(
        runtimePathsProvider = { runtimePaths }
    )
    private val responseCodec = NativeResponseCodec()
    private val reportTranslator = NativeReportTranslator(responseCodec)
    private val queryTranslator = NativeQueryTranslator(responseCodec)
    private val recordTranslator = NativeRecordTranslator(responseCodec)
    private val callExecutor = NativeCallExecutor(
        initializeRuntime = { initializeRuntimeInternal() },
        runtimePathsProvider = { runtimePaths },
        responseCodec = responseCodec,
        reportTranslator = reportTranslator,
        diagnosticsRecorder = diagnosticsRecorder,
        nextOperationId = operationIdGenerator::next,
        formatFailure = ::formatNativeFailure
    )
    private val rawRecordStore = LiveRawRecordStore()
    private val syncUseCase = IngestSyncUseCase(AutoSyncMaterializer())

    private val recordDelegate = RuntimeRecordDelegate(
        ensureRuntimePaths = ::ensureRuntimePaths,
        ensureTextStorage = ::ensureTextStorage,
        rawRecordStore = rawRecordStore,
        responseCodec = responseCodec,
        recordTranslator = recordTranslator,
        executeAfterInit = ::executeAfterInit,
        syncLiveOperation = { paths -> syncUseCase.run(paths) }
    )
    private val storageDelegate = RuntimeStorageDelegate(
        ensureTextStorage = ::ensureTextStorage,
        ensureConfigTomlStorage = ::ensureConfigTomlStorage
    )
    private val reportDelegate = RuntimeReportDelegate(
        executeReportAfterInit = ::executeReportAfterInit
    )
    private val queryDelegate = RuntimeQueryDelegate(
        queryTranslator = queryTranslator,
        executeNativeTreeQuery = ::executeNativeTreeQuery,
        executeNativeDataQuery = ::executeNativeDataQuery
    )

    private val initService = RuntimeInitService(
        initializeRuntimeInternal = ::initializeRuntimeInternal,
        clearRuntimeData = runtimeEnvironment::clearRuntimeData,
        resetRuntimeCaches = ::resetRuntimeCaches
    )
    private val ingestService = RuntimeIngestService(
        ensureRuntimePaths = ::ensureRuntimePaths,
        executeAfterInit = ::executeAfterInit,
        nativeIngest = { inputPath, dateCheckMode, saveProcessedOutput ->
            NativeBridge.nativeIngest(
                inputPath = inputPath,
                dateCheckMode = dateCheckMode,
                saveProcessedOutput = saveProcessedOutput
            )
        },
        nativeIngestSingleTxtReplaceMonth = { inputPath, dateCheckMode, saveProcessedOutput ->
            NativeBridge.nativeIngestSingleTxtReplaceMonth(
                inputPath = inputPath,
                dateCheckMode = dateCheckMode,
                saveProcessedOutput = saveProcessedOutput
            )
        }
    )
    private val recordService = RuntimeRecordService(
        recordDelegate = recordDelegate,
        storageDelegate = storageDelegate,
        clearTxtData = {
            ensureRuntimePaths()
            runtimeEnvironment.clearTxtData()
        }
    )
    private val queryService = RuntimeQueryService(queryDelegate)
    private val reportService = RuntimeReportService(reportDelegate)
    private val configService = RuntimeConfigService(storageDelegate)
    private val cryptoService = RuntimeCryptoService(
        responseCodec = responseCodec,
        nativeEncryptFile = { inputPath, outputPath, passphrase, securityLevel ->
            NativeBridge.nativeEncryptFile(
                inputPath = inputPath,
                outputPath = outputPath,
                passphrase = passphrase,
                securityLevel = securityLevel.wireValue
            )
        },
        nativeDecryptFile = { inputPath, outputPath, passphrase ->
            NativeBridge.nativeDecryptFile(
                inputPath = inputPath,
                outputPath = outputPath,
                passphrase = passphrase
            )
        }
    )

    // init
    override suspend fun initializeRuntime(): NativeCallResult =
        initService.initializeRuntime()

    override suspend fun clearAndReinitialize(): ClearAndInitResult =
        initService.clearAndReinitialize()

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

    override suspend fun listRecentDiagnostics(limit: Int): RuntimeDiagnosticsListResult =
        withContext(Dispatchers.IO) {
            if (limit <= 0) {
                return@withContext RuntimeDiagnosticsListResult(
                    ok = false,
                    entries = emptyList(),
                    message = "diagnostics limit must be greater than 0.",
                    diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
                )
            }
            val records = diagnosticsRecorder.recent(limit)
            RuntimeDiagnosticsListResult(
                ok = true,
                entries = records.map { it.toContractEntry() },
                message = "Loaded ${records.size} diagnostic record(s).",
                diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
            )
        }

    override suspend fun buildDiagnosticsPayload(maxEntries: Int): RuntimeDiagnosticsPayloadResult =
        withContext(Dispatchers.IO) {
            if (maxEntries <= 0) {
                return@withContext RuntimeDiagnosticsPayloadResult(
                    ok = false,
                    payload = "",
                    message = "maxEntries must be greater than 0.",
                    entryCount = 0,
                    diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
                )
            }

            runCatching {
                ensureRuntimePaths()
            }.onFailure { error ->
                val asException = if (error is Exception) {
                    error
                } else {
                    IllegalStateException(error.message ?: "unknown failure", error)
                }
                val operationId = operationIdGenerator.next("diagnostics_prepare_paths")
                diagnosticsRecorder.record(
                    RuntimeDiagnosticRecord(
                        timestampEpochMs = System.currentTimeMillis(),
                        operationId = operationId,
                        stage = "diagnostics.prepare_runtime_paths",
                        ok = false,
                        initialized = null,
                        message = appendFailureContext(
                            message = formatNativeFailure("prepare runtime paths failed", asException),
                            operationId = operationId
                        ),
                        errorLogPath = ""
                    )
                )
            }

            val records = diagnosticsRecorder.recent(maxEntries)
            val bundleStatus = runtimeEnvironment.lastConfigBundleStatus()
            val payload = buildDiagnosticsPayloadText(
                records = records,
                bundleStatus = bundleStatus
            )
            RuntimeDiagnosticsPayloadResult(
                ok = true,
                payload = payload,
                message = "Prepared diagnostics payload (${records.size} entries).",
                entryCount = records.size,
                diagnosticsLogPath = diagnosticsRecorder.diagnosticsLogPath()
            )
        }

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

    private fun initializeRuntimeInternal(): NativeCallResult {
        val operationId = operationIdGenerator.next("native_init")
        try {
            val paths = ensureRuntimePaths()
            val response = NativeBridge.nativeInit(
                dbPath = paths.dbPath,
                outputRoot = paths.outputRoot,
                converterConfigTomlPath = paths.configTomlPath
            )
            val payload = responseCodec.parse(response)
            val errorLogPath = runCatching {
                org.json.JSONObject(payload.content).optString("error_log_path", "")
            }.getOrDefault("")
            val result = NativeCallResult(
                initialized = payload.ok,
                operationOk = payload.ok,
                rawResponse = response,
                errorLogPath = errorLogPath,
                operationId = operationId
            )
            val message = if (payload.ok) {
                "ok"
            } else {
                appendFailureContext(
                    message = payload.errorMessage.ifEmpty { "native init failed." },
                    operationId = operationId,
                    errorLogPath = errorLogPath
                )
            }
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "native_init",
                    ok = payload.ok,
                    initialized = payload.ok,
                    message = message,
                    errorLogPath = errorLogPath
                )
            )
            return result
        } catch (error: Exception) {
            val failureMessage = formatNativeFailure("nativeInit failed", error)
            diagnosticsRecorder.record(
                RuntimeDiagnosticRecord(
                    timestampEpochMs = System.currentTimeMillis(),
                    operationId = operationId,
                    stage = "native_init",
                    ok = false,
                    initialized = false,
                    message = appendFailureContext(failureMessage, operationId = operationId),
                    errorLogPath = ""
                )
            )
            throw error
        }
    }

    private fun ensureRuntimePaths(): RuntimePaths {
        return ensureRuntimePathsCached(
            existing = runtimePaths,
            runtimeEnvironment = runtimeEnvironment,
            onPrepared = { runtimePaths = it }
        )
    }

    private fun ensureTextStorage(): TextStorage {
        return ensureTextStorageCached(
            existing = textStorage,
            runtimePathsProvider = { ensureRuntimePaths() },
            rawRecordStore = rawRecordStore,
            onPrepared = { textStorage = it }
        )
    }

    private fun ensureConfigTomlStorage(): ConfigTomlStorage {
        return ensureConfigTomlStorageCached(
            existing = configTomlStorage,
            runtimePathsProvider = { ensureRuntimePaths() },
            onPrepared = { configTomlStorage = it }
        )
    }

    private fun executeAfterInit(
        operationName: String,
        action: (RuntimePaths) -> String
    ): NativeCallResult {
        return callExecutor.executeAfterInit(
            operationName = operationName,
            action = action
        )
    }

    private fun executeReportAfterInit(
        operationName: String,
        action: (RuntimePaths) -> String
    ): ReportCallResult {
        return callExecutor.executeReportAfterInit(
            operationName = operationName,
            action = action
        )
    }

    private fun executeNativeDataQuery(
        request: DataQueryRequest,
        onRuntimePaths: ((RuntimePaths) -> Unit)? = null
    ): NativeCallResult {
        val operationName = "native_query_${request.action}"
        return executeAfterInit(operationName = operationName) { paths ->
            onRuntimePaths?.invoke(paths)
            NativeBridge.nativeQuery(
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
        }
    }

    private fun executeNativeTreeQuery(
        params: DataTreeQueryParams
    ): NativeCallResult {
        return executeAfterInit(operationName = "native_tree") {
            NativeBridge.nativeTree(
                listRoots = false,
                rootPattern = "",
                maxDepth = params.level,
                period = params.period.wireValue,
                periodArgument = params.periodArgument,
                root = ""
            )
        }
    }

    private fun resetRuntimeCaches() {
        resetRuntimeStorageCaches(
            onRuntimePathsReset = { runtimePaths = null },
            onTextStorageReset = { textStorage = null },
            onConfigTomlStorageReset = { configTomlStorage = null }
        )
    }

    private fun buildDiagnosticsPayloadText(
        records: List<RuntimeDiagnosticRecord>,
        bundleStatus: RuntimeConfigBundleStatus
    ): String {
        val generatedAtIso = DateTimeFormatter.ISO_INSTANT
            .withZone(ZoneOffset.UTC)
            .format(Instant.now())
        val bundleSchema = bundleStatus.schemaVersion?.toString() ?: "unknown"
        val bundleProfile = bundleStatus.profile.ifBlank { "unknown" }
        val bundleName = bundleStatus.bundleName.ifBlank { "unknown" }
        val bundleMissing = if (bundleStatus.missingFiles.isEmpty()) {
            "-"
        } else {
            bundleStatus.missingFiles.joinToString(",")
        }
        val logPath = diagnosticsRecorder.diagnosticsLogPath().ifBlank { "-" }

        return buildString {
            appendLine("time_tracer_android_support_payload_v1")
            appendLine("generated_at_utc=$generatedAtIso")
            appendLine("bundle.ok=${bundleStatus.ok}")
            appendLine("bundle.schema_version=$bundleSchema")
            appendLine("bundle.profile=$bundleProfile")
            appendLine("bundle.name=$bundleName")
            appendLine("bundle.required_count=${bundleStatus.requiredFiles.size}")
            appendLine("bundle.missing=$bundleMissing")
            appendLine("bundle.message=${bundleStatus.message}")
            appendLine("diagnostics.log_path=$logPath")
            appendLine("diagnostics.count=${records.size}")
            appendLine("[diagnostics]")
            for (record in records) {
                val timestampIso = DateTimeFormatter.ISO_INSTANT
                    .withZone(ZoneOffset.UTC)
                    .format(Instant.ofEpochMilli(record.timestampEpochMs))
                val initFlag = record.initialized?.toString() ?: "null"
                val logSuffix = if (record.errorLogPath.isBlank()) {
                    ""
                } else {
                    " log=${record.errorLogPath}"
                }
                appendLine(
                    "$timestampIso op=${record.operationId} stage=${record.stage} ok=${record.ok} " +
                        "initialized=$initFlag$logSuffix msg=${record.message}"
                )
            }
        }
    }
}
