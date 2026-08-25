package com.example.tracer

import androidx.annotation.StringRes
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.BarChart
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Folder
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material.icons.outlined.BarChart
import androidx.compose.material.icons.outlined.Edit
import androidx.compose.material.icons.outlined.Folder
import androidx.compose.material.icons.outlined.Settings
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SnackbarDuration
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.feature.data.R as DataFeatureR
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.ThemeConfig
import com.example.tracer.ui.viewmodel.ThemeEvent

internal enum class TracerTab {
    INSIGHTS,
    RECORD,
    FILES,
    CONFIG
}

internal val DefaultTracerTab: TracerTab = TracerTab.RECORD

internal data class TabMeta(
    val id: TracerTab,
    @param:StringRes val titleRes: Int,
    val selectedIcon: ImageVector,
    val unselectedIcon: ImageVector,
    val testTag: String? = null
)

internal enum class TracerTabScrollBehavior {
    NONE,
    VERTICAL
}

internal data class TracerTabRouteArgs(
    val dataViewModel: DataViewModel,
    val queryUiState: QueryInsightsUiState,
    val queryInsightsViewModel: QueryInsightsViewModel,
    val txtStorageGateway: TxtStorageGateway,
    val recordUiState: RecordUiState,
    val recordViewModel: RecordViewModel,
    val themeConfig: ThemeConfig,
    val onThemeEvent: (ThemeEvent) -> Unit,
    val insightsPiePalettePreset: InsightsPiePalettePreset,
    val onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
    val insightsComparisonColorScheme: InsightsComparisonColorScheme,
    val onInsightsComparisonColorSchemeChange: (InsightsComparisonColorScheme) -> Unit,
    val insightsComparisonIndicatorStyle: InsightsComparisonIndicatorStyle,
    val onInsightsComparisonIndicatorStyleChange: (InsightsComparisonIndicatorStyle) -> Unit,
    val insightsChartShowAverageLine: Boolean,
    val onInsightsChartShowAverageLineChange: (Boolean) -> Unit,
    val insightsChartSemanticMode: InsightsChartSemanticMode,
    val onInsightsChartSemanticModeChange: (InsightsChartSemanticMode) -> Unit,
    val insightsChartVisualMode: InsightsChartVisualMode,
    val onInsightsChartVisualModeChange: (InsightsChartVisualMode) -> Unit,
    val insightsChartTrendRoot: String,
    val onInsightsChartTrendRootChange: (String) -> Unit,
    val insightsAverageDayBasis: InsightsAverageDayBasis,
    val onInsightsAverageDayBasisChange: (InsightsAverageDayBasis) -> Unit,
    val insightsMode: InsightsMode,
    val onInsightsModeChange: (InsightsMode) -> Unit,
    val insightsResultDisplayMode: InsightsResultDisplayMode,
    val onInsightsResultDisplayModeChange: (InsightsResultDisplayMode) -> Unit,
    val insightsParameterSection: InsightsParameterSection,
    val onInsightsParameterSectionChange: (InsightsParameterSection) -> Unit,
    val insightsDayActivitiesView: InsightsActivityView,
    val insightsPeriodActivitiesView: InsightsActivityView,
    val onInsightsDayActivitiesViewChange: (InsightsActivityView) -> Unit,
    val onInsightsPeriodActivitiesViewChange: (InsightsActivityView) -> Unit,
    val insightsTimeParametersExpanded: Boolean,
    val onInsightsTimeParametersExpandedChange: (Boolean) -> Unit,
    val insightsHeatmapTomlConfig: InsightsHeatmapTomlConfig,
    val insightsHeatmapStylePreference: InsightsHeatmapStylePreference,
    val onInsightsHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    val onInsightsHeatmapPaletteNameChange: (String) -> Unit,
    val insightsHeatmapApplyMessage: String,
    val isAppDarkThemeActive: Boolean,
    val appLanguage: AppLanguage,
    val onSetAppLanguage: (AppLanguage) -> Unit,
    val configCardExpansionPreferences: com.example.tracer.data.ConfigCardExpansionPreferences,
    val onPersistConfigCardExpanded: (com.example.tracer.data.ConfigCard, Boolean) -> Unit,
    val promptBeforeUnconfiguredActivityRecord: Boolean,
    val onPromptBeforeUnconfiguredActivityRecordChange: (Boolean) -> Unit,
    val pageTransitionsEnabled: Boolean,
    val onPageTransitionsEnabledChange: (Boolean) -> Unit,
    val pageTransitionStyle: com.example.tracer.data.PageTransitionStyle,
    val onPageTransitionStyleChange: (com.example.tracer.data.PageTransitionStyle) -> Unit,
    val validAuthorableEventTokens: Set<String>,
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onClearQuickAccessCache: () -> Unit,
    val onPersistRecordQuickAccessCardExpanded: (Boolean) -> Unit,
    val onPersistRecordQuickAccessEditorVisibility: (Boolean) -> Unit,
    val onPersistRecordCanonicalCatalogDisplayMode: (RecordFrequentOutputMode) -> Unit,
    val onPersistRecordCanonicalCatalogSource: (CanonicalCatalogSource) -> Unit,
    val onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    val onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    val onPersistRecordFrequentLookbackDays: (Int) -> Unit,
    val onPersistRecordFrequentOutputMode: (RecordFrequentOutputMode) -> Unit,
    val onPersistRecordFrequentTopN: (Int) -> Unit,
    val activityCategoriesContent: @Composable () -> Unit,
    val onImportDataFolder: () -> Unit,
    val onImportSingleTracer: () -> Unit,
    val onExportAllMonthsTracer: () -> Unit,
    val onExportCurrentTxtTracer: () -> Unit,
    val isTracerExportInProgress: Boolean,
    val selectedTracerSecurityLevel: TracerExchangeSecurityLevel,
    val onTracerSecurityLevelChange: (TracerExchangeSecurityLevel) -> Unit,
    val onCopyDiagnosticsPayload: () -> Unit,
    val onEditDailyStatuses: () -> Unit
)

