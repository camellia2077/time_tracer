package com.example.tracer

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

    const val REPORT_MODE_SINGLE = 0
    const val REPORT_MODE_PERIOD_BATCH = 1

    const val REPORT_TYPE_DAY = 0
    const val REPORT_TYPE_MONTH = 1
    const val REPORT_TYPE_RECENT = 2
    const val REPORT_TYPE_WEEK = 3
    const val REPORT_TYPE_YEAR = 4
    const val REPORT_TYPE_RANGE = 5

    const val REPORT_FORMAT_MARKDOWN = 0

    init {
        System.loadLibrary("time_tracker_android_bridge")
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

    external fun nativeQuery(
        action: Int,
        year: Int,
        month: Int,
        fromDate: String,
        toDate: String,
        remark: String,
        dayRemark: String,
        project: String,
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
        treeMaxDepth: Int
    ): String

    external fun nativeReport(
        mode: Int,
        reportType: Int,
        argument: String,
        format: Int,
        daysList: IntArray?
    ): String
}
