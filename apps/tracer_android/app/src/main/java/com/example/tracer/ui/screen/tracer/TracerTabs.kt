package com.example.tracer

import androidx.annotation.StringRes
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AddCircle
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.SnackbarDuration
import androidx.compose.runtime.Composable
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import kotlinx.coroutines.launch

internal enum class TracerTab {
    DATA,
    REPORT,
    RECORD,
    TXT,
    CONFIG
}

internal val DefaultTracerTab: TracerTab = TracerTab.RECORD

internal data class TabMeta(
    val id: TracerTab,
    @param:StringRes val titleRes: Int,
    val icon: ImageVector,
    val testTag: String? = null
)

internal enum class TracerTabScrollBehavior {
    NONE,
    VERTICAL
}

internal data class TracerTabRouteArgs(
    val dataViewModel: DataViewModel,
    val queryUiState: QueryReportUiState,
    val queryReportViewModel: QueryReportViewModel,
    val recordUiState: RecordUiState,
    val recordViewModel: RecordViewModel,
    val configUiState: ConfigUiState,
    val configViewModel: ConfigViewModel,
    val themeConfig: ThemeConfig,
    val onSetThemeColor: (ThemeColor) -> Unit,
    val onSetThemeMode: (ThemeMode) -> Unit,
    val onSetUseDynamicColor: (Boolean) -> Unit,
    val onSetDarkThemeStyle: (DarkThemeStyle) -> Unit,
    val reportChartShowAverageLine: Boolean,
    val onReportChartShowAverageLineChange: (Boolean) -> Unit,
    val reportHeatmapTomlConfig: ReportHeatmapTomlConfig,
    val reportHeatmapStylePreference: ReportHeatmapStylePreference,
    val onReportHeatmapThemePolicyChange: (ReportHeatmapThemePolicy) -> Unit,
    val onReportHeatmapPaletteNameChange: (String) -> Unit,
    val reportHeatmapApplyMessage: String,
    val isAppDarkThemeActive: Boolean,
    val appLanguage: AppLanguage,
    val onSetAppLanguage: (AppLanguage) -> Unit,
    val validMappingNames: Set<String>,
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onPersistRecordAssistExpanded: (Boolean) -> Unit,
    val onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    val onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    val onPersistRecordSuggestTopN: (Int) -> Unit,
    val onImportFolderTracer: () -> Unit,
    val onImportSingleTxt: () -> Unit,
    val onImportSingleTracer: () -> Unit,
    val onExportAllMonthsTxt: () -> Unit,
    val onExportAllMonthsTracer: () -> Unit,
    val isTxtExportInProgress: Boolean,
    val isTracerExportInProgress: Boolean,
    val selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    val onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    val onImportAndroidConfigFullReplace: () -> Unit,
    val onImportAndroidConfigPartialUpdate: () -> Unit,
    val onExportAndroidConfigBundle: () -> Unit,
    val onCopyDiagnosticsPayload: () -> Unit
)

internal data class TracerTabLifecycleArgs(
    val controller: RuntimeGateway,
    val recordViewModel: RecordViewModel,
    val configViewModel: ConfigViewModel,
    val recordStatusText: () -> String,
    val onValidMappingNamesChanged: (Set<String>) -> Unit
)