internal data class TracerTabLifecycleArgs(
    val queryGateway: QueryGateway,
    val queryInsightsViewModel: QueryInsightsViewModel,
    val recordViewModel: RecordViewModel,
    val recordStatusText: () -> String,
    val onValidAuthorableEventTokensChanged: (Set<String>) -> Unit
)

internal data class TracerTabStatusArgs(
    val dataStatusText: String,
    val queryStatusText: String,
    val recordStatusText: String
)

internal data class TracerTabEntry(
    val meta: TabMeta,
    val scrollBehavior: TracerTabScrollBehavior,
    val onEnter: suspend (TracerTabLifecycleArgs) -> Unit = {},
    val onLeave: (TracerTabLifecycleArgs) -> Unit = {},
    val statusText: (TracerTabStatusArgs) -> String = { "" },
    val statusEvent: (TracerTabStatusEventArgs) -> TracerTabUiEvent? = { null },
    val content: @Composable (modifier: Modifier, args: TracerTabRouteArgs) -> Unit
)

internal object TracerTabRegistry {
    private val entriesInDefinitionOrder: List<TracerTabEntry> = listOf(
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.INSIGHTS,
                titleRes = R.string.tracer_tab_insights,
                selectedIcon = Icons.Filled.BarChart,
                unselectedIcon = Icons.Outlined.BarChart,
                testTag = "tab_insights"
            ),
            scrollBehavior = TracerTabScrollBehavior.NONE,
            // The ViewModel defers its first refresh until the insights presentation preferences
            // are available, preventing the default Text query from racing the persisted Chart.
            onEnter = { args -> args.queryInsightsViewModel.onInsightsTabEntered() },
            statusText = { args -> args.queryStatusText },
            statusEvent = { args ->
                if (args.statusText.startsWith("query data ", ignoreCase = true) ||
                    // Markdown insights output is rendered directly in the Insights result card.
                    // Showing the nativeInsightsJson progress/result text as a global snackbar
                    // duplicates that feedback and obscures the floating navigation.
                    args.statusText.startsWith("nativeInsightsJson(", ignoreCase = true)) {
                    null
                } else {
                    defaultStatusUiEvent(args)
                }
            },
            content = { modifier, args ->
                QueryInsightsTabContent(
                    modifier = modifier,
                    queryUiState = args.queryUiState,
                    queryInsightsViewModel = args.queryInsightsViewModel,
                    preferredInsightsMode = args.insightsMode,
                    onPreferredInsightsModeChange = args.onInsightsModeChange,
                    preferredResultDisplayMode = args.insightsResultDisplayMode,
                    onPreferredResultDisplayModeChange = args.onInsightsResultDisplayModeChange,
                    preferredParameterSection = args.insightsParameterSection,
                    onPreferredParameterSectionChange = args.onInsightsParameterSectionChange,
                    dayActivitiesView = args.insightsDayActivitiesView,
                    periodActivitiesView = args.insightsPeriodActivitiesView,
                    onDayActivitiesViewChange = args.onInsightsDayActivitiesViewChange,
                    onPeriodActivitiesViewChange = args.onInsightsPeriodActivitiesViewChange,
                    timeParametersExpanded = args.insightsTimeParametersExpanded,
                    onTimeParametersExpandedChange = args.onInsightsTimeParametersExpandedChange,
                    preferredChartSemanticMode = args.insightsChartSemanticMode,
                    onPreferredChartSemanticModeChange = args.onInsightsChartSemanticModeChange,
                    preferredChartVisualMode = args.insightsChartVisualMode,
                    onPreferredChartVisualModeChange = args.onInsightsChartVisualModeChange,
                    preferredTrendChartRoot = args.insightsChartTrendRoot,
                    onPreferredTrendChartRootChange = args.onInsightsChartTrendRootChange,
                    preferredAverageDayBasis = args.insightsAverageDayBasis,
                    chartShowAverageLine = args.insightsChartShowAverageLine,
                    piePalettePreset = args.insightsPiePalettePreset,
                    comparisonColorScheme = args.insightsComparisonColorScheme,
                    comparisonIndicatorStyle = args.insightsComparisonIndicatorStyle,
                    onChartShowAverageLineChange = args.onInsightsChartShowAverageLineChange,
                    heatmapTomlConfig = args.insightsHeatmapTomlConfig,
                    heatmapStylePreference = args.insightsHeatmapStylePreference,
                    onHeatmapThemePolicyChange = args.onInsightsHeatmapThemePolicyChange,
                    onHeatmapPaletteNameChange = args.onInsightsHeatmapPaletteNameChange,
                    heatmapApplyMessage = args.insightsHeatmapApplyMessage,
                    isAppDarkThemeActive = args.isAppDarkThemeActive,
                    onEditDailyStatuses = args.onEditDailyStatuses,
                    bottomContentPadding = floatingBottomNavScrollPadding()
                )
            }
        ),
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.RECORD,
                titleRes = R.string.tracer_tab_record,
                selectedIcon = Icons.Filled.Edit,
                unselectedIcon = Icons.Outlined.Edit,
                testTag = "tab_record"
            ),
            scrollBehavior = TracerTabScrollBehavior.VERTICAL,
            // Do not clear logical-day override on tab leave.
            // Yesterday/today is shared session state across Record and the Config-embedded TXT
            // editor so users keep one target-day intent while switching views.
            onEnter = { args ->
                refreshRecordMappingValidation(args)
                // Refresh only updates when no user override is active.
                args.recordViewModel.refreshLogicalDayDefault()
            },
            statusText = { args -> args.recordStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { _, args ->
                RecordTabContent(
                    recordUiState = args.recordUiState,
                    recordViewModel = args.recordViewModel,
                    txtStorageGateway = args.txtStorageGateway,
                    promptBeforeUnconfiguredActivityRecord = args.promptBeforeUnconfiguredActivityRecord,
                    validAuthorableEventTokens = args.validAuthorableEventTokens,
                    onPersistQuickActivities = args.onPersistRecordQuickActivities,
                    onPersistQuickAccessCardExpanded =
                        args.onPersistRecordQuickAccessCardExpanded,
                    onPersistQuickAccessEditorVisibility = args.onPersistRecordQuickAccessEditorVisibility,
                    onPersistCanonicalCatalogDisplayMode =
                        args.onPersistRecordCanonicalCatalogDisplayMode,
                    onPersistCanonicalCatalogSource = args.onPersistRecordCanonicalCatalogSource,
                    onPersistCollapsedCanonicalRootPaths =
                        args.onPersistRecordCollapsedCanonicalRootPaths,
                    onPersistOrderedCanonicalRootPaths =
                        args.onPersistRecordOrderedCanonicalRootPaths,
                    onPersistFrequentLookbackDays = args.onPersistRecordFrequentLookbackDays,
                    onPersistFrequentOutputMode = args.onPersistRecordFrequentOutputMode,
                    onPersistFrequentTopN = args.onPersistRecordFrequentTopN,
                    categoriesContent = args.activityCategoriesContent
                )
            }
        ),
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.FILES,
                titleRes = R.string.tracer_tab_files,
                selectedIcon = Icons.Filled.Folder,
                unselectedIcon = Icons.Outlined.Folder,
                testTag = "tab_files"
            ),
            scrollBehavior = TracerTabScrollBehavior.VERTICAL,
            onEnter = { args ->
                args.recordViewModel.loadCanonicalCatalogForExternalSelection()
            },
            onLeave = { args -> args.recordViewModel.discardUnsavedHistoryDraft() },
            statusText = { args -> args.recordStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { modifier, args ->
                Column(
                    modifier = modifier,
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    TxtEditorRouteContent(args)
                }
            }
        ),
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.CONFIG,
                titleRes = R.string.tracer_tab_config,
                selectedIcon = Icons.Filled.Settings,
                unselectedIcon = Icons.Outlined.Settings,
                testTag = "tab_config"
            ),
            scrollBehavior = TracerTabScrollBehavior.VERTICAL,
            onEnter = { args ->
                refreshRecordMappingValidation(args)
            },
            statusText = { args -> args.dataStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { modifier, args ->
                Column(
                    modifier = modifier,
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    ConfigSection(
                        themeConfig = args.themeConfig,
                        onCopyDiagnosticsPayload = args.onCopyDiagnosticsPayload,
                        onThemeEvent = args.onThemeEvent,
                        insightsPiePalettePreset = args.insightsPiePalettePreset,
                        onInsightsPiePalettePresetChange = args.onInsightsPiePalettePresetChange,
                        insightsComparisonColorScheme = args.insightsComparisonColorScheme,
                        onInsightsComparisonColorSchemeChange = args.onInsightsComparisonColorSchemeChange,
                        insightsComparisonIndicatorStyle = args.insightsComparisonIndicatorStyle,
                        onInsightsComparisonIndicatorStyleChange =
                            args.onInsightsComparisonIndicatorStyleChange,
                        insightsAverageDayBasis = args.insightsAverageDayBasis,
                        onInsightsAverageDayBasisChange = args.onInsightsAverageDayBasisChange,
                        appLanguage = args.appLanguage,
                        onSetAppLanguage = args.onSetAppLanguage,
                        promptBeforeUnconfiguredActivityRecord =
                            args.promptBeforeUnconfiguredActivityRecord,
                        onPromptBeforeUnconfiguredActivityRecordChange =
                            args.onPromptBeforeUnconfiguredActivityRecordChange,
                        pageTransitionsEnabled = args.pageTransitionsEnabled,
                        onPageTransitionsEnabledChange = args.onPageTransitionsEnabledChange,
                        pageTransitionStyle = args.pageTransitionStyle,
                        onPageTransitionStyleChange = args.onPageTransitionStyleChange,
                        cardExpansionPreferences = args.configCardExpansionPreferences,
                        onConfigCardExpandedChange = args.onPersistConfigCardExpanded,
                        extraContent = {
                            DataManagementRouteContent(args)
                        }
                    )
                }
            }
        )
    )

    private val entryByTab: Map<TracerTab, TracerTabEntry> =
        entriesInDefinitionOrder.associateBy { it.meta.id }

    // The registry order is the left-to-right order of the floating bottom navigation.
    val entries: List<TracerTabEntry> = listOf(
        TracerTab.FILES,
        TracerTab.INSIGHTS,
        TracerTab.RECORD,
        TracerTab.CONFIG
    ).map(entryByTab::getValue)

    fun entry(tab: TracerTab): TracerTabEntry = entryByTab.getValue(tab)

    fun indexOf(tab: TracerTab): Int = entries.indexOfFirst { it.meta.id == tab }

    suspend fun onEnter(tab: TracerTab, args: TracerTabLifecycleArgs) {
        entry(tab).onEnter(args)
    }

    fun onLeave(tab: TracerTab, args: TracerTabLifecycleArgs) {
        entry(tab).onLeave(args)
    }

    fun statusText(tab: TracerTab, args: TracerTabStatusArgs): String {
        return entry(tab).statusText(args)
    }

    fun statusEvent(tab: TracerTab, args: TracerTabStatusEventArgs): TracerTabUiEvent? {
        return entry(tab).statusEvent(args)
    }
}

