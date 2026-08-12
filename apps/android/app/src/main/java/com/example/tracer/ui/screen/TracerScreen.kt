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
import java.util.Locale

@Composable
fun TracerScreen(
    runtimeInitializer: RuntimeInitializer,
    recordGateway: RecordGateway,
    txtStorageGateway: TxtStorageGateway,
    insightsGateway: InsightsGateway,
    queryGateway: QueryGateway,
    configGateway: ConfigGateway,
    quickAccessGateway: QuickAccessGateway,
    activityHierarchyGateway: ActivityHierarchyGateway,
    activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway,
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
    var isDailyStatusEditorVisible by rememberSaveable { mutableStateOf(false) }
    var validAuthorableEventTokens by remember { mutableStateOf<Set<String>>(emptySet()) }
    val coroutineScope = rememberCoroutineScope()
    val context = LocalContext.current
    val dataViewModel: DataViewModel = viewModel(
        factory = remember(runtimeInitializer, recordGateway) {
            DataViewModelFactory(runtimeInitializer, recordGateway)
        }
    )
    val queryInsightsViewModel: QueryInsightsViewModel = viewModel(
        factory = remember(insightsGateway, queryGateway, recordGateway, context) {
            QueryInsightsViewModelFactory(
                insightsGateway = insightsGateway,
                queryGateway = queryGateway,
                recordGateway = recordGateway,
                textProvider = AndroidQueryInsightsTextProvider(context)
            )
        }
    )
    LaunchedEffect(appLanguage) {
        queryInsightsViewModel.onInsightsLocaleChange(
            when (appLanguage) {
                com.example.tracer.data.AppLanguage.System -> when (Locale.getDefault().language) {
                    "zh" -> "zh"
                    "ja" -> "ja"
                    else -> "en"
                }
                com.example.tracer.data.AppLanguage.Chinese -> "zh"
                com.example.tracer.data.AppLanguage.English -> "en"
                com.example.tracer.data.AppLanguage.Japanese -> "ja"
            }
        )
    }
    val recordInputPersistence = remember(userPreferencesRepository) {
        UserPreferencesRecordInputPersistence(userPreferencesRepository)
    }
    val quickActivitiesPreferenceGateway = remember(quickAccessGateway) {
        RuntimeQuickActivitiesGateway(quickAccessGateway)
    }
    val quickActivities by quickActivitiesPreferenceGateway.quickActivities.collectAsState()
    LaunchedEffect(quickAccessGateway) {
        runCatching { quickActivitiesPreferenceGateway.getQuickActivities() }
    }
    val recordViewModel: RecordViewModel = viewModel(
        factory = remember(
            recordGateway,
            txtStorageGateway,
            insightsGateway,
            queryGateway,
            recordInputPersistence
        ) {
            RecordViewModelFactory(
                recordGateway = recordGateway,
                txtStorageGateway = txtStorageGateway,
                insightsGateway = insightsGateway,
                queryGateway = queryGateway,
                recordInputPersistence = recordInputPersistence,
                textProvider = AndroidRecordTextProvider(context)
            )
        }
    )
    val activityHierarchyEditorViewModel: ActivityHierarchyEditorViewModel = viewModel(
        factory = remember(
            configGateway,
            activityHierarchyGateway,
            activityHierarchyMigrationGateway,
            quickActivitiesPreferenceGateway
        ) {
            ActivityHierarchyEditorViewModelFactory(
                configGateway = configGateway,
                activityHierarchyGateway = activityHierarchyGateway,
                activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
                quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
            )
        }
    )

    val dataUiState = dataViewModel.uiState
    val queryUiState = queryInsightsViewModel.uiState
    val recordUiState = recordViewModel.uiState
    val activityHierarchyEditorState = activityHierarchyEditorViewModel.uiState
    val recordFrequentPreferences by userPreferencesRepository.recordFrequentPreferences.collectAsState(
        initial = com.example.tracer.data.RecordFrequentPreferences(
            lookbackDays = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_FREQUENT_LOOKBACK_DAYS,
            topN = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_FREQUENT_TOP_N,
            outputMode = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_FREQUENT_OUTPUT_MODE,
            canonicalCatalogDisplayMode =
                com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE,
            quickActivities = emptyList(),
            quickAccessCardExpanded = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_QUICK_ACCESS_CARD_EXPANDED,
            quickAccessEditorVisible = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_QUICK_ACCESS_EDITOR_VISIBLE,
            collapsedCanonicalRootPaths = com.example.tracer.data.UserPreferencesRepository.DEFAULT_COLLAPSED_CANONICAL_ROOT_PATHS,
            orderedCanonicalRootPaths = com.example.tracer.data.UserPreferencesRepository.DEFAULT_ORDERED_CANONICAL_ROOT_PATHS
        )
    )
    val insightsChartShowAverageLine by userPreferencesRepository.insightsChartShowAverageLine.collectAsState(
        initial = com.example.tracer.data.UserPreferencesRepository.DEFAULT_INSIGHTS_CHART_SHOW_AVERAGE_LINE
    )
    val insightsChartSemanticMode by userPreferencesRepository.insightsChartSemanticMode.collectAsState(
        initial = null
    )
    val insightsChartVisualMode by userPreferencesRepository.insightsChartVisualMode.collectAsState(
        initial = null
    )
    val insightsChartTrendRoot by userPreferencesRepository.insightsChartTrendRoot.collectAsState(
        initial = null
    )
    val insightsAverageDayBasis by userPreferencesRepository.insightsAverageDayBasis.collectAsState(
        initial = null
    )
    val insightsMode by userPreferencesRepository.insightsMode.collectAsState(initial = null)
    val insightsResultDisplayMode by userPreferencesRepository.insightsResultDisplayMode.collectAsState(
        initial = null
    )
    val insightsParameterSection by userPreferencesRepository.insightsParameterSection.collectAsState(
        initial = null
    )
    val insightsTimeParametersExpanded by userPreferencesRepository.insightsTimeParametersExpanded
        .collectAsState(initial = null)
    val persistedRecordInput by userPreferencesRepository.recordPersistedInput.collectAsState(
        initial = null as PersistedRecordInputSnapshot?
    )
    val insightsPiePalettePreset by userPreferencesRepository.insightsPiePalettePreset.collectAsState(
        initial = null
    )
    val configCardExpansionPreferences by userPreferencesRepository.configCardExpansionPreferences
        .collectAsState(initial = null)
    val insightsHeatmapState = rememberTracerScreenInsightsHeatmapState(
        selectedTab = selectedTab,
        configGateway = configGateway,
        userPreferencesRepository = userPreferencesRepository
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
        insightsChartSemanticMode == null ||
        insightsChartVisualMode == null ||
        insightsChartTrendRoot == null ||
        insightsMode == null ||
        insightsResultDisplayMode == null ||
        insightsParameterSection == null ||
        insightsTimeParametersExpanded == null
        || insightsAverageDayBasis == null || insightsPiePalettePreset == null ||
        configCardExpansionPreferences == null
    ) {
        return
    }

    val loadedPersistedRecordInput = requireNotNull(persistedRecordInput)
    val loadedInsightsChartSemanticMode = requireNotNull(insightsChartSemanticMode)
    val loadedInsightsChartVisualMode = requireNotNull(insightsChartVisualMode)
    val loadedInsightsChartTrendRoot = requireNotNull(insightsChartTrendRoot)
    val loadedInsightsMode = requireNotNull(insightsMode)
    val loadedInsightsResultDisplayMode = requireNotNull(insightsResultDisplayMode)
    val loadedInsightsParameterSection = requireNotNull(insightsParameterSection)
    val loadedInsightsTimeParametersExpanded = requireNotNull(insightsTimeParametersExpanded)
    val loadedInsightsAverageDayBasis = requireNotNull(insightsAverageDayBasis)
    val loadedInsightsPiePalettePreset = requireNotNull(insightsPiePalettePreset)
    val loadedConfigCardExpansionPreferences = requireNotNull(configCardExpansionPreferences)

    val displayedRecordUiState = if (!recordViewModel.hasAppliedInitialPersistedRecordInputForUi) {
        recordUiState.copy(
            authoringMode = loadedPersistedRecordInput.lastAuthoringMode,
            txtOutputMode = loadedPersistedRecordInput.lastTxtOutputMode
        )
    } else {
        recordUiState
    }

    SyncTracerScreenRecordPreferences(
        recordFrequentPreferences = recordFrequentPreferences,
        quickActivities = quickActivities,
        persistedRecordInput = persistedRecordInput,
        recordViewModel = recordViewModel
    )

    LaunchedEffect(activityHierarchyEditorState.txtReloadRequestVersion) {
        if (activityHierarchyEditorState.txtReloadRequestVersion == 0L) {
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
            recordStatusText = recordUiState.statusText
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
        activityHierarchyEditorViewModel = activityHierarchyEditorViewModel,
        onQuickAccessReload = {
            val importedQuickActivities = runCatching {
                quickActivitiesPreferenceGateway.getQuickActivities()
            }.getOrElse {
                quickActivitiesPreferenceGateway.clearCachedQuickActivities()
                emptyList()
            }
            recordViewModel.updateQuickActivities(importedQuickActivities)
        }
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
            queryInsightsViewModel = queryInsightsViewModel,
            recordViewModel = recordViewModel,
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
        dataViewModel = dataViewModel,
        userPreferencesRepository = userPreferencesRepository,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
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
        queryInsightsViewModel = queryInsightsViewModel,
        txtStorageGateway = txtStorageGateway,
        recordUiState = displayedRecordUiState,
        recordViewModel = recordViewModel,
        themeConfig = themeConfig,
        onThemeEvent = onThemeEvent,
        insightsPiePalettePreset = loadedInsightsPiePalettePreset,
        onInsightsPiePalettePresetChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsPiePalettePreset(value)
            }
        },
        insightsChartShowAverageLine = insightsChartShowAverageLine,
        onInsightsChartShowAverageLineChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsChartShowAverageLine(value)
            }
        },
        insightsChartSemanticMode = loadedInsightsChartSemanticMode,
        onInsightsChartSemanticModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsChartSemanticMode(value)
            }
        },
        insightsChartVisualMode = loadedInsightsChartVisualMode,
        onInsightsChartVisualModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsChartVisualMode(value)
            }
        },
        insightsChartTrendRoot = loadedInsightsChartTrendRoot,
        onInsightsChartTrendRootChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsChartTrendRoot(value)
            }
        },
        insightsAverageDayBasis = loadedInsightsAverageDayBasis,
        onInsightsAverageDayBasisChange = { value ->
            coroutineScope.launch { userPreferencesRepository.setInsightsAverageDayBasis(value) }
        },
        insightsMode = loadedInsightsMode,
        onInsightsModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsMode(value)
            }
        },
        insightsResultDisplayMode = loadedInsightsResultDisplayMode,
        onInsightsResultDisplayModeChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsResultDisplayMode(value)
            }
        },
        insightsParameterSection = loadedInsightsParameterSection,
        onInsightsParameterSectionChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsParameterSection(value)
            }
        },
        insightsTimeParametersExpanded = loadedInsightsTimeParametersExpanded,
        onInsightsTimeParametersExpandedChange = { value ->
            coroutineScope.launch {
                userPreferencesRepository.setInsightsTimeParametersExpanded(value)
            }
        },
        insightsHeatmapTomlConfig = insightsHeatmapState.config,
        insightsHeatmapStylePreference = insightsHeatmapState.stylePreference,
        onInsightsHeatmapThemePolicyChange = insightsHeatmapState.onThemePolicyChange,
        onInsightsHeatmapPaletteNameChange = insightsHeatmapState.onPaletteNameChange,
        insightsHeatmapApplyMessage = insightsHeatmapState.applyMessage,
        isAppDarkThemeActive = isAppDarkThemeActive,
        appLanguage = appLanguage,
        onSetAppLanguage = onSetAppLanguage,
        configCardExpansionPreferences = loadedConfigCardExpansionPreferences,
        onPersistConfigCardExpanded = actions.onPersistConfigCardExpanded,
        validAuthorableEventTokens = validAuthorableEventTokens,
        onPersistRecordQuickActivities = actions.onPersistRecordQuickActivities,
        onClearQuickAccessCache = quickActivitiesPreferenceGateway::clearCachedQuickActivities,
        onPersistRecordQuickAccessCardExpanded =
            actions.onPersistRecordQuickAccessCardExpanded,
        onPersistRecordQuickAccessEditorVisibility = actions.onPersistRecordQuickAccessEditorVisibility,
        onPersistRecordCanonicalCatalogDisplayMode =
            actions.onPersistRecordCanonicalCatalogDisplayMode,
        onPersistRecordCollapsedCanonicalRootPaths =
            actions.onPersistRecordCollapsedCanonicalRootPaths,
        onPersistRecordOrderedCanonicalRootPaths =
            actions.onPersistRecordOrderedCanonicalRootPaths,
        onPersistRecordFrequentLookbackDays = actions.onPersistRecordFrequentLookbackDays,
        onPersistRecordFrequentOutputMode = actions.onPersistRecordFrequentOutputMode,
        onPersistRecordFrequentTopN = actions.onPersistRecordFrequentTopN,
        activityCategoriesContent = {
            ActivityHierarchyEditorContent(
                state = activityHierarchyEditorState,
                viewModel = activityHierarchyEditorViewModel
            )
        },
        onImportDataFolder = importDataFolderAction,
        onImportSingleTracer = importSingleTracerAction,
        onExportAllMonthsTracer = exportActions.onExportAllMonthsTracer,
        onExportCurrentTxtTracer = exportActions.onExportCurrentTxtTracer,
        isTracerExportInProgress = exportActions.isTracerExportInProgress,
        selectedTracerSecurityLevel = exportActions.selectedTracerSecurityLevel,
        onTracerSecurityLevelChange = exportActions.onTracerSecurityLevelChange,
        onCopyDiagnosticsPayload = actions.onCopyDiagnosticsPayload,
        onEditDailyStatuses = { isDailyStatusEditorVisible = true }
    )

    if (isDailyStatusEditorVisible) {
        DailyStatusEditorDialog(
            userPreferencesRepository = userPreferencesRepository,
            runtimeInitializer = runtimeInitializer,
            insightsMode = queryUiState.insightsMode,
            statusValues = queryUiState.statusValues,
            recordUiState = recordUiState,
            recordViewModel = recordViewModel,
            onConfigSaved = queryInsightsViewModel::insightsCurrentSelection,
            onDismissRequest = { isDailyStatusEditorVisible = false }
        )
    }
}

private val tracerTabSaver = Saver<TracerTab, String>(
    save = { tab -> tab.name },
    restore = { name ->
        runCatching { TracerTab.valueOf(name) }.getOrDefault(DefaultTracerTab)
    }
)
