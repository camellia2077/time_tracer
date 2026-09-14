package com.example.tracer

import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Job
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch

/** Debounces edits and serializes persistence, retaining only the newest queued value. */
internal class LatestValueAutoSaveCoordinator<T>(
    private val scope: CoroutineScope,
    private val debounceMillis: Long,
    private val save: suspend (T) -> RecordActionResult,
    private val onLatestSaveFinished: (RecordActionResult) -> Unit
) {
    private data class Request<T>(val generation: Long, val value: T)

    private var generation = 0L
    private var latestRequest: Request<T>? = null
    private var pendingRequest: Request<T>? = null
    private var debounceJob: Job? = null
    private var writerJob: Job? = null

    fun schedule(value: T) {
        val request = Request(generation = ++generation, value = value).also { latestRequest = it }
        debounceJob?.cancel()
        debounceJob = scope.launch {
            delay(debounceMillis)
            enqueueIfLatest(request)
        }
    }

    /** Queues the latest draft immediately, for example when the editor closes. */
    fun flush() {
        val request = latestRequest ?: return
        debounceJob?.cancel()
        enqueueIfLatest(request)
    }

    private fun enqueueIfLatest(request: Request<T>) {
        if (latestRequest != request) return
        pendingRequest = request
        startWriterIfNeeded()
    }

    private fun startWriterIfNeeded() {
        if (writerJob?.isActive == true) return
        writerJob = scope.launch {
            while (pendingRequest != null) {
                val requestToWrite = requireNotNull(pendingRequest)
                pendingRequest = null
                val result = save(requestToWrite.value)
                if (latestRequest == requestToWrite) {
                    latestRequest = null
                    onLatestSaveFinished(result)
                }
            }
        }
    }
}
