package com.example.tracer

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.util.Log
import kotlinx.coroutines.runBlocking

/** ADB-only command bridge included in debug builds. */
class DebugCommandReceiver : BroadcastReceiver() {
    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action != ACTION_REBUILD_DATABASE) {
            setResultCode(2)
            setResultData("Unsupported debug command: ${intent.action}")
            return
        }

        val result = runBlocking {
            (context.applicationContext as TracerApplication)
                .appContainer
                .runtimeInitializer
                .rebuildDatabase()
        }
        val succeeded = result.initialized && result.operationOk
        if (BuildConfig.DEBUG) {
            Log.i(
                "TracerDebugCommand",
                "rebuild database initialized=${result.initialized} " +
                    "operationOk=${result.operationOk} response=${result.rawResponse}"
            )
        }
        setResultCode(if (succeeded) 0 else 1)
        setResultData(result.rawResponse)
    }

    companion object {
        const val ACTION_REBUILD_DATABASE =
            "com.example.tracer.action.REBUILD_DATABASE"
    }
}
