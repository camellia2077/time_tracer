package com.example.tracer

import android.app.ActivityManager
import android.app.LocaleManager
import android.graphics.Color as AndroidColor
import android.os.Build
import android.os.LocaleList
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.SideEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.luminance
import androidx.compose.ui.platform.LocalView
import androidx.compose.ui.Modifier
import androidx.core.view.WindowCompat
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.InsightsPiePaletteTomlLoader
import com.example.tracer.ui.theme.TracerTheme
import com.example.tracer.ui.viewmodel.ThemeViewModel
import com.example.tracer.ui.viewmodel.ThemeViewModelFactory

import androidx.activity.enableEdgeToEdge

private const val DARK_SYSTEM_BAR_ICON_LUMINANCE_THRESHOLD = 0.5f

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        applySystemTaskLabel()
        enableEdgeToEdge()
        val appContainer = (application as TracerApplication).appContainer
        val runtimeInitializer = appContainer.runtimeInitializer
        val recordGateway = appContainer.recordGateway
        val txtStorageGateway = appContainer.txtStorageGateway
        val insightsGateway = appContainer.insightsGateway
        val queryGateway = appContainer.queryGateway
        val configGateway = appContainer.configGateway
        val activityHierarchyGateway = appContainer.activityHierarchyGateway
        val activityHierarchyMigrationGateway = appContainer.activityHierarchyMigrationGateway
        val tracerExchangeGateway = appContainer.tracerExchangeGateway
        val userPreferencesRepository = appContainer.userPreferencesRepository
        InsightsPiePaletteTomlLoader.installFromAssets(assets)

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
                    SynchronizeSystemBarsWithTheme()
                    Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.background) {
                        TracerScreen(
                            runtimeInitializer = runtimeInitializer,
                            recordGateway = recordGateway,
                            txtStorageGateway = txtStorageGateway,
                            insightsGateway = insightsGateway,
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

    /**
     * Keep the task-card label explicit as well as the manifest label.
     *
     * Some OEM launchers populate a newly installed app's first recent-task card
     * before they refresh ActivityInfo.  Supplying the task description during
     * the earliest activity lifecycle point avoids their package-name fallback.
     */
    private fun applySystemTaskLabel() {
        val appLabel = getString(R.string.app_name)
        title = appLabel
        val taskDescription = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
            ActivityManager.TaskDescription.Builder()
                .setLabel(appLabel)
                .build()
        } else {
            @Suppress("DEPRECATION")
            ActivityManager.TaskDescription(appLabel)
        }
        setTaskDescription(taskDescription)
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

@Composable
private fun MainActivity.SynchronizeSystemBarsWithTheme() {
    val view = LocalView.current
    val background = MaterialTheme.colorScheme.background
    val useDarkSystemBarIcons = shouldUseDarkSystemBarIcons(background)

    SideEffect {
        window.statusBarColor = AndroidColor.TRANSPARENT
        window.navigationBarColor = AndroidColor.TRANSPARENT
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
            window.isStatusBarContrastEnforced = false
            window.isNavigationBarContrastEnforced = false
        }
        WindowCompat.getInsetsController(window, view).apply {
            isAppearanceLightStatusBars = useDarkSystemBarIcons
            isAppearanceLightNavigationBars = useDarkSystemBarIcons
        }
    }
}

internal fun shouldUseDarkSystemBarIcons(background: Color): Boolean =
    background.luminance() > DARK_SYSTEM_BAR_ICON_LUMINANCE_THRESHOLD
