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
        val runningState = currentState.copy(statusText = "Initializing runtime...")
        val result = runtimeInitializer.initializeRuntime()
        return runningState.copy(
            initialized = result.initialized,
            statusText = if (result.initialized && result.operationOk) {
                ""
            } else {
                "Runtime initialization failed."
            }
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
                statusText = if (result.initialized && result.operationOk) {
                    "TXT import completed."
                } else {
                    "TXT import failed."
                }
            ),
            operationOk = result.operationOk
        )
    }

    suspend fun clearDataAndReinitialize(
        currentState: DataUiState,
        statusText: DestructiveActionStatusText
    ): DataUiState {
        val runningState = currentState.copy(statusText = statusText.running)
        val result = runtimeInitializer.clearAndReinitialize()
        return runningState.copy(
            initialized = result.initialized,
            statusText = if (result.operationOk) {
                statusText.success
            } else {
                statusText.failure
            }
        )
    }

    suspend fun clearDatabase(
        currentState: DataUiState,
        statusText: DestructiveActionStatusText
    ): DataUiState {
        val runningState = currentState.copy(statusText = statusText.running)
        val result = runtimeInitializer.clearDatabase()
        return runningState.copy(
            initialized = false,
            statusText = if (result.ok) {
                statusText.success
            } else {
                statusText.failure
            }
        )
    }

    suspend fun rebuildDatabase(
        currentState: DataUiState,
        statusText: DestructiveActionStatusText
    ): DataUiState {
        val runningState = currentState.copy(statusText = statusText.running)
        val result = runtimeInitializer.rebuildDatabase()
        return runningState.copy(
            initialized = result.initialized,
            statusText = if (result.initialized && result.operationOk) {
                statusText.success
            } else {
                statusText.failure
            }
        )
    }

    suspend fun clearTxt(
        currentState: DataUiState,
        statusText: DestructiveActionStatusText
    ): DataUiState {
        val runningState = currentState.copy(statusText = statusText.running)
        val result = recordGateway.clearTxt()
        return runningState.copy(
            statusText = if (result.ok) {
                statusText.success
            } else {
                statusText.failure
            }
        )
    }
}