@Composable
private fun DataManagementRouteContent(args: TracerTabRouteArgs) {
    val clearTxtStatusText = DestructiveActionStatusText(
        running = stringResource(DataFeatureR.string.data_status_clear_txt_running),
        success = stringResource(DataFeatureR.string.data_status_clear_txt_success),
        failure = stringResource(DataFeatureR.string.data_status_clear_txt_failure)
    )
    val clearDatabaseStatusText = DestructiveActionStatusText(
        running = stringResource(DataFeatureR.string.data_status_clear_database_running),
        success = stringResource(DataFeatureR.string.data_status_clear_database_success),
        failure = stringResource(DataFeatureR.string.data_status_clear_database_failure)
    )
    val rebuildDatabaseStatusText = DestructiveActionStatusText(
        running = stringResource(DataFeatureR.string.data_status_rebuild_database_running),
        success = stringResource(DataFeatureR.string.data_status_rebuild_database_success),
        failure = stringResource(DataFeatureR.string.data_status_rebuild_database_failure)
    )
    val clearAllDataStatusText = DestructiveActionStatusText(
        running = stringResource(DataFeatureR.string.data_status_clear_all_data_running),
        success = stringResource(DataFeatureR.string.data_status_clear_all_data_success),
        failure = stringResource(DataFeatureR.string.data_status_clear_all_data_failure)
    )
    DataManagementSection(
        onImportDataFolder = args.onImportDataFolder,
        onImportSingleTracer = args.onImportSingleTracer,
        // Export performs its own authoritative TXT inspection when started.
        // It is available directly from Config > Data Management.
        canExportAllMonthsTracer = true,
        canExportCurrentTxtTracer = true,
        onExportAllMonthsTracer = args.onExportAllMonthsTracer,
        onExportCurrentTxtTracer = args.onExportCurrentTxtTracer,
        isTracerExportInProgress = args.isTracerExportInProgress,
        selectedTracerSecurityLevel = args.selectedTracerSecurityLevel,
        onTracerSecurityLevelChange = args.onTracerSecurityLevelChange,
        showCryptoProgress = args.recordUiState.cryptoProgress.isVisible,
        cryptoProgressTitle = args.recordUiState.cryptoProgress.operationText,
        cryptoProgressPhase = args.recordUiState.cryptoProgress.phaseText,
        cryptoOverallProgress = args.recordUiState.cryptoProgress.overallProgress,
        cryptoOverallText = args.recordUiState.cryptoProgress.overallText,
        cryptoDetailsText = args.recordUiState.cryptoProgress.detailsText,
        cryptoAdvancedDetailsText = args.recordUiState.cryptoProgress.advancedDetailsText,
        expanded = args.configCardExpansionPreferences.dataManagementExpanded,
        onToggleExpanded = {
            args.onPersistConfigCardExpanded(
                com.example.tracer.data.ConfigCard.DATA_MANAGEMENT,
                !args.configCardExpansionPreferences.dataManagementExpanded
            )
        },
        onClearTxt = {
            args.dataViewModel.clearTxt(clearTxtStatusText)
            args.recordViewModel.clearTxtEditorState()
        },
        onClearDatabase = {
            args.dataViewModel.clearDatabase(clearDatabaseStatusText)
        },
        onRebuildDatabase = {
            args.dataViewModel.rebuildDatabase(rebuildDatabaseStatusText)
        },
        onClearData = {
            args.onClearQuickAccessCache()
            args.dataViewModel.clearDataAndReinitialize(clearAllDataStatusText)
        }
    )
}

