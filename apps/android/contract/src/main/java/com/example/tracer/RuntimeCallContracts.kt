package com.example.tracer

/** Shared lifecycle and mutation outcomes exposed by runtime capabilities. */
data class NativeCallResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val rawResponse: String,
    val errorLogPath: String = "",
    val operationId: String = ""
)

data class ClearAndInitResult(
    val initialized: Boolean,
    val operationOk: Boolean,
    val clearMessage: String,
    val initResponse: String,
    val operationId: String = ""
)

data class ClearTxtResult(
    val ok: Boolean,
    val message: String
)

data class ClearDatabaseResult(
    val ok: Boolean,
    val message: String
)

data class RecordActionResult(
    val ok: Boolean,
    val message: String,
    val operationId: String = ""
)
