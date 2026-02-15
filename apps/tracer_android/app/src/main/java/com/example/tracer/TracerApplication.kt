package com.example.tracer

import android.app.Application
import com.example.tracer.di.AppContainer

class TracerApplication : Application() {
    val appContainer: AppContainer by lazy(LazyThreadSafetyMode.SYNCHRONIZED) {
        AppContainer(applicationContext)
    }
}