internal data class TracerTabStatusArgs(
    val dataStatusText: String,
    val queryStatusText: String,
    val recordStatusText: String,
    val configStatusText: String
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
    val entries: List<TracerTabEntry> = listOf(
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.DATA,
                titleRes = R.string.tracer_tab_data,
                icon = Icons.Default.Home,
                testTag = "tab_data"
            ),
            scrollBehavior = TracerTabScrollBehavior.NONE,
            statusText = { args -> args.dataStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { modifier, args ->
                val coroutineScope = rememberCoroutineScope()
                DataManagementSection(
                    modifier = modifier,
                    onIngestFull = {
                        coroutineScope.launch {
                            val ingestOk = args.dataViewModel.ingestFullAndGetResult()
                            // Keep Data tab export availability in sync with TXT store state,
                            // even when ingest returns non-ok (e.g. partial/diagnostic response).
                            args.recordViewModel.refreshHistory()
                            if (!ingestOk) {
                                return@launch
                            }
                        }
                    },
                    onImportFolderTracer = args.onImportFolderTracer,
                    onImportSingleTxt = args.onImportSingleTxt,
                    onImportSingleTracer = args.onImportSingleTracer,
                    canExportAllMonthsTxt = args.recordUiState.availableMonths.isNotEmpty(),
                    onExportAllMonthsTxt = args.onExportAllMonthsTxt,
                    isTxtExportInProgress = args.isTxtExportInProgress,
                    canExportAllMonthsTracer = args.recordUiState.availableMonths.isNotEmpty(),
                    onExportAllMonthsTracer = args.onExportAllMonthsTracer,
                    isTracerExportInProgress = args.isTracerExportInProgress,
                    selectedTracerSecurityLevel = args.selectedTracerSecurityLevel,
                    onTracerSecurityLevelChange = args.onTracerSecurityLevelChange,
                    showCryptoProgress = args.recordUiState.cryptoProgress.isVisible,
                    cryptoProgressTitle = args.recordUiState.cryptoProgress.operationText,
                    cryptoProgressPhase = args.recordUiState.cryptoProgress.phaseText,
                    cryptoOverallProgress = args.recordUiState.cryptoProgress.overallProgress,
                    cryptoOverallText = args.recordUiState.cryptoProgress.overallText,
                    cryptoCurrentProgress = args.recordUiState.cryptoProgress.currentProgress,
                    cryptoCurrentText = args.recordUiState.cryptoProgress.currentText,
                    cryptoDetailsText = args.recordUiState.cryptoProgress.detailsText,
                    cryptoAdvancedDetailsText = args.recordUiState.cryptoProgress.advancedDetailsText,
                    onClearTxt = {
                        args.dataViewModel.clearTxt()
                        args.recordViewModel.clearTxtEditorState()
                    },
                    onClearData = args.dataViewModel::clearDataAndReinitialize
                )
            }
        ),
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.REPORT,
                titleRes = R.string.tracer_tab_report,
                icon = Icons.Default.DateRange,
                testTag = "tab_report"
            ),
            scrollBehavior = TracerTabScrollBehavior.VERTICAL,
            statusText = { args -> args.queryStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { _, args ->
                QueryReportTabContent(
                    queryUiState = args.queryUiState,
                    queryReportViewModel = args.queryReportViewModel,
                    chartShowAverageLine = args.reportChartShowAverageLine,
                    onChartShowAverageLineChange = args.onReportChartShowAverageLineChange,
                    heatmapTomlConfig = args.reportHeatmapTomlConfig,
                    heatmapStylePreference = args.reportHeatmapStylePreference,
                    onHeatmapThemePolicyChange = args.onReportHeatmapThemePolicyChange,
                    onHeatmapPaletteNameChange = args.onReportHeatmapPaletteNameChange,
                    heatmapApplyMessage = args.reportHeatmapApplyMessage,
                    isAppDarkThemeActive = args.isAppDarkThemeActive
                )
            }
        ),
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.RECORD,
                titleRes = R.string.tracer_tab_record,
                icon = Icons.Default.AddCircle,
                testTag = "tab_record"
            ),
            scrollBehavior = TracerTabScrollBehavior.VERTICAL,
            onEnter = { args -> refreshRecordMappingValidation(args) },
            statusText = { args -> args.recordStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { _, args ->
                RecordTabContent(
                    recordUiState = args.recordUiState,
                    recordViewModel = args.recordViewModel,
                    validMappingNames = args.validMappingNames,
                    onPersistQuickActivities = args.onPersistRecordQuickActivities,
                    onPersistAssistExpanded = args.onPersistRecordAssistExpanded,
                    onPersistAssistSettingsExpanded = args.onPersistRecordAssistSettingsExpanded,
                    onPersistSuggestionLookbackDays = args.onPersistRecordSuggestLookbackDays,
                    onPersistSuggestionTopN = args.onPersistRecordSuggestTopN
                )
            }
        ),
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.TXT,
                titleRes = R.string.tracer_tab_txt,
                icon = Icons.Default.Edit,
                testTag = "tab_txt"
            ),
            scrollBehavior = TracerTabScrollBehavior.NONE,
            onEnter = { args -> refreshRecordMappingValidation(args) },
            onLeave = { args -> args.recordViewModel.discardUnsavedHistoryDraft() },
            statusText = { args -> args.recordStatusText },
            statusEvent = { null },
            content = { _, args ->
                TxtEditorSection(
                    availableMonths = args.recordUiState.availableMonths,
                    selectedMonth = args.recordUiState.selectedMonth,
                    onOpenPreviousMonth = args.recordViewModel::openPreviousMonth,
                    onOpenNextMonth = args.recordViewModel::openNextMonth,
                    onOpenMonth = args.recordViewModel::openMonth,
                    selectedHistoryFile = args.recordUiState.selectedHistoryFile,
                    onRefreshHistory = args.recordViewModel::refreshHistory,
                    editableHistoryContent = args.recordUiState.editableHistoryContent,
                    onEditableHistoryContentChange = args.recordViewModel::updateEditableHistoryContent,
                    onSaveHistoryFile = args.recordViewModel::saveHistoryFileAndSync,
                    inlineStatusText = args.recordUiState.statusText
                )
            }
        ),
        TracerTabEntry(
            meta = TabMeta(
                id = TracerTab.CONFIG,
                titleRes = R.string.tracer_tab_config,
                icon = Icons.Default.Settings,
                testTag = "tab_config"
            ),
            scrollBehavior = TracerTabScrollBehavior.VERTICAL,
            onEnter = { args -> args.configViewModel.refreshConfigFiles() },
            onLeave = { args -> args.configViewModel.discardUnsavedDraft() },
            statusText = { args -> args.configStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { _, args ->
                ConfigSection(
                    selectedCategory = args.configUiState.selectedCategory,
                    converterFiles = args.configUiState.converterFiles,
                    reportFiles = args.configUiState.reportFiles,
                    selectedFile = args.configUiState.selectedFile,
                    editableContent = args.configUiState.editableContent,
                    themeConfig = args.themeConfig,
                    onSelectConverter = { args.configViewModel.selectCategory(ConfigCategory.CONVERTER) },
                    onSelectReports = { args.configViewModel.selectCategory(ConfigCategory.REPORTS) },
                    onRefreshFiles = args.configViewModel::refreshConfigFiles,
                    onOpenFile = args.configViewModel::openFile,
                    onImportAndroidConfigFullReplace = args.onImportAndroidConfigFullReplace,
                    onImportAndroidConfigPartialUpdate = args.onImportAndroidConfigPartialUpdate,
                    onExportAndroidConfig = args.onExportAndroidConfigBundle,
                    onCopyDiagnosticsPayload = args.onCopyDiagnosticsPayload,
                    onEditableContentChange = args.configViewModel::onEditableContentChange,
                    onSaveCurrentFile = args.configViewModel::saveCurrentFile,
                    onSetThemeColor = args.onSetThemeColor,
                    onSetThemeMode = args.onSetThemeMode,
                    onSetUseDynamicColor = args.onSetUseDynamicColor,
                    onSetDarkThemeStyle = args.onSetDarkThemeStyle,
                    appLanguage = args.appLanguage,
                    onSetAppLanguage = args.onSetAppLanguage
                )
            }
        )
    )

    private val entryByTab: Map<TracerTab, TracerTabEntry> = entries.associateBy { it.meta.id }

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

