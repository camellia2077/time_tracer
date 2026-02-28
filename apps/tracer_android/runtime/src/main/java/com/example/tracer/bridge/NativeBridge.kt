package com.example.tracer

import java.util.concurrent.atomic.AtomicReference

object NativeBridge {
    const val UNSET_INT = -1

    const val DATE_CHECK_NONE = 0
    const val DATE_CHECK_CONTINUITY = 1
    const val DATE_CHECK_FULL = 2

    const val QUERY_ACTION_YEARS = 0
    const val QUERY_ACTION_MONTHS = 1
    const val QUERY_ACTION_DAYS = 2
    const val QUERY_ACTION_DAYS_DURATION = 3
    const val QUERY_ACTION_DAYS_STATS = 4
    const val QUERY_ACTION_SEARCH = 5
    const val QUERY_ACTION_ACTIVITY_SUGGEST = 6
    const val QUERY_ACTION_TREE = 7
    const val QUERY_ACTION_MAPPING_NAMES = 8
    const val QUERY_ACTION_REPORT_CHART = 9

    const val REPORT_MODE_SINGLE = 0
    const val REPORT_MODE_PERIOD_BATCH = 1

    const val REPORT_TYPE_DAY = 0
    const val REPORT_TYPE_MONTH = 1
    const val REPORT_TYPE_RECENT = 2
    const val REPORT_TYPE_WEEK = 3
    const val REPORT_TYPE_YEAR = 4
    const val REPORT_TYPE_RANGE = 5

    const val REPORT_FORMAT_MARKDOWN = 0

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

    external fun nativeValidateStructure(
        inputPath: String
    ): String

    external fun nativeValidateLogic(
        inputPath: String,
        dateCheckMode: Int
    ): String

    external fun nativeEncryptFile(
        inputPath: String,
        outputPath: String,
        passphrase: String,
        securityLevel: String
    ): String

    external fun nativeDecryptFile(
        inputPath: String,
        outputPath: String,
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
        overnight: Boolean,
        reverse: Boolean,
        limit: Int,
        topN: Int,
        lookbackDays: Int,
        scoreByDuration: Boolean,
        treePeriod: String,
        treePeriodArgument: String,
        treeMaxDepth: Int,
        outputMode: String
    ): String

    external fun nativeTree(
        listRoots: Boolean,
        rootPattern: String,
        maxDepth: Int,
        period: String,
        periodArgument: String,
        root: String
    ): String

    external fun nativeReport(
        mode: Int,
        reportType: Int,
        argument: String,
        format: Int,
        daysList: IntArray?
    ): String
}
