package com.example.tracer

import android.content.pm.ApplicationInfo
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.platform.LocalClipboardManager
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.text.AnnotatedString
import androidx.compose.ui.unit.dp
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.viewmodel.compose.viewModel
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

@Composable
fun TracerScreen(
    runtimeInitializer: RuntimeInitializer,
    recordGateway: RecordGateway,
    txtStorageGateway: TxtStorageGateway,
    reportGateway: ReportGateway,
    queryGateway: QueryGateway,
    controller: RuntimeGateway,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit
) {
    var selectedTab by remember { mutableStateOf(DefaultTracerTab) }
    var validMappingNames by remember { mutableStateOf<Set<String>>(emptySet()) }
    val coroutineScope = rememberCoroutineScope()
    val context = LocalContext.current
    val clipboardManager = LocalClipboardManager.current
    val diagnosticsPrepareText = stringResource(R.string.tracer_diagnostics_prepare)
    val dataViewModel: DataViewModel = viewModel(
        factory = remember(runtimeInitializer, recordGateway) {
            DataViewModelFactory(runtimeInitializer, recordGateway)
        }
    )
    val queryReportViewModel: QueryReportViewModel = viewModel(
        factory = remember(reportGateway, queryGateway, context) {
            QueryReportViewModelFactory(
                reportGateway = reportGateway,
                queryGateway = queryGateway,
                textProvider = AndroidQueryReportTextProvider(context)
            )
        }
    )
    val recordViewModel: RecordViewModel = viewModel(
        factory = remember(recordGateway, txtStorageGateway, queryGateway) {
            RecordViewModelFactory(recordGateway, txtStorageGateway, queryGateway)
        }
    )
    val configViewModel: ConfigViewModel = viewModel(
        factory = remember(controller) {
            ConfigViewModelFactory(controller)
        }
    )

    val dataUiState = dataViewModel.uiState
    val queryUiState = queryReportViewModel.uiState
    val recordUiState = recordViewModel.uiState
    val configUiState = configViewModel.uiState
    val recordSuggestionPreferences by userPreferencesRepository.recordSuggestionPreferences.collectAsState(
        initial = com.example.tracer.data.RecordSuggestionPreferences(
            lookbackDays = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_SUGGEST_LOOKBACK_DAYS,
            topN = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_SUGGEST_TOP_N,
            quickActivities = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_QUICK_ACTIVITIES,
            assistExpanded = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_ASSIST_EXPANDED,
            assistSettingsExpanded = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_ASSIST_SETTINGS_EXPANDED
        )
    )
    val reportChartShowAverageLine by userPreferencesRepository.reportChartShowAverageLine.collectAsState(
        initial = com.example.tracer.data.UserPreferencesRepository.DEFAULT_REPORT_CHART_SHOW_AVERAGE_LINE
    )
    var reportHeatmapTomlConfig by remember { mutableStateOf(defaultReportHeatmapTomlConfig()) }
    var reportHeatmapStylePreference by remember {
        mutableStateOf(
            ReportHeatmapTomlLoader.deriveStylePreference(reportHeatmapTomlConfig)
        )
    }
    var reportHeatmapApplyMessage by remember { mutableStateOf("") }
    var reportHeatmapAutoApplyJob by remember { mutableStateOf<Job?>(null) }
    val lifecycleOwner = LocalLifecycleOwner.current
    val isSystemDark = isSystemInDarkTheme()
    val isAppDarkThemeActive = when (themeConfig.themeMode) {
        com.example.tracer.data.ThemeMode.Dark -> true
        com.example.tracer.data.ThemeMode.Light -> false
        com.example.tracer.data.ThemeMode.System -> isSystemDark
    }

    LaunchedEffect(
        recordSuggestionPreferences.lookbackDays,
        recordSuggestionPreferences.topN,
        recordSuggestionPreferences.quickActivities,
        recordSuggestionPreferences.assistExpanded,
        recordSuggestionPreferences.assistSettingsExpanded
    ) {
        recordViewModel.updateSuggestionPreferences(
            lookbackDays = recordSuggestionPreferences.lookbackDays,
            topN = recordSuggestionPreferences.topN
        )
        recordViewModel.updateQuickActivities(recordSuggestionPreferences.quickActivities)
        recordViewModel.updateAssistUiState(
            assistExpanded = recordSuggestionPreferences.assistExpanded,
            assistSettingsExpanded = recordSuggestionPreferences.assistSettingsExpanded
        )
    }

    val statusText = TracerTabRegistry.statusText(
        tab = selectedTab,
        args = TracerTabStatusArgs(
            dataStatusText = dataUiState.statusText,
            queryStatusText = queryUiState.statusText,
            recordStatusText = recordUiState.statusText,
            configStatusText = configUiState.statusText
        )
    )
    val isDebuggableBuild = remember(context) {
        (context.applicationInfo.flags and ApplicationInfo.FLAG_DEBUGGABLE) != 0
    }
    val snackbarHostState = remember { SnackbarHostState() }
    val exportActions = rememberTracerExportActions(
        context = context,
        coroutineScope = coroutineScope,
        recordUiState = recordUiState,
        txtStorageGateway = txtStorageGateway,
        cryptoGateway = controller,
        recordViewModel = recordViewModel,
        configViewModel = configViewModel
    )
    val importSingleTxtAction = rememberTracerSingleTxtImportAction(
        context = context,
        coroutineScope = coroutineScope,
        dataViewModel = dataViewModel,
        recordViewModel = recordViewModel
    )
    val importFolderTracerAction = rememberTracerFolderTracerImportAction(
        context = context,
        coroutineScope = coroutineScope,
        dataViewModel = dataViewModel,
        recordViewModel = recordViewModel,
        cryptoGateway = controller
    )
    val importSingleTracerAction = rememberTracerSingleTracerImportAction(
        context = context,
        coroutineScope = coroutineScope,
        dataViewModel = dataViewModel,
        recordViewModel = recordViewModel,
        cryptoGateway = controller
    )
    val configImportActions = rememberTracerAndroidConfigImportActions(
        context = context,
        coroutineScope = coroutineScope,
        configViewModel = configViewModel
    )
    val onCopyDiagnosticsPayload: () -> Unit = {
        coroutineScope.launch {
            configViewModel.setStatusText(diagnosticsPrepareText)
            val payloadResult = withContext(Dispatchers.IO) {
                controller.buildDiagnosticsPayload(maxEntries = 50)
            }
            if (!payloadResult.ok || payloadResult.payload.isBlank()) {
                configViewModel.setStatusText(payloadResult.message)
                return@launch
            }

            clipboardManager.setText(AnnotatedString(payloadResult.payload))
            configViewModel.setStatusText(payloadResult.message)
        }
    }

    val tabLifecycleArgs = {
        buildTracerTabLifecycleArgs(
            controller = controller,
            recordViewModel = recordViewModel,
            configViewModel = configViewModel,
            onValidMappingNamesChanged = { names -> validMappingNames = names }
        )
    }
    val onCoordinatorEvent: (TracerCoordinatorEvent) -> Unit = { event ->
        dispatchTracerCoordinatorEvent(
            event = event,
            selectedTab = selectedTab,
            tabLifecycleArgs = tabLifecycleArgs,
            onTabChanged = { nextTab -> selectedTab = nextTab }
        )
    }

    StatusSnackbarEffect(
        selectedTab = selectedTab,
        statusText = statusText,
        snackbarHostState = snackbarHostState,
        onCoordinatorEvent = onCoordinatorEvent
    )

    LaunchedEffect(selectedTab, controller) {
        TracerTabRegistry.onEnter(selectedTab, tabLifecycleArgs())
    }

    LaunchedEffect(selectedTab) {
        if (selectedTab == TracerTab.REPORT) {
            val loadedConfig = withContext(Dispatchers.IO) {
                ReportHeatmapTomlLoader.load(controller)
            }
            reportHeatmapTomlConfig = loadedConfig
            reportHeatmapStylePreference =
                ReportHeatmapTomlLoader.deriveStylePreference(loadedConfig)
            reportHeatmapApplyMessage = ""
        }
    }

    fun scheduleHeatmapTomlAutoApply(nextStyle: ReportHeatmapStylePreference) {
        val targetConfig = reportHeatmapTomlConfig
        reportHeatmapAutoApplyJob?.cancel()
        reportHeatmapApplyMessage = ""
        reportHeatmapAutoApplyJob = coroutineScope.launch {
            val applyResult = withContext(Dispatchers.IO) {
                applyHeatmapStyleToToml(
                    controller = controller,
                    config = targetConfig,
                    style = nextStyle
                )
            }
            if (!applyResult.ok) {
                reportHeatmapApplyMessage = applyResult.message
                return@launch
            }
            reportHeatmapApplyMessage = ""
            if (applyResult.updatedConfig != null) {
                reportHeatmapTomlConfig = applyResult.updatedConfig
            }
            if (applyResult.updatedStyle != null) {
                reportHeatmapStylePreference = applyResult.updatedStyle
            }
        }
    }

    DisposableEffect(lifecycleOwner, selectedTab) {
        val observer = LifecycleEventObserver { _, event ->
            if (event == Lifecycle.Event.ON_STOP) {
                TracerTabRegistry.onLeave(selectedTab, tabLifecycleArgs())
            }
        }
        lifecycleOwner.lifecycle.addObserver(observer)
        onDispose {
            lifecycleOwner.lifecycle.removeObserver(observer)
        }
    }

    TracerBottomNavShell(
        selectedTab = selectedTab,
        onTabSelected = { nextTab ->
            onCoordinatorEvent(TracerCoordinatorEvent.SelectTab(nextTab))
        },
        snackbarHostState = snackbarHostState
    ) { innerPadding ->
        val tabContentModifier = Modifier.tracerTabContentModifier(selectedTab, innerPadding)

        Column(
            modifier = tabContentModifier,
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            if (isDebuggableBuild && selectedTab != TracerTab.TXT && statusText.isNotBlank()) {
                Text(
                    text = statusText,
                    style = MaterialTheme.typography.bodySmall,
                    modifier = Modifier.fillMaxWidth()
                )
            }

            TracerTabRouteContent(
                modifier = if (selectedTab == TracerTab.DATA) Modifier.weight(1f) else Modifier,
                selectedTab = selectedTab,
                dataViewModel = dataViewModel,
                queryUiState = queryUiState,
                queryReportViewModel = queryReportViewModel,
                recordUiState = recordUiState,
                recordViewModel = recordViewModel,
                configUiState = configUiState,
                configViewModel = configViewModel,
                themeConfig = themeConfig,
                onSetThemeColor = onSetThemeColor,
                onSetThemeMode = onSetThemeMode,
                onSetUseDynamicColor = onSetUseDynamicColor,
                onSetDarkThemeStyle = onSetDarkThemeStyle,
                reportChartShowAverageLine = reportChartShowAverageLine,
                onReportChartShowAverageLineChange = { value ->
                    coroutineScope.launch {
                        userPreferencesRepository.setReportChartShowAverageLine(value)
                    }
                },
                reportHeatmapTomlConfig = reportHeatmapTomlConfig,
                reportHeatmapStylePreference = reportHeatmapStylePreference,
                onReportHeatmapThemePolicyChange = { value ->
                    val nextStyle = when (value) {
                        ReportHeatmapThemePolicy.FOLLOW_SYSTEM -> {
                            ReportHeatmapStylePreference(
                                themePolicy = ReportHeatmapThemePolicy.FOLLOW_SYSTEM,
                                paletteName = ""
                            )
                        }

                        ReportHeatmapThemePolicy.PALETTE -> {
                            reportHeatmapStylePreference.copy(
                                themePolicy = ReportHeatmapThemePolicy.PALETTE
                            )
                        }
                    }
                    if (nextStyle != reportHeatmapStylePreference) {
                        reportHeatmapStylePreference = nextStyle
                        scheduleHeatmapTomlAutoApply(nextStyle)
                    }
                },
                onReportHeatmapPaletteNameChange = { value ->
                    val nextStyle = reportHeatmapStylePreference.copy(
                        paletteName = value.trim()
                    )
                    if (nextStyle != reportHeatmapStylePreference) {
                        reportHeatmapStylePreference = nextStyle
                        scheduleHeatmapTomlAutoApply(nextStyle)
                    }
                },
                reportHeatmapApplyMessage = reportHeatmapApplyMessage,
                isAppDarkThemeActive = isAppDarkThemeActive,
                appLanguage = appLanguage,
                onSetAppLanguage = onSetAppLanguage,
                validMappingNames = validMappingNames,
                onPersistRecordQuickActivities = { activities ->
                    coroutineScope.launch {
                        userPreferencesRepository.setRecordQuickActivities(activities)
                    }
                },
                onPersistRecordAssistExpanded = { value ->
                    coroutineScope.launch {
                        userPreferencesRepository.setRecordAssistExpanded(value)
                    }
                },
                onPersistRecordAssistSettingsExpanded = { value ->
                    coroutineScope.launch {
                        userPreferencesRepository.setRecordAssistSettingsExpanded(value)
                    }
                },
                onPersistRecordSuggestLookbackDays = { value ->
                    coroutineScope.launch {
                        userPreferencesRepository.setRecordSuggestLookbackDays(value)
                    }
                },
                onPersistRecordSuggestTopN = { value ->
                    coroutineScope.launch {
                        userPreferencesRepository.setRecordSuggestTopN(value)
                    }
                },
                onImportFolderTracer = importFolderTracerAction,
                onImportSingleTxt = importSingleTxtAction,
                onImportSingleTracer = importSingleTracerAction,
                onExportAllMonthsTxt = exportActions.onExportAllMonthsTxt,
                onExportAllMonthsTracer = exportActions.onExportAllMonthsTracer,
                isTxtExportInProgress = exportActions.isTxtExportInProgress,
                isTracerExportInProgress = exportActions.isTracerExportInProgress,
                selectedTracerSecurityLevel = exportActions.selectedTracerSecurityLevel,
                onTracerSecurityLevelChange = exportActions.onTracerSecurityLevelChange,
                onImportAndroidConfigFullReplace = configImportActions.onImportAndroidConfigFullReplace,
                onImportAndroidConfigPartialUpdate = configImportActions.onImportAndroidConfigPartialUpdate,
                onExportAndroidConfigBundle = exportActions.onExportAndroidConfigBundle,
                onCopyDiagnosticsPayload = onCopyDiagnosticsPayload
            )
        }
    }
}

