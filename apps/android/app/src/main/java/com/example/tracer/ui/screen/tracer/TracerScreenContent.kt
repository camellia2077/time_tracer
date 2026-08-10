package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.material3.SnackbarHostState
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
internal fun TracerScreenContent(
    selectedTab: TracerTab,
    snackbarHostState: SnackbarHostState,
    onCoordinatorEvent: (TracerCoordinatorEvent) -> Unit,
    dataViewModel: DataViewModel,
    queryUiState: QueryInsightsUiState,
    queryInsightsViewModel: QueryInsightsViewModel,
    txtStorageGateway: TxtStorageGateway,
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    configUiState: ConfigUiState,
    configViewModel: ConfigViewModel,
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
    validAuthorableEventTokens: Set<String>,
    onPersistRecordQuickActivities: (List<String>) -> Unit,
    onClearQuickAccessCache: () -> Unit,
    onPersistRecordQuickAccessCardExpanded: (Boolean) -> Unit,
    onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    onPersistRecordCanonicalCatalogDisplayMode: (RecordSuggestionOutputMode) -> Unit,
    onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    onPersistRecordSuggestOutputMode: (RecordSuggestionOutputMode) -> Unit,
    onPersistRecordSuggestTopN: (Int) -> Unit,
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
            TracerTabRouteContent(
                modifier = Modifier,
                selectedTab = selectedTab,
                dataViewModel = dataViewModel,
                queryUiState = queryUiState,
                queryInsightsViewModel = queryInsightsViewModel,
                txtStorageGateway = txtStorageGateway,
                recordUiState = recordUiState,
                recordViewModel = recordViewModel,
                configUiState = configUiState,
                configViewModel = configViewModel,
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
                validAuthorableEventTokens = validAuthorableEventTokens,
                onPersistRecordQuickActivities = onPersistRecordQuickActivities,
                onClearQuickAccessCache = onClearQuickAccessCache,
                onPersistRecordQuickAccessCardExpanded =
                    onPersistRecordQuickAccessCardExpanded,
                onPersistRecordAssistSettingsExpanded = onPersistRecordAssistSettingsExpanded,
                onPersistRecordCanonicalCatalogDisplayMode =
                    onPersistRecordCanonicalCatalogDisplayMode,
                onPersistRecordCollapsedCanonicalRootPaths =
                    onPersistRecordCollapsedCanonicalRootPaths,
                onPersistRecordOrderedCanonicalRootPaths =
                    onPersistRecordOrderedCanonicalRootPaths,
                onPersistRecordSuggestLookbackDays = onPersistRecordSuggestLookbackDays,
                onPersistRecordSuggestOutputMode = onPersistRecordSuggestOutputMode,
                onPersistRecordSuggestTopN = onPersistRecordSuggestTopN,
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
        }
    }
}
