package com.example.tracer

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File
import java.util.Locale

class NativeRuntimeController(context: Context) : RuntimeGateway {
    // RuntimeGateway is the aggregate contract that composes all domain gateways.
    private var runtimePaths: RuntimePaths? = null
    private var textStorage: TextStorage? = null
    private var configTomlStorage: ConfigTomlStorage? = null
    private val runtimeEnvironment = RuntimeEnvironment(context)
    private val responseCodec = NativeResponseCodec()
    private val callExecutor = NativeCallExecutor(
        initializeRuntime = { initializeRuntimeInternal() },
        runtimePathsProvider = { runtimePaths },
        responseCodec = responseCodec,
        buildNativeErrorResponse = ::buildNativeErrorResponseJson,
        formatFailure = ::formatNativeFailure
    )
    private val rawRecordStore = LiveRawRecordStore()
    private val syncUseCase = IngestSyncUseCase(AutoSyncMaterializer())

    private val recordDelegate = RuntimeRecordDelegate(
        ensureRuntimePaths = ::ensureRuntimePaths,
        ensureTextStorage = ::ensureTextStorage,
        rawRecordStore = rawRecordStore,
        responseCodec = responseCodec,
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
        responseCodec = responseCodec,
        executeNativeDataQuery = ::executeNativeDataQuery
    )