private const val ActivityMappingValidationUnavailablePrefix =
    "Activity mapping validation unavailable:"

private suspend fun refreshRecordMappingValidation(args: TracerTabLifecycleArgs) {
    val mappingResult = args.controller.listActivityMappingNames()
    if (mappingResult.ok) {
        args.onValidMappingNamesChanged(mappingResult.names.toSet())
        if (args.recordStatusText().startsWith(ActivityMappingValidationUnavailablePrefix)) {
            args.recordViewModel.setStatusText("")
        }
        return
    }

    args.onValidMappingNamesChanged(emptySet())
    args.recordViewModel.setStatusText(
        "$ActivityMappingValidationUnavailablePrefix ${mappingResult.message}"
    )
}

private fun defaultStatusUiEvent(args: TracerTabStatusEventArgs): TracerTabUiEvent? {
    if (!args.isTabUnchanged || !args.hasStatusUpdate) {
        return null
    }

    val isPartialExportCompletion = args.statusText.startsWith("Export all completed ->")
    if (isPartialExportCompletion) {
        return TracerTabUiEvent.ShowSnackbar(
            message = "Export all completed with issues.",
            actionLabel = "Details",
            withDismissAction = true,
            duration = SnackbarDuration.Long,
            onActionEvent = TracerCoordinatorEvent.SelectTab(TracerTab.TXT)
        )
    }

    val isFailureStatus = args.statusText.contains("fail", ignoreCase = true) ||
        args.statusText.contains("error", ignoreCase = true)
    return TracerTabUiEvent.ShowSnackbar(
        message = args.statusText,
        withDismissAction = true,
        duration = if (isFailureStatus) SnackbarDuration.Long else SnackbarDuration.Short
    )
}
