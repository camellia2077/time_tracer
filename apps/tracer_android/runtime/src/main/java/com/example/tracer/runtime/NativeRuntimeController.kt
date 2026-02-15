package com.example.tracer

import android.content.Context
import kotlinx.coroutines.CancellationException
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject
import java.text.SimpleDateFormat
import java.util.Calendar
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
        buildNativeErrorResponse = ::buildNativeErrorResponse,
        formatFailure = ::formatFailure
    )
    private val rawRecordStore = LiveRawRecordStore()
    private val syncUseCase = IngestSyncUseCase(AutoSyncMaterializer())
    private val activityAliasResolver = ActivityAliasResolver()

    override suspend fun initializeRuntime(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            initializeRuntimeInternal()
        } catch (error: Exception) {
            onErrorNativeCall(prefix = "nativeInit failed", error = error)
        }
    }

    override suspend fun querySmoke(): NativeCallResult = withContext(Dispatchers.IO) {
        executeAfterInit {
            NativeBridge.nativeQuery(
                action = NativeBridge.QUERY_ACTION_YEARS,
                year = 0,
                month = 0,
                fromDate = "",
                toDate = "",
                remark = "",
                dayRemark = "",
                project = "",
                exercise = NativeBridge.UNSET_INT,
                status = NativeBridge.UNSET_INT,
                overnight = false,
                reverse = false,
                limit = NativeBridge.UNSET_INT,
                topN = NativeBridge.UNSET_INT,
                lookbackDays = NativeBridge.UNSET_INT,
                scoreByDuration = false,
                treePeriod = "",
                treePeriodArgument = "",
                treeMaxDepth = NativeBridge.UNSET_INT
            )
        }
    }

    override suspend fun ingestSmoke(): NativeCallResult = withContext(Dispatchers.IO) {
        executeAfterInit { paths ->
            NativeBridge.nativeIngest(
                inputPath = paths.smokeInputPath,
                dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY,
                saveProcessedOutput = false
            )
        }
    }

    override suspend fun ingestFull(): NativeCallResult = withContext(Dispatchers.IO) {
        executeAfterInit { paths ->
            NativeBridge.nativeIngest(
                inputPath = paths.fullInputPath,
                dateCheckMode = NativeBridge.DATE_CHECK_CONTINUITY,
                saveProcessedOutput = false
            )
        }
    }

    override suspend fun createCurrentMonthTxt(): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureTextStorage()
            val ensureMonthResult = storage.createCurrentMonthFile()
            val action = if (ensureMonthResult.created) {
                "created"
            } else {
                "already exists"
            }
            RecordActionResult(
                ok = true,
                message = "month txt $action -> ${ensureMonthResult.monthFile.absolutePath}"
            )
        } catch (error: Exception) {
            onErrorRecordAction(prefix = "Create current month txt failed", error = error)
        }
    }

    override suspend fun createMonthTxt(month: String): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val match = Regex("""^(\d{4})-(\d{2})$""").matchEntire(month.trim())
            if (match == null) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Invalid month format. Use YYYY-MM."
                )
            }

            val year = match.groupValues[1].toIntOrNull()
            val monthValue = match.groupValues[2].toIntOrNull()
            if (year == null || monthValue == null || monthValue !in 1..12) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Invalid month value. Use YYYY-MM with month 01..12."
                )
            }

            val storage = ensureTextStorage()
            val ensureMonthResult = storage.createMonthFile(
                year = year,
                month = monthValue
            )
            val action = if (ensureMonthResult.created) {
                "created"
            } else {
                "already exists"
            }
            RecordActionResult(
                ok = true,
                message = "month txt $action -> ${ensureMonthResult.monthFile.absolutePath}"
            )
        } catch (error: Exception) {
            onErrorRecordAction(prefix = "Create month txt failed", error = error)
        }
    }

    override suspend fun recordNow(
        activityName: String,
        remark: String,
        targetDateIso: String?
    ): RecordActionResult = withContext(Dispatchers.IO) {
        try {
            val paths = ensureRuntimePaths()
            val storage = ensureTextStorage()
            val logicalDateResult = parseLogicalDate(targetDateIso)
            if (!logicalDateResult.ok || logicalDateResult.date == null) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = logicalDateResult.message
                )
            }
            val logicalDate = logicalDateResult.date

            val normalizedActivity = rawRecordStore.normalizeActivityName(activityName)
            if (normalizedActivity.isEmpty()) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "Activity name is empty."
                )
            }
            val normalizedRemark = rawRecordStore.normalizeRemark(remark)
            val ensureMonthResult = try {
                val cal = Calendar.getInstance().apply {
                    time = SimpleDateFormat("yyyy-MM-dd", Locale.US).parse(logicalDate)
                        ?: throw IllegalArgumentException("Invalid date format")
                }
                storage.createMonthFile(
                    year = cal.get(Calendar.YEAR),
                    month = cal.get(Calendar.MONTH) + 1
                )
            } catch (error: Exception) {
                return@withContext onErrorRecordAction(
                    prefix = "Record failed: cannot prepare month file",
                    error = error
                )
            }
            val createdMonthFile = ensureMonthResult.created

            val snapshot = try {
                rawRecordStore.appendRecord(
                    liveRawInputPath = paths.liveRawInputPath,
                    logicalDateString = logicalDate!!,
                    activityName = normalizedActivity,
                    remark = normalizedRemark
                )
            } catch (error: Exception) {
                return@withContext onErrorRecordAction(
                    prefix = "Record failed: cannot write TXT",
                    error = error
                )
            }
            val recordSummary = buildRecordSummary(
                snapshot = snapshot,
                monthFileCreated = createdMonthFile
            )

            val syncResult = executeAfterInit { activePaths ->
                syncUseCase.run(activePaths)
            }

            if (!syncResult.initialized) {
                return@withContext RecordActionResult(
                    ok = false,
                    message = "$recordSummary\nsync: failed (native init failed)"
                )
            }
            if (!syncResult.operationOk) {
                val payload = responseCodec.parse(syncResult.rawResponse)
                val error = if (payload.errorMessage.isNotEmpty()) {
                    payload.errorMessage
                } else {
                    syncResult.rawResponse
                }
                return@withContext RecordActionResult(
                    ok = false,
                    message = "$recordSummary\nsync: failed ($error)"
                )
            }

            RecordActionResult(
                ok = true,
                message = "$recordSummary\nsync: ok"
            )
        } catch (error: Exception) {
            onErrorRecordAction(prefix = "Record failed", error = error)
        }
    }

    override suspend fun syncLiveToDatabase(): NativeCallResult = withContext(Dispatchers.IO) {
        try {
            executeAfterInit { paths ->
                syncUseCase.run(paths)
            }
        } catch (error: Exception) {
            onErrorNativeCall(prefix = "syncLiveToDatabase failed", error = error)
        }
    }

    override suspend fun listLiveTxtFiles(): TxtHistoryListResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureTextStorage()
            storage.listTxtFiles()
        } catch (error: Exception) {
            TxtHistoryListResult(
                ok = false,
                files = emptyList(),
                message = formatFailure("list live txt failed", error)
            )
        }
    }

    override suspend fun readLiveTxtFile(relativePath: String): TxtFileContentResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureTextStorage()
            storage.readTxtFile(relativePath)
        } catch (error: Exception) {
            TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = formatFailure("read live txt failed", error)
            )
        }
    }

    override suspend fun saveLiveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        withContext(Dispatchers.IO) {
            try {
                ensureRuntimePaths()
                val storage = ensureTextStorage()
                val saveResult = storage.writeTxtFile(relativePath = relativePath, content = content)
                if (!saveResult.ok) {
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save failed: ${saveResult.message}"
                    )
                }

                val syncResult = executeAfterInit { activePaths ->
                    syncUseCase.run(activePaths)
                }
                if (!syncResult.initialized) {
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save ok, sync failed: native init failed."
                    )
                }
                if (!syncResult.operationOk) {
                    val payload = responseCodec.parse(syncResult.rawResponse)
                    val error = if (payload.errorMessage.isNotEmpty()) {
                        payload.errorMessage
                    } else {
                        syncResult.rawResponse
                    }
                    return@withContext RecordActionResult(
                        ok = false,
                        message = "save ok, sync failed: $error"
                    )
                }

                RecordActionResult(
                    ok = true,
                    message = "save+sync -> ${saveResult.filePath}"
                )
            } catch (error: Exception) {
                onErrorRecordAction(prefix = "save live txt failed", error = error)
            }
        }

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureConfigTomlStorage()
            storage.listTomlFiles()
        } catch (error: Exception) {
            ConfigTomlListResult(
                ok = false,
                converterFiles = emptyList(),
                reportFiles = emptyList(),
                message = formatFailure("list config toml failed", error)
            )
        }
    }

    override suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult = withContext(Dispatchers.IO) {
        try {
            val storage = ensureConfigTomlStorage()
            storage.readTomlFile(relativePath)
        } catch (error: Exception) {
            TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = formatFailure("read config toml failed", error)
            )
        }
    }

    override suspend fun saveConfigTomlFile(relativePath: String, content: String): TxtFileContentResult =
        withContext(Dispatchers.IO) {
            try {
                val storage = ensureConfigTomlStorage()
                storage.writeTomlFile(relativePath, content)
            } catch (error: Exception) {
                TxtFileContentResult(
                    ok = false,
                    filePath = relativePath,
                    content = "",
                    message = formatFailure("save config toml failed", error)
                )
            }
        }

    override suspend fun reportDayMarkdown(date: String): ReportCallResult = withContext(Dispatchers.IO) {
        executeReportAfterInit {
            NativeBridge.nativeReport(
                mode = NativeBridge.REPORT_MODE_SINGLE,
                reportType = NativeBridge.REPORT_TYPE_DAY,
                argument = date,
                format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                daysList = null
            )
        }
    }

    override suspend fun reportMonthMarkdown(month: String): ReportCallResult =
        withContext(Dispatchers.IO) {
            executeReportAfterInit {
                NativeBridge.nativeReport(
                    mode = NativeBridge.REPORT_MODE_SINGLE,
                    reportType = NativeBridge.REPORT_TYPE_MONTH,
                    argument = month,
                    format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                    daysList = null
                )
            }
        }

    override suspend fun reportYearMarkdown(year: String): ReportCallResult =
        withContext(Dispatchers.IO) {
            executeReportAfterInit {
                NativeBridge.nativeReport(
                    mode = NativeBridge.REPORT_MODE_SINGLE,
                    reportType = NativeBridge.REPORT_TYPE_YEAR,
                    argument = year,
                    format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                    daysList = null
                )
            }
        }

    override suspend fun reportWeekMarkdown(week: String): ReportCallResult =
        withContext(Dispatchers.IO) {
            executeReportAfterInit {
                NativeBridge.nativeReport(
                    mode = NativeBridge.REPORT_MODE_SINGLE,
                    reportType = NativeBridge.REPORT_TYPE_WEEK,
                    argument = week,
                    format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                    daysList = null
                )
            }
        }

    override suspend fun reportRecentMarkdown(days: String): ReportCallResult =
        withContext(Dispatchers.IO) {
            executeReportAfterInit {
                NativeBridge.nativeReport(
                    mode = NativeBridge.REPORT_MODE_SINGLE,
                    reportType = NativeBridge.REPORT_TYPE_RECENT,
                    argument = days,
                    format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                    daysList = null
                )
            }
        }

    override suspend fun reportRange(startDate: String, endDate: String): ReportCallResult =
        withContext(Dispatchers.IO) {
            executeReportAfterInit {
                NativeBridge.nativeReport(
                    mode = NativeBridge.REPORT_MODE_SINGLE,
                    reportType = NativeBridge.REPORT_TYPE_RANGE,
                    argument = "$startDate|$endDate",
                    format = NativeBridge.REPORT_FORMAT_MARKDOWN,
                    daysList = null
                )
            }
        }

    override suspend fun queryActivitySuggestions(
        lookbackDays: Int,
        topN: Int
    ): ActivitySuggestionResult = withContext(Dispatchers.IO) {
        if (lookbackDays <= 0) {
            return@withContext ActivitySuggestionResult(
                ok = false,
                suggestions = emptyList(),
                message = "lookbackDays must be greater than 0."
            )
        }
        if (topN <= 0) {
            return@withContext ActivitySuggestionResult(
                ok = false,
                suggestions = emptyList(),
                message = "topN must be greater than 0."
            )
        }

        try {
            var activePaths: RuntimePaths? = null
            val queryResult = executeNativeDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_ACTIVITY_SUGGEST,
                    topN = topN,
                    lookbackDays = lookbackDays
                ),
                onRuntimePaths = { paths -> activePaths = paths }
            )

            if (!queryResult.initialized) {
                return@withContext ActivitySuggestionResult(
                    ok = false,
                    suggestions = emptyList(),
                    message = extractNativeInitFailureMessage(queryResult.rawResponse)
                )
            }

            val payload = responseCodec.parse(queryResult.rawResponse)
            if (!payload.ok) {
                return@withContext ActivitySuggestionResult(
                    ok = false,
                    suggestions = emptyList(),
                    message = payload.errorMessage.ifEmpty { "query activity suggestions failed." }
                )
            }

            val rawActivities = parseSuggestedActivities(payload.content)
            val aliasMapping = activityAliasResolver.loadAliasMapping(activePaths)
            val validActivityNames = activityAliasResolver.buildActivityNameSet(aliasMapping)
            val suggestions = activityAliasResolver.normalizeSuggestedActivities(
                activities = rawActivities,
                aliasMapping = aliasMapping,
                validActivityNames = validActivityNames,
                maxItems = topN
            )

            ActivitySuggestionResult(
                ok = true,
                suggestions = suggestions,
                message = if (suggestions.isEmpty()) {
                    "No activity suggestions in recent $lookbackDays days."
                } else {
                    "Loaded ${suggestions.size} activity suggestion(s)."
                }
            )
        } catch (error: Exception) {
            ActivitySuggestionResult(
                ok = false,
                suggestions = emptyList(),
                message = formatFailure("query activity suggestions failed", error)
            )
        }
    }

    override suspend fun queryDayDurations(params: DataDurationQueryParams): DataQueryTextResult =
        withContext(Dispatchers.IO) {
            runDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_DAYS_DURATION,
                    year = params.year,
                    month = params.month,
                    fromDateIso = params.fromDateIso,
                    toDateIso = params.toDateIso,
                    reverse = params.reverse,
                    limit = params.limit
                )
            )
        }

    override suspend fun queryDayDurationStats(params: DataDurationQueryParams): DataQueryTextResult =
        withContext(Dispatchers.IO) {
            val period = params.period
            val normalizedPeriodArgument = normalizePeriodArgument(params.periodArgument)
            if (period != null) {
                val validationError = validatePeriodArgument(period, normalizedPeriodArgument)
                if (validationError != null) {
                    return@withContext validationError
                }
            }
            runDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_DAYS_STATS,
                    year = params.year,
                    month = params.month,
                    fromDateIso = params.fromDateIso,
                    toDateIso = params.toDateIso,
                    topN = params.topN,
                    treePeriod = period?.wireValue,
                    treePeriodArgument = if (period == null) {
                        null
                    } else {
                        normalizedPeriodArgument
                    }
                )
            )
        }

    override suspend fun queryProjectTree(params: DataTreeQueryParams): DataQueryTextResult =
        withContext(Dispatchers.IO) {
            val normalizedPeriodArgument = normalizePeriodArgument(params.periodArgument)
            val validationError = validatePeriodArgument(params.period, normalizedPeriodArgument)
            if (validationError != null) {
                return@withContext validationError
            }
            runDataQuery(
                request = DataQueryRequest(
                    action = NativeBridge.QUERY_ACTION_TREE,
                    treePeriod = params.period.wireValue,
                    treePeriodArgument = normalizedPeriodArgument,
                    treeMaxDepth = params.level
                )
            )
        }

    override suspend fun listActivityMappingNames(): ActivityMappingNamesResult =
        withContext(Dispatchers.IO) {
            try {
                val paths = ensureRuntimePaths()
                val aliasMapping = activityAliasResolver.loadAliasMapping(paths)
                val names = activityAliasResolver.buildActivityNameSet(aliasMapping).toList().sorted()
                if (names.isEmpty()) {
                    return@withContext ActivityMappingNamesResult(
                        ok = false,
                        names = emptyList(),
                        message = "No valid entries found in mapping_config.toml [text_mappings]."
                    )
                }

                ActivityMappingNamesResult(
                    ok = true,
                    names = names,
                    message = "Loaded ${names.size} mapping names."
                )
            } catch (error: Exception) {
                ActivityMappingNamesResult(
                    ok = false,
                    names = emptyList(),
                    message = formatFailure("list activity mapping names failed", error)
                )
            }
        }

    override suspend fun clearLiveTxt(): ClearTxtResult = withContext(Dispatchers.IO) {
        try {
            ensureRuntimePaths()
            runtimeEnvironment.clearLiveTxtData()
        } catch (error: Exception) {
            ClearTxtResult(
                ok = false,
                message = formatFailure("clear live txt failed", error)
            )
        }
    }

    override suspend fun clearAndReinitialize(): ClearAndInitResult = withContext(Dispatchers.IO) {
        try {
            val clearMessage = runtimeEnvironment.clearRuntimeData()
            runtimePaths = null
            textStorage = null
            configTomlStorage = null
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
                initResponse = buildNativeErrorResponse(formatFailure("clear and reinitialize failed", error))
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
        return NativeCallResult(
            initialized = payload.ok,
            operationOk = payload.ok,
            rawResponse = response
        )
    }

    private fun ensureRuntimePaths(): RuntimePaths {
        val existing = runtimePaths
        if (existing != null) {
            return existing
        }

        val prepared = runtimeEnvironment.prepareRuntimePaths()
        runtimePaths = prepared
        return prepared
    }

    private fun ensureTextStorage(): TextStorage {
        val existing = textStorage
        if (existing != null) {
            return existing
        }
        val paths = ensureRuntimePaths()
        val created = LiveRawTextStorage(
            liveRawInputPath = paths.liveRawInputPath,
            recordStore = rawRecordStore
        )
        textStorage = created
        return created
    }

    private fun ensureConfigTomlStorage(): ConfigTomlStorage {
        val existing = configTomlStorage
        if (existing != null) {
            return existing
        }
        val paths = ensureRuntimePaths()
        val created = ConfigTomlStorage(paths.configRootPath)
        configTomlStorage = created
        return created
    }

    private fun executeAfterInit(action: (RuntimePaths) -> String): NativeCallResult {
        return callExecutor.executeAfterInit(action)
    }

    private fun executeReportAfterInit(action: (RuntimePaths) -> String): ReportCallResult {
        return callExecutor.executeReportAfterInit(action)
    }

    private fun runDataQuery(request: DataQueryRequest): DataQueryTextResult {
        val queryResult = executeNativeDataQuery(request)

        if (!queryResult.initialized) {
            return DataQueryTextResult(
                ok = false,
                outputText = "",
                message = extractNativeInitFailureMessage(queryResult.rawResponse)
            )
        }

        val payload = responseCodec.parse(queryResult.rawResponse)
        return DataQueryTextResult(
            ok = payload.ok,
            outputText = if (payload.ok) payload.content else "",
            message = if (payload.ok) {
                "query ok"
            } else {
                payload.errorMessage.ifEmpty { "data query failed." }
            }
        )
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
                treeMaxDepth = request.treeMaxDepth ?: NativeBridge.UNSET_INT
            )
        }
    }

    private fun normalizePeriodArgument(periodArgument: String?): String {
        return periodArgument?.trim().orEmpty()
    }

    private fun validatePeriodArgument(
        period: DataTreePeriod,
        normalizedPeriodArgument: String
    ): DataQueryTextResult? {
        if (period == DataTreePeriod.RECENT || normalizedPeriodArgument.isNotBlank()) {
            return null
        }
        return DataQueryTextResult(
            ok = false,
            outputText = "",
            message = "periodArgument is required for period ${period.wireValue}."
        )
    }

    private fun extractNativeInitFailureMessage(rawResponse: String): String {
        return responseCodec.parse(rawResponse).errorMessage.ifEmpty { "native init failed." }
    }

    private fun parseSuggestedActivities(content: String): List<String> {
        val activities = mutableListOf<String>()
        for (rawLine in content.lineSequence()) {
            val line = rawLine.trim()
            if (line.isEmpty() || line.startsWith("Total:")) {
                continue
            }
            val activity = line.substringBefore("|").trim()
            if (activity.isNotEmpty()) {
                activities += activity
            }
        }
        return activities
    }

    private data class DataQueryRequest(
        val action: Int,
        val year: Int? = null,
        val month: Int? = null,
        val fromDateIso: String? = null,
        val toDateIso: String? = null,
        val remark: String? = null,
        val dayRemark: String? = null,
        val project: String? = null,
        val exercise: Int? = null,
        val status: Int? = null,
        val overnight: Boolean = false,
        val reverse: Boolean = false,
        val limit: Int? = null,
        val topN: Int? = null,
        val lookbackDays: Int? = null,
        val scoreByDuration: Boolean = false,
        val treePeriod: String? = null,
        val treePeriodArgument: String? = null,
        val treeMaxDepth: Int? = null
    )

    private data class LogicalDateParseResult(
        val ok: Boolean,
        val date: String?,
        val message: String
    )

    private fun parseLogicalDate(targetDateIso: String?): LogicalDateParseResult {
        if (targetDateIso.isNullOrBlank()) {
            val nowStr = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(Calendar.getInstance().time)
            return LogicalDateParseResult(
                ok = true,
                date = nowStr,
                message = ""
            )
        }

        return try {
            val sdf = SimpleDateFormat("yyyy-MM-dd", Locale.US).apply { isLenient = false }
            val parsed = sdf.parse(targetDateIso.trim())
            LogicalDateParseResult(
                ok = true,
                date = SimpleDateFormat("yyyy-MM-dd", Locale.US).format(parsed!!),
                message = ""
            )
        } catch (_: Exception) {
            LogicalDateParseResult(
                ok = false,
                date = null,
                message = "Invalid target date. Use ISO YYYY-MM-DD."
            )
        }
    }

    private fun buildRecordSummary(
        snapshot: RecordWriteSnapshot,
        monthFileCreated: Boolean
    ): String {
        val parts = mutableListOf<String>()
        parts += "record: ok"
        parts += "logical_date: ${snapshot.logicalDate}"
        parts += "target_file: ${snapshot.monthFile.absolutePath}"
        parts += "write_time: ${snapshot.eventTime}"
        if (monthFileCreated) {
            parts += "month_file_created: true"
        }
        for (warning in snapshot.warnings) {
            parts += "warning: $warning"
        }
        return parts.joinToString(separator = "\n")
    }

    private fun onErrorNativeCall(prefix: String, error: Exception): NativeCallResult {
        val message = formatFailure(prefix, error)
        return NativeCallResult(
            initialized = false,
            operationOk = false,
            rawResponse = buildNativeErrorResponse(message)
        )
    }

    private fun onErrorRecordAction(prefix: String, error: Exception): RecordActionResult {
        return RecordActionResult(
            ok = false,
            message = formatFailure(prefix, error)
        )
    }

    private fun formatFailure(prefix: String, error: Exception): String {
        if (error is CancellationException) {
            throw error
        }
        val details = error.message?.takeIf { it.isNotBlank() } ?: (error::class.simpleName ?: "Unknown error")
        return "$prefix: $details"
    }

    private fun buildNativeErrorResponse(errorMessage: String): String {
        return JSONObject()
            .put("ok", false)
            .put("content", "")
            .put("error_message", errorMessage)
            .toString()
    }

}