@Composable
private fun TxtEditorRouteContent(args: TracerTabRouteArgs) {
    TxtEditorSection(
        txtStorageGateway = args.txtStorageGateway,
        canonicalCatalogRoots = args.recordUiState.canonicalCatalogRoots,
        isCanonicalCatalogLoading = args.recordUiState.isCanonicalCatalogLoading,
        canonicalCatalogStatusText = args.recordUiState.canonicalCatalogStatusText,
        onCanonicalCatalogRequested = args.recordViewModel::loadCanonicalCatalogForExternalSelection,
        collapsedCanonicalRootPaths = args.recordUiState.collapsedCanonicalRootPaths,
        orderedCanonicalRootPaths = args.recordUiState.orderedCanonicalRootPaths,
        onCollapsedCanonicalRootPathsChange = args.recordViewModel::updateCollapsedCanonicalRootPaths,
        onOrderedCanonicalRootPathsChange = args.recordViewModel::updateOrderedCanonicalRootPaths,
        inspectionEntries = args.recordUiState.txtInspectionEntries,
        availableMonths = args.recordUiState.availableMonths,
        selectedMonth = args.recordUiState.selectedMonth,
        logicalDayTarget = args.recordUiState.logicalDayTarget,
        txtHistoryLoaded = args.recordUiState.txtHistoryLoaded,
        initialDayMarker = args.recordUiState.txtDayMarker,
        logicalDayClock = args.recordViewModel.logicalDayClock,
        onOpenPreviousMonth = args.recordViewModel::openPreviousMonth,
        onOpenNextMonth = args.recordViewModel::openNextMonth,
        onOpenMonth = args.recordViewModel::openMonth,
        selectedHistoryFile = args.recordUiState.selectedHistoryFile,
        selectedHistoryContent = args.recordUiState.selectedHistoryContent,
        onRefreshHistory = args.recordViewModel::refreshHistory,
        editableHistoryContent = args.recordUiState.editableHistoryContent,
        onEditableHistoryContentChange = args.recordViewModel::updateEditableHistoryContent,
        onDayMarkerPersist = args.recordViewModel::onTxtDayMarkerChange,
        onSaveHistoryFile = args.recordViewModel::saveHistoryFileAndSync,
        onSaveHistoryRepresentationOnly = args.recordViewModel::saveHistoryFileRepresentationOnly,
        embeddedInScrollableParent = true,
        inlineStatusText = args.recordUiState.statusText,
        onCreateCurrentMonthTxt = args.recordViewModel::createCurrentMonthTxt
    )
}

