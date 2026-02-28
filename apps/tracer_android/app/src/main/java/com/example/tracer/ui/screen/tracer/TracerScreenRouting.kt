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
        .padding(innerPadding)
        .padding(
            start = ScreenOuterPadding,
            end = ScreenOuterPadding,
            bottom = bottomOuterPadding
        )
    val selectedEntry = TracerTabRegistry.entry(selectedTab)
    return if (selectedEntry.scrollBehavior == TracerTabScrollBehavior.VERTICAL) {
        baseModifier.verticalScroll(rememberScrollState())
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
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    configUiState: ConfigUiState,
    configViewModel: ConfigViewModel,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit,
    reportChartShowAverageLine: Boolean,
    onReportChartShowAverageLineChange: (Boolean) -> Unit,
    reportHeatmapTomlConfig: ReportHeatmapTomlConfig,
    reportHeatmapStylePreference: ReportHeatmapStylePreference,
    onReportHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    onReportHeatmapPaletteNameChange: (String) -> Unit,
    reportHeatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit,
    validMappingNames: Set<String>,
    onPersistRecordQuickActivities: (List<String>) -> Unit,
    onPersistRecordAssistExpanded: (Boolean) -> Unit,
    onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    onPersistRecordSuggestTopN: (Int) -> Unit,
    onImportFolderTracer: () -> Unit,
    onImportSingleTxt: () -> Unit,
    onImportSingleTracer: () -> Unit,
    onExportAllMonthsTxt: () -> Unit,
    onExportAllMonthsTracer: () -> Unit,
    isTxtExportInProgress: Boolean,
    isTracerExportInProgress: Boolean,
    selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    onImportAndroidConfigFullReplace: () -> Unit,
    onImportAndroidConfigPartialUpdate: () -> Unit,
    onExportAndroidConfigBundle: () -> Unit,
    onCopyDiagnosticsPayload: () -> Unit
) {
    val args = TracerTabRouteArgs(
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
        onReportChartShowAverageLineChange = onReportChartShowAverageLineChange,
        reportHeatmapTomlConfig = reportHeatmapTomlConfig,
        reportHeatmapStylePreference = reportHeatmapStylePreference,
        onReportHeatmapThemePolicyChange = onReportHeatmapThemePolicyChange,
        onReportHeatmapPaletteNameChange = onReportHeatmapPaletteNameChange,
        reportHeatmapApplyMessage = reportHeatmapApplyMessage,
        isAppDarkThemeActive = isAppDarkThemeActive,
        appLanguage = appLanguage,
        onSetAppLanguage = onSetAppLanguage,
        validMappingNames = validMappingNames,
        onPersistRecordQuickActivities = onPersistRecordQuickActivities,
        onPersistRecordAssistExpanded = onPersistRecordAssistExpanded,
        onPersistRecordAssistSettingsExpanded = onPersistRecordAssistSettingsExpanded,
        onPersistRecordSuggestLookbackDays = onPersistRecordSuggestLookbackDays,
        onPersistRecordSuggestTopN = onPersistRecordSuggestTopN,
        onImportFolderTracer = onImportFolderTracer,
        onImportSingleTxt = onImportSingleTxt,
        onImportSingleTracer = onImportSingleTracer,
        onExportAllMonthsTxt = onExportAllMonthsTxt,
        onExportAllMonthsTracer = onExportAllMonthsTracer,
        isTxtExportInProgress = isTxtExportInProgress,
        isTracerExportInProgress = isTracerExportInProgress,
        selectedTracerSecurityLevel = selectedTracerSecurityLevel,
        onTracerSecurityLevelChange = onTracerSecurityLevelChange,
        onImportAndroidConfigFullReplace = onImportAndroidConfigFullReplace,
        onImportAndroidConfigPartialUpdate = onImportAndroidConfigPartialUpdate,
        onExportAndroidConfigBundle = onExportAndroidConfigBundle,
        onCopyDiagnosticsPayload = onCopyDiagnosticsPayload
    )
    TracerTabRegistry.entry(selectedTab).content(modifier, args)
}
