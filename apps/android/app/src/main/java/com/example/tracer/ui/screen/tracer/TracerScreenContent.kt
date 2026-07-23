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
    queryUiState: QueryReportUiState,
    queryReportViewModel: QueryReportViewModel,
    txtStorageGateway: TxtStorageGateway,
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    configUiState: ConfigUiState,
    configViewModel: ConfigViewModel,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit,
    reportPiePalettePreset: ReportPiePalettePreset,
    onReportPiePalettePresetChange: (ReportPiePalettePreset) -> Unit,
    reportChartShowAverageLine: Boolean,
    onReportChartShowAverageLineChange: (Boolean) -> Unit,
    reportChartSemanticMode: ReportChartSemanticMode,
    onReportChartSemanticModeChange: (ReportChartSemanticMode) -> Unit,
    reportChartVisualMode: ReportChartVisualMode,
    onReportChartVisualModeChange: (ReportChartVisualMode) -> Unit,
    reportMode: ReportMode,
    onReportModeChange: (ReportMode) -> Unit,
    reportResultDisplayMode: ReportResultDisplayMode,
    onReportResultDisplayModeChange: (ReportResultDisplayMode) -> Unit,
    reportParameterSection: ReportParameterSection,
    onReportParameterSectionChange: (ReportParameterSection) -> Unit,
    reportTimeParametersExpanded: Boolean,
    onReportTimeParametersExpandedChange: (Boolean) -> Unit,
    reportHeatmapTomlConfig: ReportHeatmapTomlConfig,
    reportHeatmapStylePreference: ReportHeatmapStylePreference,
    onReportHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onReportHeatmapPaletteNameChange: (String) -> Unit,
    reportHeatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit,
    validAuthorableEventTokens: Set<String>,
    onPersistRecordQuickActivities: (List<String>) -> Unit,
    onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    onPersistRecordCanonicalCatalogDisplayMode: (RecordSuggestionOutputMode) -> Unit,
    onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    onPersistRecordSuggestOutputMode: (RecordSuggestionOutputMode) -> Unit,
    onPersistRecordSuggestTopN: (Int) -> Unit,
    onImportSingleTxt: () -> Unit,
    onImportTomlFolder: () -> Unit,
    onImportSingleTracer: () -> Unit,
    onExportAllMonthsTracer: () -> Unit,
    onExportCurrentTxtTracer: () -> Unit,
    isTracerExportInProgress: Boolean,
    selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    onCopyDiagnosticsPayload: () -> Unit
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
                modifier = if (selectedTab == TracerTab.DATA) Modifier.weight(1f) else Modifier,
                selectedTab = selectedTab,
                dataViewModel = dataViewModel,
                queryUiState = queryUiState,
                queryReportViewModel = queryReportViewModel,
                txtStorageGateway = txtStorageGateway,
                recordUiState = recordUiState,
                recordViewModel = recordViewModel,
                configUiState = configUiState,
                configViewModel = configViewModel,
                themeConfig = themeConfig,
                onSetThemeColor = onSetThemeColor,
                onSetThemeMode = onSetThemeMode,
                onSetUseDynamicColor = onSetUseDynamicColor,
                onSetDarkThemeStyle = onSetDarkThemeStyle,
                reportPiePalettePreset = reportPiePalettePreset,
                onReportPiePalettePresetChange = onReportPiePalettePresetChange,
                reportChartShowAverageLine = reportChartShowAverageLine,
                onReportChartShowAverageLineChange = onReportChartShowAverageLineChange,
                reportChartSemanticMode = reportChartSemanticMode,
                onReportChartSemanticModeChange = onReportChartSemanticModeChange,
                reportChartVisualMode = reportChartVisualMode,
                onReportChartVisualModeChange = onReportChartVisualModeChange,
                reportMode = reportMode,
                onReportModeChange = onReportModeChange,
                reportResultDisplayMode = reportResultDisplayMode,
                onReportResultDisplayModeChange = onReportResultDisplayModeChange,
                reportParameterSection = reportParameterSection,
                onReportParameterSectionChange = onReportParameterSectionChange,
                reportTimeParametersExpanded = reportTimeParametersExpanded,
                onReportTimeParametersExpandedChange = onReportTimeParametersExpandedChange,
                reportHeatmapTomlConfig = reportHeatmapTomlConfig,
                reportHeatmapStylePreference = reportHeatmapStylePreference,
                onReportHeatmapThemePolicyChange = onReportHeatmapThemePolicyChange,
                onReportHeatmapPaletteNameChange = onReportHeatmapPaletteNameChange,
                reportHeatmapApplyMessage = reportHeatmapApplyMessage,
                isAppDarkThemeActive = isAppDarkThemeActive,
                appLanguage = appLanguage,
                onSetAppLanguage = onSetAppLanguage,
                validAuthorableEventTokens = validAuthorableEventTokens,
                onPersistRecordQuickActivities = onPersistRecordQuickActivities,
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
                onImportSingleTxt = onImportSingleTxt,
                onImportTomlFolder = onImportTomlFolder,
                onImportSingleTracer = onImportSingleTracer,
                onExportAllMonthsTracer = onExportAllMonthsTracer,
                onExportCurrentTxtTracer = onExportCurrentTxtTracer,
                isTracerExportInProgress = isTracerExportInProgress,
                selectedTracerSecurityLevel = selectedTracerSecurityLevel,
                onTracerSecurityLevelChange = onTracerSecurityLevelChange,
                onCopyDiagnosticsPayload = onCopyDiagnosticsPayload
            )
        }
    }
}
