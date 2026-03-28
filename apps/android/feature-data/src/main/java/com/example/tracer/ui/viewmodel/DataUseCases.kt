package com.example.tracer

internal class DataUseCases(
    private val runtimeInitializer: RuntimeInitializer,
    private val recordGateway: RecordGateway
) {
    data class OperationUpdate(
        val state: DataUiState,
        val operationOk: Boolean
    )

    suspend fun initializeRuntime(currentState: DataUiState): DataUiState {
        val runningState = currentState.copy(statusText = "nativeInit running...")
        val result = runtimeInitializer.initializeRuntime()
        return runningState.copy(
            initialized = result.initialized,
            statusText = "nativeInit -> ${result.rawResponse}"
        )
    }

    suspend fun ingestSingleTxtReplaceMonth(
        currentState: DataUiState,
        inputPath: String
    ): DataUiState {
        return ingestSingleTxtReplaceMonthWithResult(currentState, inputPath).state
    }

    suspend fun ingestSingleTxtReplaceMonthWithResult(
        currentState: DataUiState,
        inputPath: String
    ): OperationUpdate {
        val runningState = currentState.copy(
            statusText = "TXT import running..."
        )
        val result = runtimeInitializer.ingestSingleTxtReplaceMonth(inputPath)
        return OperationUpdate(
            state = runningState.copy(
                initialized = result.initialized,
                statusText = "TXT import -> ${result.rawResponse}"
            ),
            operationOk = result.operationOk
        )
    }

    suspend fun clearDataAndReinitialize(currentState: DataUiState): DataUiState {
        val runningState = currentState.copy(statusText = "clearing app data...")
        val result = runtimeInitializer.clearAndReinitialize()
        return runningState.copy(
            initialized = result.initialized,
            statusText = "${result.clearMessage}\nnativeInit -> ${result.initResponse}"
        )
    }

    suspend fun clearDatabase(currentState: DataUiState): DataUiState {
        val runningState = currentState.copy(statusText = "clearing database...")
        val result = runtimeInitializer.clearDatabase()
        return runningState.copy(
            initialized = false,
            statusText = result.message
        )
    }

    suspend fun clearTxt(currentState: DataUiState): DataUiState {
        val runningState = currentState.copy(statusText = "clearing txt...")
        val result = recordGateway.clearTxt()
        return runningState.copy(statusText = result.message)
    }
}
