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
        // Report consumes it below its fixed Day/Week/Month header; the other tabs do not
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
    reportResultDisplayMode: ReportResultDisplayMode,
    onReportResultDisplayModeChange: (ReportResultDisplayMode) -> Unit,
    reportParameterSection: ReportParameterSection,
    onReportParameterSectionChange: (ReportParameterSection) -> Unit,
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
    val args = TracerTabRouteArgs(
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
        reportResultDisplayMode = reportResultDisplayMode,
        onReportResultDisplayModeChange = onReportResultDisplayModeChange,
        reportParameterSection = reportParameterSection,
        onReportParameterSectionChange = onReportParameterSectionChange,
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
        onPersistRecordCanonicalCatalogDisplayMode = onPersistRecordCanonicalCatalogDisplayMode,
        onPersistRecordCollapsedCanonicalRootPaths = onPersistRecordCollapsedCanonicalRootPaths,
        onPersistRecordOrderedCanonicalRootPaths = onPersistRecordOrderedCanonicalRootPaths,
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
    TracerTabRegistry.entry(selectedTab).content(modifier, args)
}
