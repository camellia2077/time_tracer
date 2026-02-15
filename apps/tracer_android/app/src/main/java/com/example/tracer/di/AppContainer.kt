package com.example.tracer.di

import android.content.Context
import com.example.tracer.ConfigGateway
import com.example.tracer.NativeRuntimeController
import com.example.tracer.QueryGateway
import com.example.tracer.RecordGateway
import com.example.tracer.ReportGateway
import com.example.tracer.RuntimeGateway
import com.example.tracer.RuntimeInitializer
import com.example.tracer.TxtStorageGateway
import com.example.tracer.data.UserPreferencesRepository
import com.example.tracer.data.dataStore

class AppContainer(private val appContext: Context) {
    private val nativeRuntimeController: NativeRuntimeController by lazy(LazyThreadSafetyMode.SYNCHRONIZED) {
        NativeRuntimeController(appContext)
    }

    val runtimeGateway: RuntimeGateway
        get() = nativeRuntimeController

    val runtimeInitializer: RuntimeInitializer
        get() = nativeRuntimeController

    val recordGateway: RecordGateway
        get() = nativeRuntimeController

    val reportGateway: ReportGateway
        get() = nativeRuntimeController

    val queryGateway: QueryGateway
        get() = nativeRuntimeController

    val txtStorageGateway: TxtStorageGateway
        get() = nativeRuntimeController

    val configGateway: ConfigGateway
        get() = nativeRuntimeController

    val userPreferencesRepository: UserPreferencesRepository by lazy(LazyThreadSafetyMode.SYNCHRONIZED) {
        UserPreferencesRepository(appContext.dataStore)
    }
}
