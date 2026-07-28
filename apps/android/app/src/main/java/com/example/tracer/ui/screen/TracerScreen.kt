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
import androidx.compose.runtime.saveable.Saver
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.platform.LocalContext
import androidx.lifecycle.Lifecycle
import androidx.lifecycle.LifecycleEventObserver
import androidx.lifecycle.compose.LocalLifecycleOwner
import androidx.lifecycle.viewmodel.compose.viewModel
import com.example.tracer.PersistedRecordInputSnapshot
import com.example.tracer.data.UserPreferencesRecordInputPersistence
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
    onThemeEvent: (com.example.tracer.ui.viewmodel.ThemeEvent) -> Unit,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit
) {
    var selectedTab by rememberSaveable(stateSaver = tracerTabSaver) {
        mutableStateOf(DefaultTracerTab)
    }
    var validAuthorableEventTokens by remember { mutableStateOf<Set<String>>(emptySet()) }
    val coroutineScope = rememberCoroutineScope()
    val context = LocalContext.current
    val dataViewModel: DataViewModel = viewModel(
        factory = remember(runtimeInitializer, recordGateway) {
            DataViewModelFactory(runtimeInitializer, recordGateway)
        }
    )
    val queryReportViewModel: QueryReportViewModel = viewModel(
        factory = remember(reportGateway, queryGateway, recordGateway, context) {
            QueryReportViewModelFactory(
                reportGateway = reportGateway,
                queryGateway = queryGateway,
                recordGateway = recordGateway,
                textProvider = AndroidQueryReportTextProvider(context)
            )
        }
    )
    LaunchedEffect(appLanguage) {
        queryReportViewModel.onReportLocaleChange(
            when (appLanguage) {
                com.example.tracer.data.AppLanguage.Chinese -> "zh"
                com.example.tracer.data.AppLanguage.English -> "en"
                com.example.tracer.data.AppLanguage.Japanese -> "ja"
            }
        )
    }
    val recordInputPersistence = remember(userPreferencesRepository) {
        UserPreferencesRecordInputPersistence(userPreferencesRepository)
    }
    val quickActivitiesPreferenceGateway = remember(userPreferencesRepository) {
        UserPreferencesQuickActivitiesGateway(userPreferencesRepository)
    }
    val recordViewModel: RecordViewModel = viewModel(
        factory = remember(recordGateway, txtStorageGateway, queryGateway, recordInputPersistence) {
            RecordViewModelFactory(
                recordGateway = recordGateway,
                txtStorageGateway = txtStorageGateway,
                queryGateway = queryGateway,
                recordInputPersistence = recordInputPersistence,
                textProvider = AndroidRecordTextProvider(context)
            )
        }
    )
    val configViewModel: ConfigViewModel = viewModel(
        factory = remember(configGateway, txtStorageGateway, quickActivitiesPreferenceGateway) {
            ConfigViewModelFactory(
                configGateway = configGateway,
                txtStorageGateway = txtStorageGateway,
                quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
            )
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
            outputMode = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_SUGGEST_OUTPUT_MODE,
            canonicalCatalogDisplayMode =
                com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE,
            quickActivities = emptyList(),
            quickAccessCardExpanded = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_QUICK_ACCESS_CARD_EXPANDED,
            assistSettingsExpanded = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_ASSIST_SETTINGS_EXPANDED,
            collapsedCanonicalRootPaths = com.example.tracer.data.UserPreferencesRepository.DEFAULT_COLLAPSED_CANONICAL_ROOT_PATHS,
            orderedCanonicalRootPaths = com.example.tracer.data.UserPreferencesRepository.DEFAULT_ORDERED_CANONICAL_ROOT_PATHS
        )
    )
    val reportChartShowAverageLine by userPreferencesRepository.reportChartShowAverageLine.collectAsState(
        initial = com.example.tracer.data.UserPreferencesRepository.DEFAULT_REPORT_CHART_SHOW_AVERAGE_LINE
    )
    val reportChartSemanticMode by userPreferencesRepository.reportChartSemanticMode.collectAsState(
        initial = null
    )
    val reportChartVisualMode by userPreferencesRepository.reportChartVisualMode.collectAsState(
        initial = null
    )
    val reportMode by userPreferencesRepository.reportMode.collectAsState(initial = null)
    val reportResultDisplayMode by userPreferencesRepository.reportResultDisplayMode.collectAsState(
        initial = null
    )
    val reportParameterSection by userPreferencesRepository.reportParameterSection.collectAsState(
        initial = null
    )
    val reportTimeParametersExpanded by userPreferencesRepository.reportTimeParametersExpanded
        .collectAsState(initial = null)
    val persistedRecordInput by userPreferencesRepository.recordPersistedInput.collectAsState(
        initial = null as PersistedRecordInputSnapshot?
    )
    val reportPiePalettePreset by userPreferencesRepository.reportPiePalettePreset.collectAsState(
        initial = com.example.tracer.data.UserPreferencesRepository.DEFAULT_REPORT_PIE_PALETTE_PRESET
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

    // Do not render preference-backed segmented controls with their defaults and then
    // hydrate them in a later frame. That state change is rendered by Material as a short
    // selection animation during cold start.
    if (persistedRecordInput == null ||
        reportChartSemanticMode == null ||
        reportChartVisualMode == null ||
        reportMode == null ||
        reportResultDisplayMode == null ||
        reportParameterSection == null ||
        reportTimeParametersExpanded == null
    ) {
        return
    }

    val loadedPersistedRecordInput = requireNotNull(persistedRecordInput)
    val loadedReportChartSemanticMode = requireNotNull(reportChartSemanticMode)
    val loadedReportChartVisualMode = requireNotNull(reportChartVisualMode)
    val loadedReportMode = requireNotNull(reportMode)
    val loadedReportResultDisplayMode = requireNotNull(reportResultDisplayMode)
    val loadedReportParameterSection = requireNotNull(reportParameterSection)
    val loadedReportTimeParametersExpanded = requireNotNull(reportTimeParametersExpanded)

    val displayedRecordUiState = if (!recordViewModel.hasAppliedInitialPersistedRecordInputForUi) {
        recordUiState.copy(
            authoringMode = loadedPersistedRecordInput.lastAuthoringMode,
            txtOutputMode = loadedPersistedRecordInput.lastTxtOutputMode
        )
    } else {
        recordUiState
    }

    SyncTracerScreenRecordPreferences(
        recordSuggestionPreferences = recordSuggestionPreferences,
        persistedRecordInput = persistedRecordInput,
        recordViewModel = recordViewModel
    )

    LaunchedEffect(configUiState.txtReloadRequestVersion) {
        if (configUiState.txtReloadRequestVersion == 0L) {
            return@LaunchedEffect
        }
        val selectedHistoryFile = recordViewModel.uiState.selectedHistoryFile
        if (selectedHistoryFile.isBlank()) {
            return@LaunchedEffect
        }
        // Reload the currently opened TXT from disk after a successful alias rename migration.
        // Without this refresh, the editor can keep showing pre-migration in-memory content,
        // which makes it look like the TXT alias replacement did not happen.
        recordViewModel.openHistoryFile(selectedHistoryFile)
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
    val snackbarHostState = remember { SnackbarHostState() }
    val exportActions = rememberTracerExportActions(
        context = context,
        coroutineScope = coroutineScope,
        recordUiState = displayedRecordUiState,
        dataViewModel = dataViewModel,
        txtStorageGateway = txtStorageGateway,
        configGateway = configGateway,
        tracerExchangeGateway = tracerExchangeGateway,
        recordViewModel = recordViewModel
    )
    val importDataFolderAction = rememberTracerDataFolderImportAction(
        context = context,
        coroutineScope = coroutineScope,
        recordViewModel = recordViewModel,
        dataViewModel = dataViewModel,
        configGateway = configGateway,
        configViewModel = configViewModel
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
            queryReportViewModel = queryReportViewModel,
            recordViewModel = recordViewModel,
            configViewModel = configViewModel,
            recordStatusText = { recordViewModel.uiState.statusText },
            onValidAuthorableEventTokensChanged = { names -> validAuthorableEventTokens = names }
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
        snackbarHostState = snackbarHostState,
        onCoordinatorEvent = actions.onCoordinatorEvent,
        dataViewModel = dataViewModel,
        queryUiState = queryUiState,
        queryReportViewModel = queryReportViewModel,
        txtStorageGateway = txtStorageGateway,
        recordUiState = displayedRecordUiState,
        recordViewModel = recordViewModel,
        configUiState = configUiState,
        configViewModel = configViewModel,
        themeConfig = themeConfig,
        onThemeEvent = onThemeEvent,
        reportPiePalettePreset = reportPiePalettePreset,
        onReportPiePalettePresetChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportPiePalettePreset(value)
            }
        },
        reportChartShowAverageLine = reportChartShowAverageLine,
        onReportChartShowAverageLineChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportChartShowAverageLine(value)
            }
        },
        reportChartSemanticMode = loadedReportChartSemanticMode,
        onReportChartSemanticModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportChartSemanticMode(value)
            }
        },
        reportChartVisualMode = loadedReportChartVisualMode,
        onReportChartVisualModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportChartVisualMode(value)
            }
        },
        reportMode = loadedReportMode,
        onReportModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportMode(value)
            }
        },
        reportResultDisplayMode = loadedReportResultDisplayMode,
        onReportResultDisplayModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportResultDisplayMode(value)
            }
        },
        reportParameterSection = loadedReportParameterSection,
        onReportParameterSectionChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportParameterSection(value)
            }
        },
        reportTimeParametersExpanded = loadedReportTimeParametersExpanded,
        onReportTimeParametersExpandedChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setReportTimeParametersExpanded(value)
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
        validAuthorableEventTokens = validAuthorableEventTokens,
        onPersistRecordQuickActivities = actions.onPersistRecordQuickActivities,
        onPersistRecordQuickAccessCardExpanded =
            actions.onPersistRecordQuickAccessCardExpanded,
        onPersistRecordAssistSettingsExpanded = actions.onPersistRecordAssistSettingsExpanded,
        onPersistRecordCanonicalCatalogDisplayMode =
            actions.onPersistRecordCanonicalCatalogDisplayMode,
        onPersistRecordCollapsedCanonicalRootPaths =
            actions.onPersistRecordCollapsedCanonicalRootPaths,
        onPersistRecordOrderedCanonicalRootPaths =
            actions.onPersistRecordOrderedCanonicalRootPaths,
        onPersistRecordSuggestLookbackDays = actions.onPersistRecordSuggestLookbackDays,
        onPersistRecordSuggestOutputMode = actions.onPersistRecordSuggestOutputMode,
        onPersistRecordSuggestTopN = actions.onPersistRecordSuggestTopN,
        onImportDataFolder = importDataFolderAction,
        onImportSingleTracer = importSingleTracerAction,
        onExportAllMonthsTracer = exportActions.onExportAllMonthsTracer,
        onExportCurrentTxtTracer = exportActions.onExportCurrentTxtTracer,
        isTracerExportInProgress = exportActions.isTracerExportInProgress,
        selectedTracerSecurityLevel = exportActions.selectedTracerSecurityLevel,
        onTracerSecurityLevelChange = exportActions.onTracerSecurityLevelChange,
        onCopyDiagnosticsPayload = actions.onCopyDiagnosticsPayload
    )
}

private val tracerTabSaver = Saver<TracerTab, String>(
    save = { tab -> tab.name },
    restore = { name ->
        runCatching { TracerTab.valueOf(name) }.getOrDefault(DefaultTracerTab)
    }
)
