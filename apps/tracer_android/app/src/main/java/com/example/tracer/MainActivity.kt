package com.example.tracer

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.tracer.ui.theme.TracerTheme
import com.example.tracer.ui.viewmodel.ThemeViewModel
import com.example.tracer.ui.viewmodel.ThemeViewModelFactory

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        val appContainer = (application as TracerApplication).appContainer
        val runtimeGateway = appContainer.runtimeGateway
        val runtimeInitializer = appContainer.runtimeInitializer
        val recordGateway = appContainer.recordGateway
        val txtStorageGateway = appContainer.txtStorageGateway
        val reportGateway = appContainer.reportGateway
        val queryGateway = appContainer.queryGateway
        val userPreferencesRepository = appContainer.userPreferencesRepository

        setContent {
            val themeViewModel: ThemeViewModel = viewModel(
                factory = ThemeViewModelFactory(userPreferencesRepository)
            )
            val themeConfig by themeViewModel.themeConfig.collectAsState()

            if (themeConfig != null) {
                // Force unwrapping is safe here because of the null check
                val currentTheme = themeConfig!!
                TracerTheme(themeConfig = currentTheme) {
                    Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.background) {
                        TracerScreen(
                            runtimeInitializer = runtimeInitializer,
                            recordGateway = recordGateway,
                            txtStorageGateway = txtStorageGateway,
                            reportGateway = reportGateway,
                            queryGateway = queryGateway,
                            controller = runtimeGateway,
                            userPreferencesRepository = userPreferencesRepository,
                            themeConfig = currentTheme,
                            onSetThemeColor = themeViewModel::setThemeColor,
                            onSetThemeMode = themeViewModel::setThemeMode,
                            onSetUseDynamicColor = themeViewModel::setUseDynamicColor,
                            onSetDarkThemeStyle = themeViewModel::setDarkThemeStyle
                        )
                    }
                }
            } else {
                // Show a placeholder (e.g., system background) while loading preferences
                // This prevents the "flash" of default theme color
                Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.background) {
                    // Empty or a loading indicator could go here
                }
            }
        }
    }
}
