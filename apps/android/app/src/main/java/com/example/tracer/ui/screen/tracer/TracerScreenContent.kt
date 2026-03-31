package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SnackbarHostState
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
internal fun TracerScreenContent(
    selectedTab: TracerTab,
    statusText: String,
    isDebuggableBuild: Boolean,
    snackbarHostState: SnackbarHostState,
    onCoordinatorEvent: (TracerCoordinatorEvent) -> Unit,
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
    validAuthorableEventTokens: Set<String>,
    onPersistRecordQuickActivities: (List<String>) -> Unit,
    onPersistRecordAssistExpanded: (Boolean) -> Unit,
    onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    onPersistRecordSuggestTopN: (Int) -> Unit,
    onImportSingleTxt: () -> Unit,
    onImportSingleTracer: () -> Unit,
    onExportAllMonthsTracer: () -> Unit,
    isTracerExportInProgress: Boolean,
    selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    onCopyDiagnosticsPayload: () -> Unit,
    onClearDatabase: () -> Unit
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
                validAuthorableEventTokens = validAuthorableEventTokens,
                onPersistRecordQuickActivities = onPersistRecordQuickActivities,
                onPersistRecordAssistExpanded = onPersistRecordAssistExpanded,
                onPersistRecordAssistSettingsExpanded = onPersistRecordAssistSettingsExpanded,
                onPersistRecordSuggestLookbackDays = onPersistRecordSuggestLookbackDays,
                onPersistRecordSuggestTopN = onPersistRecordSuggestTopN,
                onImportSingleTxt = onImportSingleTxt,
                onImportSingleTracer = onImportSingleTracer,
                onExportAllMonthsTracer = onExportAllMonthsTracer,
                isTracerExportInProgress = isTracerExportInProgress,
                selectedTracerSecurityLevel = selectedTracerSecurityLevel,
                onTracerSecurityLevelChange = onTracerSecurityLevelChange,
                onCopyDiagnosticsPayload = onCopyDiagnosticsPayload,
                onClearDatabase = onClearDatabase
            )
        }
    }
}
