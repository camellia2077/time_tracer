package com.example.tracer.di

import android.content.Context
import com.example.tracer.ConfigGateway
import com.example.tracer.ActivityHierarchyGateway
import com.example.tracer.ActivityHierarchyMigrationGateway
import com.example.tracer.NativeRuntimeController
import com.example.tracer.QueryGateway
import com.example.tracer.QuickAccessGateway
import com.example.tracer.RecordGateway
import com.example.tracer.InsightsGateway
import com.example.tracer.RuntimeInitializer
import com.example.tracer.TracerExchangeGateway
import com.example.tracer.TxtStorageGateway
import com.example.tracer.data.UserPreferencesRepository
import com.example.tracer.data.dataStore
import kotlinx.coroutines.flow.first
import kotlinx.coroutines.runBlocking
import org.json.JSONArray
import org.json.JSONObject

class AppContainer(private val appContext: Context) {
    private val nativeRuntimeController: NativeRuntimeController by lazy(LazyThreadSafetyMode.SYNCHRONIZED) {
        NativeRuntimeController(appContext) {
            runBlocking {
                userPreferencesRepository.insightsStatusConfigs.first().toRuntimeJson()
            }
        }
    }

    val runtimeInitializer: RuntimeInitializer
        get() = nativeRuntimeController

    val recordGateway: RecordGateway
        get() = nativeRuntimeController

    val insightsGateway: InsightsGateway
        get() = nativeRuntimeController

    val queryGateway: QueryGateway
        get() = nativeRuntimeController

    val txtStorageGateway: TxtStorageGateway
        get() = nativeRuntimeController

    val configGateway: ConfigGateway
        get() = nativeRuntimeController

    val quickAccessGateway: QuickAccessGateway
        get() = nativeRuntimeController

    val activityHierarchyGateway: ActivityHierarchyGateway
        get() = nativeRuntimeController

    val activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway
        get() = nativeRuntimeController

    val tracerExchangeGateway: TracerExchangeGateway
        get() = nativeRuntimeController

    val userPreferencesRepository: UserPreferencesRepository by lazy(LazyThreadSafetyMode.SYNCHRONIZED) {
        UserPreferencesRepository(appContext.dataStore)
    }
}

private fun com.example.tracer.data.InsightsStatusConfigs.toRuntimeJson(): String =
    JSONObject()
        .apply {
            put("day", day.toJson())
            put("week", week.toJson())
            put("month", month.toJson())
            put("year", year.toJson())
            put("recent", recent.toJson())
            put("range", range.toJson())
        }
        .toString()

private fun com.example.tracer.data.DailyStatusConfig.toJson(): JSONArray =
    JSONArray().apply {
        this@toJson.statuses.forEach { status ->
                    put(
                        JSONObject()
                            .put("id", status.id)
                            .put("label", status.label)
                            .put("parent", status.parent)
                    )
            }
    }
