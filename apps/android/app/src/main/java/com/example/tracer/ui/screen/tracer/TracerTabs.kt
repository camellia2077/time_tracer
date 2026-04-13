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
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode

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
    val txtStorageGateway: TxtStorageGateway,
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
    val validAuthorableEventTokens: Set<String>,
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onPersistRecordAssistExpanded: (Boolean) -> Unit,
    val onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    val onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    val onPersistRecordSuggestTopN: (Int) -> Unit,
    val onImportSingleTxt: () -> Unit,
    val onImportSingleTracer: () -> Unit,
    val onExportAllMonthsTracer: () -> Unit,
    val isTracerExportInProgress: Boolean,
    val selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    val onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    val onCopyDiagnosticsPayload: () -> Unit,
    val onClearDatabase: () -> Unit
)

internal data class TracerTabLifecycleArgs(
    val queryGateway: QueryGateway,
    val recordViewModel: RecordViewModel,
    val configViewModel: ConfigViewModel,
    val recordStatusText: () -> String,
    val onValidAuthorableEventTokensChanged: (Set<String>) -> Unit
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
                DataManagementSection(
                    modifier = modifier,
                    onImportSingleTxt = args.onImportSingleTxt,
                    onImportSingleTracer = args.onImportSingleTracer,
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
                    onClearDatabase = args.onClearDatabase,
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
                    availableTxtMonths = args.recordUiState.availableMonths,
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
            // Do not clear logical-day override on tab leave.
            // Yesterday/today is shared session state across Record and TXT so users keep one
            // target-day intent while switching tabs.
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
                    validAuthorableEventTokens = args.validAuthorableEventTokens,
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
            onEnter = { args ->
                refreshRecordMappingValidation(args)
                // Keep TXT aligned with Record's auto default when users have not overridden.
                args.recordViewModel.refreshLogicalDayDefault()
            },
            onLeave = { args -> args.recordViewModel.discardUnsavedHistoryDraft() },
            statusText = { args -> args.recordStatusText },
            statusEvent = { null },
            content = { _, args ->
                TxtEditorSection(
                    txtStorageGateway = args.txtStorageGateway,
                    inspectionEntries = args.recordUiState.txtInspectionEntries,
                    availableMonths = args.recordUiState.availableMonths,
                    selectedMonth = args.recordUiState.selectedMonth,
                    logicalDayTarget = args.recordUiState.logicalDayTarget,
                    onOpenPreviousMonth = args.recordViewModel::openPreviousMonth,
                    onOpenNextMonth = args.recordViewModel::openNextMonth,
                    onOpenMonth = args.recordViewModel::openMonth,
                    selectedHistoryFile = args.recordUiState.selectedHistoryFile,
                    onRefreshHistory = args.recordViewModel::refreshHistory,
                    editableHistoryContent = args.recordUiState.editableHistoryContent,
                    onEditableHistoryContentChange = args.recordViewModel::updateEditableHistoryContent,
                    onSaveHistoryFile = args.recordViewModel::saveHistoryFileAndSync,
                    inlineStatusText = args.recordUiState.statusText,
                    onCreateCurrentMonthTxt = args.recordViewModel::createCurrentMonthTxt
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
                    selectedConverterSubcategory = args.configUiState.selectedConverterSubcategory,
                    converterFiles = args.configUiState.converterFiles,
                    chartFiles = args.configUiState.chartFiles,
                    metaFiles = args.configUiState.metaFiles,
                    reportFiles = args.configUiState.reportFiles,
                    selectedFilePath = args.configUiState.selectedFilePath,
                    selectedFileDisplayName = args.configUiState.selectedFileDisplayName,
                    editableContent = args.configUiState.editableContent,
                    aliasEditorMode = args.configUiState.aliasEditorMode,
                    aliasDocumentDraft = args.configUiState.aliasDocumentDraft,
                    aliasParentOptions = args.configUiState.aliasParentOptions,
                    aliasAdvancedTomlDraft = args.configUiState.aliasAdvancedTomlDraft,
                    aliasEditorErrorMessage = args.configUiState.aliasEditorErrorMessage,
                    themeConfig = args.themeConfig,
                    onSelectConverter = { args.configViewModel.selectCategory(ConfigCategory.CONVERTER) },
                    onSelectCharts = { args.configViewModel.selectCategory(ConfigCategory.CHARTS) },
                    onSelectMeta = { args.configViewModel.selectCategory(ConfigCategory.META) },
                    onSelectReports = { args.configViewModel.selectCategory(ConfigCategory.REPORTS) },
                    onSelectConverterAliases = {
                        args.configViewModel.selectConverterSubcategory(ConverterSubcategory.ALIASES)
                    },
                    onSelectConverterRules = {
                        args.configViewModel.selectConverterSubcategory(ConverterSubcategory.RULES)
                    },
                    onRefreshFiles = args.configViewModel::refreshConfigFiles,
                    onOpenFile = args.configViewModel::openFile,
                    onCopyDiagnosticsPayload = args.onCopyDiagnosticsPayload,
                    onEditableContentChange = args.configViewModel::onEditableContentChange,
                    onSelectAliasStructuredMode = {
                        args.configViewModel.selectAliasEditorMode(AliasEditorMode.STRUCTURED)
                    },
                    onSelectAliasAdvancedMode = {
                        args.configViewModel.selectAliasEditorMode(AliasEditorMode.ADVANCED)
                    },
                    onAliasParentChange = args.configViewModel::updateAliasParent,
                    onAliasAdvancedTomlChange = args.configViewModel::onAliasAdvancedTomlChange,
                    onAddAliasGroup = args.configViewModel::addAliasGroup,
                    onRenameAliasGroup = args.configViewModel::renameAliasGroup,
                    onDeleteAliasGroup = args.configViewModel::deleteAliasGroup,
                    onAddAliasEntry = args.configViewModel::addAliasEntry,
                    onUpdateAliasEntry = args.configViewModel::updateAliasEntry,
                    onDeleteAliasEntry = args.configViewModel::deleteAliasEntry,
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
    return TracerTabUiEvent.ShowSnackbar(
        message = args.statusText,
        withDismissAction = true,
        duration = if (isFailureStatus) SnackbarDuration.Long else SnackbarDuration.Short
    )
}
