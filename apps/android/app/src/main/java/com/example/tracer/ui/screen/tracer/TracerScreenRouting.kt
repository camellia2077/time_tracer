package com.example.tracer

import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
internal fun Modifier.tracerTabContentModifier(
    selectedTab: TracerTab,
    innerPadding: PaddingValues
): Modifier {
    val bottomOuterPadding = if (selectedTab == TracerTab.RECORD) 0.dp else ScreenOuterPadding
    val baseModifier = this
        .fillMaxSize()
        // TracerBottomNavShell already provides the status-bar inset through innerPadding.
        // Insights consumes it below its fixed Day/Week/Month header; the other tabs do not
        // have that header and must not add a second top inset that becomes empty space.
        .padding(innerPadding)
        .padding(
            start = ScreenOuterPadding,
            end = ScreenOuterPadding,
            bottom = bottomOuterPadding
        )
    val selectedEntry = TracerTabRegistry.entry(selectedTab)
    return if (selectedEntry.scrollBehavior == TracerTabScrollBehavior.VERTICAL) {
        // Apply the navigation clearance after verticalScroll so it becomes part of the
        // scrollable child rather than a fixed shell inset. The former lets the final Chart
        // Result (and other vertical-tab content) move above the floating bar; the latter
        // leaves a permanently visible root-Surface strip below the bar.
        baseModifier
            .verticalScroll(rememberScrollState())
            .padding(bottom = floatingBottomNavScrollPadding())
    } else {
        baseModifier
    }
}

