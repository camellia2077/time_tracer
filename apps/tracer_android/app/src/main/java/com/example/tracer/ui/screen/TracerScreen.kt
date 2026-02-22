package com.example.tracer

import android.content.pm.ApplicationInfo
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
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
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.unit.dp
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
    val lifecycleOwner = LocalLifecycleOwner.current

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

    val statusText = statusTextForSelectedTab(
        selectedTab = selectedTab,
        dataStatusText = dataUiState.statusText,
        queryStatusText = queryUiState.statusText,
        recordStatusText = recordUiState.statusText,
        configStatusText = configUiState.statusText
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
        recordViewModel = recordViewModel,
        configViewModel = configViewModel
    )
    val importSingleTxtAction = rememberTracerSingleTxtImportAction(
        context = context,
        coroutineScope = coroutineScope,
        dataViewModel = dataViewModel
    )
    val configImportActions = rememberTracerAndroidConfigImportActions(
        context = context,
        coroutineScope = coroutineScope,
        configViewModel = configViewModel
    )

    StatusSnackbarEffect(
        selectedTab = selectedTab,
        statusText = statusText,
        snackbarHostState = snackbarHostState,
        onShowExportDetails = { selectedTab = TracerTab.TXT }
    )

    // Refresh tab-specific data when switching tabs.
    LaunchedEffect(selectedTab, controller) {
        if (selectedTab == TracerTab.CONFIG) {
            configViewModel.refreshConfigFiles()
        }
        if (selectedTab == TracerTab.RECORD || selectedTab == TracerTab.TXT) {
            val mappingResult = controller.listActivityMappingNames()
            if (mappingResult.ok) {
                validMappingNames = mappingResult.names.toSet()
                if (recordUiState.statusText.startsWith("Activity mapping validation unavailable:")) {
                    recordViewModel.setStatusText("")
                }
            } else {
                validMappingNames = emptySet()
                recordViewModel.setStatusText(
                    "Activity mapping validation unavailable: ${mappingResult.message}"
                )
            }
        }
    }

    DisposableEffect(lifecycleOwner, selectedTab) {
        val observer = LifecycleEventObserver { _, event ->
            if (event != Lifecycle.Event.ON_STOP) {
                return@LifecycleEventObserver
            }
            when (selectedTab) {
                TracerTab.TXT -> recordViewModel.discardUnsavedHistoryDraft()
                TracerTab.CONFIG -> configViewModel.discardUnsavedDraft()
                else -> Unit
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
            if (nextTab != selectedTab) {
                when (selectedTab) {
                    TracerTab.TXT -> recordViewModel.discardUnsavedHistoryDraft()
                    TracerTab.CONFIG -> configViewModel.discardUnsavedDraft()
                    else -> Unit
                }
                selectedTab = nextTab
            }
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
                appLanguage = appLanguage,
                onSetAppLanguage = onSetAppLanguage,
                validMappingNames = validMappingNames,
                userPreferencesRepository = userPreferencesRepository,
                coroutineScope = coroutineScope,
                onImportSingleTxt = importSingleTxtAction,
                onExportAllMonthsTxt = exportActions.onExportAllMonthsTxt,
                isTxtExportInProgress = exportActions.isTxtExportInProgress,
                onImportAndroidConfigFullReplace = configImportActions.onImportAndroidConfigFullReplace,
                onImportAndroidConfigPartialUpdate = configImportActions.onImportAndroidConfigPartialUpdate,
                onExportAndroidConfigBundle = exportActions.onExportAndroidConfigBundle
            )

            QueryResultDisplay(
                selectedTab = selectedTab,
                resultDisplayMode = queryUiState.resultDisplayMode,
                activeResult = queryUiState.activeResult,
                analysisError = queryUiState.analysisError,
                chartRoots = queryUiState.chartRoots,
                chartSelectedRoot = queryUiState.chartSelectedRoot,
                chartDateInputMode = queryUiState.chartDateInputMode,
                chartLookbackDays = queryUiState.chartLookbackDays,
                chartRangeStartDate = queryUiState.chartRangeStartDate,
                chartRangeEndDate = queryUiState.chartRangeEndDate,
                chartLoading = queryUiState.chartLoading,
                chartError = queryUiState.chartError,
                chartPoints = queryUiState.chartPoints,
                chartAverageDurationSeconds = queryUiState.chartAverageDurationSeconds,
                chartUsesLegacyStatsFallback = queryUiState.chartUsesLegacyStatsFallback,
                chartShowAverageLine = reportChartShowAverageLine,
                onChartRootChange = queryReportViewModel::onChartRootChange,
                onChartDateInputModeChange = queryReportViewModel::onChartDateInputModeChange,
                onChartLookbackDaysChange = queryReportViewModel::onChartLookbackDaysChange,
                onChartRangeStartDateChange = queryReportViewModel::onChartRangeStartDateChange,
                onChartRangeEndDateChange = queryReportViewModel::onChartRangeEndDateChange,
                onChartShowAverageLineChange = { value ->
                    coroutineScope.launch {
                        userPreferencesRepository.setReportChartShowAverageLine(value)
                    }
                },
                onLoadChart = queryReportViewModel::loadChart
            )
        }
    }
}