private const val ActivityAuthorableTokenValidationUnavailablePrefix =
    "Activity authorable token validation unavailable:"

private suspend fun refreshRecordMappingValidation(args: TracerTabLifecycleArgs) {
    val mappingResult = args.queryGateway.listAuthorableEventTokens()
    if (mappingResult.ok) {
        args.onValidAuthorableEventTokensChanged(mappingResult.names.toSet())
        if (args.recordStatusText().startsWith(ActivityAuthorableTokenValidationUnavailablePrefix)) {
            args.recordViewModel.setStatusText("")
        }
        return
    }

    args.onValidAuthorableEventTokensChanged(emptySet())
    args.recordViewModel.setStatusText(
        "$ActivityAuthorableTokenValidationUnavailablePrefix ${mappingResult.message}"
    )
}

private fun defaultStatusUiEvent(args: TracerTabStatusEventArgs): TracerTabUiEvent? {
    if (!args.isTabUnchanged || !args.hasStatusUpdate) {
        return null
    }

    val isFailureStatus = args.statusText.contains("fail", ignoreCase = true) ||
        args.statusText.contains("error", ignoreCase = true)
    val visuals = if (!isFailureStatus && args.selectedTab == TracerTab.RECORD) {
        buildStructuredRecordSnackbarVisuals(args.statusText)
            ?: defaultSnackbarVisuals(
                message = args.statusText,
                duration = SnackbarDuration.Short
            )
    } else {
        defaultSnackbarVisuals(
            message = args.statusText,
            duration = if (isFailureStatus) SnackbarDuration.Long else SnackbarDuration.Short
        )
    }
    return TracerTabUiEvent.ShowSnackbar(
        visuals = visuals
    )
}

private fun defaultSnackbarVisuals(
    message: String,
    duration: SnackbarDuration
): TracerSnackbarVisuals = TracerSnackbarVisuals(
    message = message,
    duration = duration,
    withDismissAction = true
)

private fun buildStructuredRecordSnackbarVisuals(
    statusText: String
): TracerSnackbarVisuals? {
    // Record success intentionally uses exactly two lines: activity first, duration second.
    // Parsing that shape once here keeps the rest of the snackbar pipeline structured and
    // avoids scattering Record-specific newline assumptions across the UI host.
    val lines = statusText.lineSequence()
        .map { it.trim() }
        .filter { it.isNotEmpty() }
        .toList()
    if (lines.size < 2) {
        return null
    }
    return TracerSnackbarVisuals(
        message = lines[0],
        supportingText = lines.drop(1).joinToString("\n"),
        duration = SnackbarDuration.Short,
        withDismissAction = true
    )
}