@Composable
internal fun TracerTabRouteContent(
    modifier: Modifier = Modifier,
    selectedTab: TracerTab,
    dataViewModel: DataViewModel,
    queryUiState: QueryInsightsUiState,
    queryInsightsViewModel: QueryInsightsViewModel,
    txtStorageGateway: TxtStorageGateway,
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onThemeEvent: (com.example.tracer.ui.viewmodel.ThemeEvent) -> Unit,
    insightsPiePalettePreset: InsightsPiePalettePreset,
    onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
    insightsChartShowAverageLine: Boolean,
    onInsightsChartShowAverageLineChange: (Boolean) -> Unit,
    insightsChartSemanticMode: InsightsChartSemanticMode,
    onInsightsChartSemanticModeChange: (InsightsChartSemanticMode) -> Unit,
    insightsChartVisualMode: InsightsChartVisualMode,
    onInsightsChartVisualModeChange: (InsightsChartVisualMode) -> Unit,
    insightsChartTrendRoot: String,
    onInsightsChartTrendRootChange: (String) -> Unit,
    insightsAverageDayBasis: InsightsAverageDayBasis,
    onInsightsAverageDayBasisChange: (InsightsAverageDayBasis) -> Unit,
    insightsMode: InsightsMode,
    onInsightsModeChange: (InsightsMode) -> Unit,
    insightsResultDisplayMode: InsightsResultDisplayMode,
    onInsightsResultDisplayModeChange: (InsightsResultDisplayMode) -> Unit,
    insightsParameterSection: InsightsParameterSection,
    onInsightsParameterSectionChange: (InsightsParameterSection) -> Unit,
    insightsDayActivitiesView: InsightsActivityView,
    insightsPeriodActivitiesView: InsightsActivityView,
    onInsightsDayActivitiesViewChange: (InsightsActivityView) -> Unit,
    onInsightsPeriodActivitiesViewChange: (InsightsActivityView) -> Unit,
    insightsTimeParametersExpanded: Boolean,
    onInsightsTimeParametersExpandedChange: (Boolean) -> Unit,
    insightsHeatmapTomlConfig: InsightsHeatmapTomlConfig,
    insightsHeatmapStylePreference: InsightsHeatmapStylePreference,
    onInsightsHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    onInsightsHeatmapPaletteNameChange: (String) -> Unit,
    insightsHeatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit,
    configCardExpansionPreferences: com.example.tracer.data.ConfigCardExpansionPreferences,
    onPersistConfigCardExpanded: (com.example.tracer.data.ConfigCard, Boolean) -> Unit,
    validAuthorableEventTokens: Set<String>,
    onPersistRecordQuickActivities: (List<String>) -> Unit,
    onClearQuickAccessCache: () -> Unit,
    onPersistRecordQuickAccessCardExpanded: (Boolean) -> Unit,
    onPersistRecordQuickAccessEditorVisibility: (Boolean) -> Unit,
    onPersistRecordCanonicalCatalogDisplayMode: (RecordFrequentOutputMode) -> Unit,
    onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    onPersistRecordFrequentLookbackDays: (Int) -> Unit,
    onPersistRecordFrequentOutputMode: (RecordFrequentOutputMode) -> Unit,
    onPersistRecordFrequentTopN: (Int) -> Unit,
    activityCategoriesContent: @Composable () -> Unit,
    onImportDataFolder: () -> Unit,
    onImportSingleTracer: () -> Unit,
    onExportAllMonthsTracer: () -> Unit,
    onExportCurrentTxtTracer: () -> Unit,
    isTracerExportInProgress: Boolean,
    selectedTracerSecurityLevel: TracerExchangeSecurityLevel,
    onTracerSecurityLevelChange: (TracerExchangeSecurityLevel) -> Unit,
    onCopyDiagnosticsPayload: () -> Unit,
    onEditDailyStatuses: () -> Unit
) {
    val args = TracerTabRouteArgs(
        dataViewModel = dataViewModel,
        queryUiState = queryUiState,
        queryInsightsViewModel = queryInsightsViewModel,
        txtStorageGateway = txtStorageGateway,
        recordUiState = recordUiState,
        recordViewModel = recordViewModel,
        themeConfig = themeConfig,
        onThemeEvent = onThemeEvent,
        insightsPiePalettePreset = insightsPiePalettePreset,
        onInsightsPiePalettePresetChange = onInsightsPiePalettePresetChange,
        insightsChartShowAverageLine = insightsChartShowAverageLine,
        onInsightsChartShowAverageLineChange = onInsightsChartShowAverageLineChange,
        insightsChartSemanticMode = insightsChartSemanticMode,
        onInsightsChartSemanticModeChange = onInsightsChartSemanticModeChange,
        insightsChartVisualMode = insightsChartVisualMode,
        onInsightsChartVisualModeChange = onInsightsChartVisualModeChange,
        insightsChartTrendRoot = insightsChartTrendRoot,
        onInsightsChartTrendRootChange = onInsightsChartTrendRootChange,
        insightsAverageDayBasis = insightsAverageDayBasis,
        onInsightsAverageDayBasisChange = onInsightsAverageDayBasisChange,
        insightsMode = insightsMode,
        onInsightsModeChange = onInsightsModeChange,
        insightsResultDisplayMode = insightsResultDisplayMode,
        onInsightsResultDisplayModeChange = onInsightsResultDisplayModeChange,
        insightsParameterSection = insightsParameterSection,
        onInsightsParameterSectionChange = onInsightsParameterSectionChange,
        insightsDayActivitiesView = insightsDayActivitiesView,
        insightsPeriodActivitiesView = insightsPeriodActivitiesView,
        onInsightsDayActivitiesViewChange = onInsightsDayActivitiesViewChange,
        onInsightsPeriodActivitiesViewChange = onInsightsPeriodActivitiesViewChange,
        insightsTimeParametersExpanded = insightsTimeParametersExpanded,
        onInsightsTimeParametersExpandedChange = onInsightsTimeParametersExpandedChange,
        insightsHeatmapTomlConfig = insightsHeatmapTomlConfig,
        insightsHeatmapStylePreference = insightsHeatmapStylePreference,
        onInsightsHeatmapThemePolicyChange = onInsightsHeatmapThemePolicyChange,
        onInsightsHeatmapPaletteNameChange = onInsightsHeatmapPaletteNameChange,
        insightsHeatmapApplyMessage = insightsHeatmapApplyMessage,
        isAppDarkThemeActive = isAppDarkThemeActive,
        appLanguage = appLanguage,
        onSetAppLanguage = onSetAppLanguage,
        configCardExpansionPreferences = configCardExpansionPreferences,
        onPersistConfigCardExpanded = onPersistConfigCardExpanded,
        validAuthorableEventTokens = validAuthorableEventTokens,
        onPersistRecordQuickActivities = onPersistRecordQuickActivities,
        onClearQuickAccessCache = onClearQuickAccessCache,
        onPersistRecordQuickAccessCardExpanded = onPersistRecordQuickAccessCardExpanded,
        onPersistRecordQuickAccessEditorVisibility = onPersistRecordQuickAccessEditorVisibility,
        onPersistRecordCanonicalCatalogDisplayMode = onPersistRecordCanonicalCatalogDisplayMode,
        onPersistRecordCollapsedCanonicalRootPaths = onPersistRecordCollapsedCanonicalRootPaths,
        onPersistRecordOrderedCanonicalRootPaths = onPersistRecordOrderedCanonicalRootPaths,
        onPersistRecordFrequentLookbackDays = onPersistRecordFrequentLookbackDays,
        onPersistRecordFrequentOutputMode = onPersistRecordFrequentOutputMode,
        onPersistRecordFrequentTopN = onPersistRecordFrequentTopN,
        activityCategoriesContent = activityCategoriesContent,
        onImportDataFolder = onImportDataFolder,
        onImportSingleTracer = onImportSingleTracer,
        onExportAllMonthsTracer = onExportAllMonthsTracer,
        onExportCurrentTxtTracer = onExportCurrentTxtTracer,
        isTracerExportInProgress = isTracerExportInProgress,
        selectedTracerSecurityLevel = selectedTracerSecurityLevel,
        onTracerSecurityLevelChange = onTracerSecurityLevelChange,
        onCopyDiagnosticsPayload = onCopyDiagnosticsPayload,
        onEditDailyStatuses = onEditDailyStatuses
    )
    TracerTabRegistry.entry(selectedTab).content(modifier, args)
}
