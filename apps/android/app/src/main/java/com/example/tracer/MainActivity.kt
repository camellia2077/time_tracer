package com.example.tracer

import android.app.LocaleManager
import android.os.Build
import android.os.LocaleList
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.Modifier
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.ReportPiePaletteTomlLoader
import com.example.tracer.ui.theme.TracerTheme
import com.example.tracer.ui.viewmodel.ThemeViewModel
import com.example.tracer.ui.viewmodel.ThemeViewModelFactory

import androidx.activity.enableEdgeToEdge

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        title = getString(R.string.app_name)
        enableEdgeToEdge()
        val appContainer = (application as TracerApplication).appContainer
        val runtimeInitializer = appContainer.runtimeInitializer
        val recordGateway = appContainer.recordGateway
        val txtStorageGateway = appContainer.txtStorageGateway
        val reportGateway = appContainer.reportGateway
        val queryGateway = appContainer.queryGateway
        val configGateway = appContainer.configGateway
        val activityHierarchyGateway = appContainer.activityHierarchyGateway
        val activityHierarchyMigrationGateway = appContainer.activityHierarchyMigrationGateway
        val tracerExchangeGateway = appContainer.tracerExchangeGateway
        val userPreferencesRepository = appContainer.userPreferencesRepository
        ReportPiePaletteTomlLoader.installFromAssets(assets)

        setContent {
            val themeViewModel: ThemeViewModel = viewModel(
                factory = ThemeViewModelFactory(userPreferencesRepository)
            )
            val themeConfig by themeViewModel.themeConfig.collectAsState()
            val appLanguage by themeViewModel.appLanguage.collectAsState()

            LaunchedEffect(appLanguage) {
                applyAppLanguage(appLanguage)
            }

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
                            configGateway = configGateway,
                            quickAccessGateway = appContainer.quickAccessGateway,
                            activityHierarchyGateway = activityHierarchyGateway,
                            activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
                            tracerExchangeGateway = tracerExchangeGateway,
                            userPreferencesRepository = userPreferencesRepository,
                            themeConfig = currentTheme,
                            onThemeEvent = themeViewModel::onThemeEvent,
                            appLanguage = appLanguage,
                            onSetAppLanguage = themeViewModel::setAppLanguage
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

    private fun applyAppLanguage(language: AppLanguage) {
        val localeTag = when (language) {
            AppLanguage.System -> ""
            AppLanguage.Chinese -> "zh"
            AppLanguage.English -> "en"
            AppLanguage.Japanese -> "ja"
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            val localeManager = getSystemService(LocaleManager::class.java) ?: return
            if (localeManager.applicationLocales.toLanguageTags() == localeTag) {
                return
            }
            localeManager.applicationLocales = LocaleList.forLanguageTags(localeTag)
            return
        }

        val locales = if (language == AppLanguage.System) {
            LocaleList.getDefault()
        } else {
            LocaleList.forLanguageTags(localeTag)
        }
        val configuration = resources.configuration
        if (configuration.locales != locales) {
            configuration.setLocales(locales)
            @Suppress("DEPRECATION")
            resources.updateConfiguration(configuration, resources.displayMetrics)
            recreate()
        }
    }
}