    // init
    override suspend fun initializeRuntime(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            initializeRuntimeInternal()
        } catch (error: Exception) {
            buildNativeCallFailure(prefix = "nativeInit failed", error = error)
        }
    }

    // ingest
    override suspend fun ingestSmoke(): NativeCallResult =
        runIngestFlow { paths -> paths.smokeInputPath }

    override suspend fun ingestFull(): NativeCallResult =
        runIngestFlow { paths -> paths.fullInputPath }

    override suspend fun ingestSingleTxtReplaceMonth(inputPath: String): NativeCallResult =
        withContext(Dispatchers.IO) {
            try {
                val managedInputPath = prepareManagedSingleTxtInput(
                    paths = ensureRuntimePaths(),
                    sourceInputPath = inputPath
                )
                executeAfterInit {
                    NativeBridge.nativeIngestSingleTxtReplaceMonth(
                        inputPath = managedInputPath,
                        dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY,
                        // Android build disables processed JSON output in core
                        // (TT_ENABLE_PROCESSED_JSON_IO=OFF).
                        saveProcessedOutput = false
                    )
                }
            } catch (error: Exception) {
                buildNativeCallFailure(
                    prefix = "nativeIngest(single_txt_replace_month) failed",
                    error = error
                )
            }
        }

    // record
    override suspend fun createCurrentMonthTxt(): RecordActionResult =
        recordDelegate.createCurrentMonthTxt()

    override suspend fun createMonthTxt(month: String): RecordActionResult =
        recordDelegate.createMonthTxt(month)

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?,
        preferredTxtPath: String?
    ): RecordActionResult =
        recordDelegate.recordNow(activityName, remark, targetDateIso, preferredTxtPath)

    override suspend fun syncLiveToDatabase(): NativeCallResult =
        recordDelegate.syncLiveToDatabase()

    override suspend fun listTxtFiles(): TxtHistoryListResult =
        storageDelegate.listTxtFiles()

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult =
        storageDelegate.readTxtFile(relativePath)

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        recordDelegate.saveTxtFileAndSync(relativePath, content)

    // storage
    override suspend fun listConfigTomlFiles(): ConfigTomlListResult =
        storageDelegate.listConfigTomlFiles()

    override suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult =
        storageDelegate.readConfigTomlFile(relativePath)

    override suspend fun saveConfigTomlFile(relativePath: String, content: String): TxtFileContentResult =
        storageDelegate.saveConfigTomlFile(relativePath, content)

    // report
    override suspend fun reportDayMarkdown(date: String): ReportCallResult =
        reportDelegate.reportDayMarkdown(date)

    override suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        reportDelegate.reportMonthMarkdown(month)

    override suspend fun reportYearMarkdown(year: String): ReportCallResult =
        reportDelegate.reportYearMarkdown(year)

    override suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        reportDelegate.reportWeekMarkdown(week)

    override suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        reportDelegate.reportRecentMarkdown(days)

    override suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        reportDelegate.reportRange(startDate, endDate)

    // query
    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult =
        queryDelegate.queryActivitySuggestions(lookbackDays, topN)

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurations(params)

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        queryDelegate.queryDayDurationStats(params)

    override suspend fun queryProjectTree(params: DataTreeQueryParams): DataQueryTextResult =
        queryDelegate.queryProjectTree(params)

    override suspend fun queryReportChart(params: ReportChartQueryParams): ReportChartQueryResult =
        queryDelegate.queryReportChart(params)

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        queryDelegate.listActivityMappingNames()

    override suspend fun clearTxt(): ClearTxtResult = withContext(Dispatchers.IO) {
        try {
            ensureRuntimePaths()
            runtimeEnvironment.clearTxtData()
        } catch (error: Exception) {
            ClearTxtResult(
                ok = false,
                message = formatNativeFailure("clear txt failed", error)
            )
        }
    }

    override suspend fun clearAndReinitialize(): ClearAndInitResult = withContext(Dispatchers.IO) {
        try {
            val clearMessage = runtimeEnvironment.clearRuntimeData()
            resetRuntimeCaches()
            val initResult = initializeRuntimeInternal()
            ClearAndInitResult(
                initialized = initResult.initialized,
                operationOk = initResult.operationOk,
                clearMessage = clearMessage,
                initResponse = initResult.rawResponse
            )
        } catch (error: Exception) {
            ClearAndInitResult(
                initialized = false,
                operationOk = false,
                clearMessage = "clear -> failed",
                initResponse = buildNativeErrorResponseJson(
                    formatNativeFailure("clear and reinitialize failed", error)
                )
            )
        }
    }

    private fun initializeRuntimeInternal(): NativeCallResult {
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
        return NativeCallResult(
            initialized = payload.ok,
            operationOk = payload.ok,
            rawResponse = response,
            errorLogPath = errorLogPath
        )
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

    private fun executeAfterInit(action: (RuntimePaths) -> String): NativeCallResult {
        return callExecutor.executeAfterInit(action)
    }

    private fun executeReportAfterInit(action: (RuntimePaths) -> String): ReportCallResult {
        return callExecutor.executeReportAfterInit(action)
    }

    private suspend fun runIngestFlow(inputPathSelector: (RuntimePaths) -> String): NativeCallResult =
        withContext(Dispatchers.IO) {
            executeAfterInit { paths ->
                NativeBridge.nativeIngest(
                    inputPath = inputPathSelector(paths),
                    dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY,
                    // Android build disables processed JSON output in core
                    // (TT_ENABLE_PROCESSED_JSON_IO=OFF).
                    saveProcessedOutput = false
                )
            }
        }

    private data class ParsedMonthHeader(
        val year: Int,
        val month: Int
    ) {
        val monthKey: String
            get() = String.format(Locale.US, "%04d-%02d", year, month)
    }

    private fun prepareManagedSingleTxtInput(
        paths: RuntimePaths,
        sourceInputPath: String
    ): String {
        val sourceFile = File(sourceInputPath.trim())
        require(sourceFile.exists() && sourceFile.isFile) {
            "inputPath must point to an existing TXT file."
        }

        val normalizedContent = normalizeImportedText(sourceFile.readText())
        val monthHeader = parseRequiredMonthHeader(normalizedContent)
        val targetFile = File(paths.fullInputPath, "${monthHeader.monthKey}.txt")
        targetFile.parentFile?.mkdirs()
        targetFile.writeText(normalizedContent)
        return targetFile.absolutePath
    }

    private fun normalizeImportedText(rawContent: String): String {
        // Accept UTF-8 BOM imports and normalize line endings before core parse.
        return rawContent
            .removePrefix("\uFEFF")
            .replace("\r\n", "\n")
            .replace('\r', '\n')
    }

    private fun parseRequiredMonthHeader(content: String): ParsedMonthHeader {
        var yearValue: Int? = null
        var monthValue: Int? = null

        val lines = content.lines()
        for (line in lines) {
            val trimmed = line.trim()
            if (trimmed.isEmpty()) {
                continue
            }

            if (
                yearValue == null &&
                trimmed.length == 5 &&
                trimmed[0] == 'y' &&
                trimmed.substring(1).all(Char::isDigit)
            ) {
                yearValue = trimmed.substring(1).toInt()
                continue
            }

            if (
                yearValue != null &&
                monthValue == null &&
                trimmed.length == 3 &&
                trimmed[0] == 'm' &&
                trimmed.substring(1).all(Char::isDigit)
            ) {
                val parsedMonth = trimmed.substring(1).toInt()
                require(parsedMonth in 1..12) {
                    "Invalid month header: m${trimmed.substring(1)}."
                }
                monthValue = parsedMonth
                break
            }
        }

        if (yearValue == null || monthValue == null) {
            throw IllegalArgumentException(
                "Single TXT import requires headers yYYYY + mMM in file content."
            )
        }
        return ParsedMonthHeader(year = yearValue, month = monthValue)
    }

    private fun executeNativeDataQuery(
        request: DataQueryRequest,
        onRuntimePaths: ((RuntimePaths) -> Unit)? = null
    ): NativeCallResult {
        return executeAfterInit { paths ->
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

    private fun resetRuntimeCaches() {
        resetRuntimeStorageCaches(
            onRuntimePathsReset = { runtimePaths = null },
            onTextStorageReset = { textStorage = null },
            onConfigTomlStorageReset = { configTomlStorage = null }
        )
    }
}
