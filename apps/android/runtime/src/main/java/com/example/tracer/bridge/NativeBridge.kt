package com.example.tracer

import java.util.concurrent.atomic.AtomicReference

/**
 * Raw JNI surface mirrored 1:1 from `apps/tracer_core_shell/api/android_jni`.
 *
 * Kotlin runtime code should prefer `NativeRuntimeBridge` so JNI wiring,
 * protocol-code translation, and future signature churn stay isolated here.
 */
object NativeBridge {
    const val UNSET_INT = -1

    // Android user data may contain intentionally unrecorded days. Production
    // Android flows therefore pass DATE_CHECK_NONE explicitly; continuity and
    // full remain available only for callers that deliberately request them.
    const val DATE_CHECK_NONE = 0
    const val DATE_CHECK_CONTINUITY = 1
    const val DATE_CHECK_FULL = 2
    const val RECORD_TIME_ORDER_STRICT_CALENDAR = 0
    const val RECORD_TIME_ORDER_LOGICAL_DAY_0600 = 1

    const val QUERY_ACTION_YEARS = 0
    const val QUERY_ACTION_MONTHS = 1
    const val QUERY_ACTION_DAYS = 2
    const val QUERY_ACTION_DAYS_DURATION = 3
    const val QUERY_ACTION_DAYS_STATS = 4
    const val QUERY_ACTION_SEARCH = 5
    const val QUERY_ACTION_ACTIVITY_SUGGEST = 6
    const val QUERY_ACTION_TREE = 7
    const val QUERY_ACTION_MAPPING_NAMES = 8
    const val QUERY_ACTION_ACTIVITY_ALIAS_MAPPINGS = 9
    const val QUERY_ACTION_INSIGHTS_CHART = 10
    const val QUERY_ACTION_MAPPING_ALIAS_KEYS = 11
    const val QUERY_ACTION_WAKE_KEYWORDS = 12
    const val QUERY_ACTION_AUTHORABLE_EVENT_TOKENS = 13
    const val QUERY_ACTION_INSIGHTS_COMPOSITION = 14

    private val cryptoProgressListenerRef = AtomicReference<((String) -> Unit)?>(null)

    init {
        System.loadLibrary("time_tracker_android_bridge")
    }

    fun setCryptoProgressListener(listener: ((String) -> Unit)?) {
        cryptoProgressListenerRef.set(listener)
    }

    @JvmStatic
    fun onCryptoProgressJson(progressJson: String) {
        cryptoProgressListenerRef.get()?.invoke(progressJson)
    }

    external fun nativeInit(
        dbPath: String,
        outputRoot: String,
        converterConfigTomlPath: String
    ): String

    external fun nativeInitPipeline(
        dbPath: String,
        outputRoot: String,
        converterConfigTomlPath: String
    ): String

    external fun nativeShutdown(): String

    external fun nativeIngest(
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ): String

    external fun nativeIngestSingleTxtReplaceMonth(
        inputPath: String,
        dateCheckMode: Int,
        saveProcessedOutput: Boolean
    ): String

    external fun nativeListTxtIngestSyncStatus(
        requestJson: String
    ): String

    external fun nativeClearTxtIngestSyncStatus(): String

    external fun nativeValidateStructure(
        inputPath: String
    ): String

    external fun nativeValidateLogic(
        inputPath: String,
        dateCheckMode: Int
    ): String

    external fun nativeRecordActivityAtomically(
        targetDateIso: String,
        rawActivityName: String,
        remark: String,
        preferredTxtPath: String,
        dateCheckMode: Int,
        timeOrderMode: Int
    ): String

    external fun nativeUpdateActivityRemarkAtomically(
        targetDateIso: String,
        logicalId: Long,
        remark: String,
        preferredTxtPath: String,
        dateCheckMode: Int
    ): String

    external fun nativeUpdateDayRemarkAtomically(
        targetDateIso: String,
        remark: String,
        preferredTxtPath: String,
        dateCheckMode: Int
    ): String

    external fun nativeConfig(
        requestJson: String
    ): String

    external fun nativeExportTracerExchange(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: String,
        dateCheckMode: Int
    ): String

    external fun nativeExportTracerExchangeFromPayloadJson(
        requestJson: String,
        outputFd: Int
    ): String

    external fun nativeImportTracerExchange(
        inputPath: String,
        workRoot: String,
        passphrase: String
    ): String

    external fun nativeInspectTracerExchange(
        inputPath: String,
        passphrase: String
    ): String

    external fun nativeQuery(
        action: Int,
        year: Int,
        month: Int,
        fromDate: String,
        toDate: String,
        remark: String,
        dayRemark: String,
        project: String,
        root: String,
        exercise: Int,
        status: Int,
        crossMidnightActivity: Boolean,
        reverse: Boolean,
        limit: Int,
        topN: Int,
        lookbackDays: Int,
        anchorDate: String,
        scoreByDuration: Boolean,
        treePeriod: String,
        treePeriodArgument: String,
        treeMaxDepth: Int,
        outputMode: String,
        averageDayBasis: String
    ): String

    external fun nativeInsightsJson(
        requestJson: String
    ): String
}
