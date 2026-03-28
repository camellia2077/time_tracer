package com.example.tracer

import android.content.pm.ApplicationInfo
import androidx.compose.foundation.isSystemInDarkTheme
import androidx.compose.material3.SnackbarHostState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.DisposableEffect
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.viewmodel.compose.viewModel
import kotlinx.coroutines.launch

@Composable
fun TracerScreen(
    runtimeInitializer: RuntimeInitializer,
    recordGateway: RecordGateway,
    txtStorageGateway: TxtStorageGateway,
    reportGateway: ReportGateway,
    queryGateway: QueryGateway,
    configGateway: ConfigGateway,
    tracerExchangeGateway: TracerExchangeGateway,
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
        factory = remember(configGateway) {
            ConfigViewModelFactory(configGateway)
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
    val reportHeatmapState = rememberTracerScreenReportHeatmapState(
        selectedTab = selectedTab,
        configGateway = configGateway
    )
    val lifecycleOwner = LocalLifecycleOwner.current
    val isSystemDark = isSystemInDarkTheme()
    val isAppDarkThemeActive = when (themeConfig.themeMode) {
        com.example.tracer.data.ThemeMode.Dark -> true
        com.example.tracer.data.ThemeMode.Light -> false
        com.example.tracer.data.ThemeMode.System -> isSystemDark
    }

    SyncTracerScreenRecordPreferences(
        recordSuggestionPreferences = recordSuggestionPreferences,
        recordViewModel = recordViewModel
    )

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
        tracerExchangeGateway = tracerExchangeGateway,
        recordViewModel = recordViewModel
    )
    val importSingleTxtAction = rememberTracerTxtFolderImportAction(
        context = context,
        coroutineScope = coroutineScope,
        dataViewModel = dataViewModel,
        recordViewModel = recordViewModel
    )
    val importSingleTracerAction = rememberTracerSingleTracerImportAction(
        context = context,
        coroutineScope = coroutineScope,
        dataViewModel = dataViewModel,
        recordViewModel = recordViewModel,
        tracerExchangeGateway = tracerExchangeGateway
    )
    val tabLifecycleArgs = {
        TracerTabLifecycleArgs(
            queryGateway = queryGateway,
            recordViewModel = recordViewModel,
            configViewModel = configViewModel,
            recordStatusText = { recordViewModel.uiState.statusText },
            onValidMappingNamesChanged = { names -> validMappingNames = names }
        )
    }
    val actions = rememberTracerScreenActions(
        selectedTab = selectedTab,
        tabLifecycleArgs = tabLifecycleArgs,
        onTabChanged = { nextTab -> selectedTab = nextTab },
        coroutineScope = coroutineScope,
        configGateway = configGateway,
        configViewModel = configViewModel,
        userPreferencesRepository = userPreferencesRepository
    )

    StatusSnackbarEffect(
        selectedTab = selectedTab,
        statusText = statusText,
        snackbarHostState = snackbarHostState,
        onCoordinatorEvent = actions.onCoordinatorEvent
    )

    LaunchedEffect(selectedTab, queryGateway) {
        TracerTabRegistry.onEnter(selectedTab, tabLifecycleArgs())
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

    TracerScreenContent(
        selectedTab = selectedTab,
        statusText = statusText,
        isDebuggableBuild = isDebuggableBuild,
        snackbarHostState = snackbarHostState,
        onCoordinatorEvent = actions.onCoordinatorEvent,
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
        reportHeatmapTomlConfig = reportHeatmapState.config,
        reportHeatmapStylePreference = reportHeatmapState.stylePreference,
        onReportHeatmapThemePolicyChange = reportHeatmapState.onThemePolicyChange,
        onReportHeatmapPaletteNameChange = reportHeatmapState.onPaletteNameChange,
        reportHeatmapApplyMessage = reportHeatmapState.applyMessage,
        isAppDarkThemeActive = isAppDarkThemeActive,
        appLanguage = appLanguage,
        onSetAppLanguage = onSetAppLanguage,
        validMappingNames = validMappingNames,
        onPersistRecordQuickActivities = actions.onPersistRecordQuickActivities,
        onPersistRecordAssistExpanded = actions.onPersistRecordAssistExpanded,
        onPersistRecordAssistSettingsExpanded = actions.onPersistRecordAssistSettingsExpanded,
        onPersistRecordSuggestLookbackDays = actions.onPersistRecordSuggestLookbackDays,
        onPersistRecordSuggestTopN = actions.onPersistRecordSuggestTopN,
        onImportSingleTxt = importSingleTxtAction,
        onImportSingleTracer = importSingleTracerAction,
        onExportAllMonthsTracer = exportActions.onExportAllMonthsTracer,
        isTracerExportInProgress = exportActions.isTracerExportInProgress,
        selectedTracerSecurityLevel = exportActions.selectedTracerSecurityLevel,
        onTracerSecurityLevelChange = exportActions.onTracerSecurityLevelChange,
        onCopyDiagnosticsPayload = actions.onCopyDiagnosticsPayload,
        onClearDatabase = dataViewModel::clearDatabase
    )
}