private data class HeatmapTomlApplyResult(
    val ok: Boolean,
    val message: String,
    val updatedConfig: ReportHeatmapTomlConfig? = null,
    val updatedStyle: ReportHeatmapStylePreference? = null
)

private suspend fun applyHeatmapStyleToToml(
    controller: RuntimeGateway,
    config: ReportHeatmapTomlConfig,
    style: ReportHeatmapStylePreference
): HeatmapTomlApplyResult {
    val configPath = ReportHeatmapTomlLoader.configPath()
    val readResult = controller.readConfigTomlFile(configPath)
    if (!readResult.ok) {
        return HeatmapTomlApplyResult(
            ok = false,
            message = readResult.message.ifBlank {
                "Apply failed: cannot read $configPath."
            }
        )
    }

    val (nextLightPalette, nextDarkPalette) = ReportHeatmapTomlLoader
        .resolveDefaultPalettesForStyle(
            config = config,
            stylePreference = style
        )
    val rewrittenContent = ReportHeatmapTomlLoader.rewriteDefaults(
        rawToml = readResult.content,
        lightPalette = nextLightPalette,
        darkPalette = nextDarkPalette
    )
    if (rewrittenContent == null) {
        return HeatmapTomlApplyResult(
            ok = false,
            message = "Apply failed: missing [defaults] section in $configPath."
        )
    }

    val saveResult = controller.saveConfigTomlFile(configPath, rewrittenContent)
    if (!saveResult.ok) {
        return HeatmapTomlApplyResult(
            ok = false,
            message = saveResult.message.ifBlank {
                "Apply failed: cannot save $configPath."
            }
        )
    }

    val updatedConfig = ReportHeatmapTomlLoader.parse(rewrittenContent)
    return HeatmapTomlApplyResult(
        ok = true,
        message = saveResult.message.ifBlank {
            "Applied heatmap style to $configPath."
        },
        updatedConfig = updatedConfig,
        updatedStyle = ReportHeatmapTomlLoader.deriveStylePreference(updatedConfig)
    )
}
