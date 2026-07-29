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
import androidx.compose.ui.res.stringResource
import com.example.tracer.feature.data.R as DataFeatureR
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.ThemeConfig
import com.example.tracer.ui.viewmodel.ThemeEvent

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
    val onThemeEvent: (ThemeEvent) -> Unit,
    val reportPiePalettePreset: ReportPiePalettePreset,
    val onReportPiePalettePresetChange: (ReportPiePalettePreset) -> Unit,
    val reportChartShowAverageLine: Boolean,
    val onReportChartShowAverageLineChange: (Boolean) -> Unit,
    val reportChartSemanticMode: ReportChartSemanticMode,
    val onReportChartSemanticModeChange: (ReportChartSemanticMode) -> Unit,
    val reportChartVisualMode: ReportChartVisualMode,
    val onReportChartVisualModeChange: (ReportChartVisualMode) -> Unit,
    val reportMode: ReportMode,
    val onReportModeChange: (ReportMode) -> Unit,
    val reportResultDisplayMode: ReportResultDisplayMode,
    val onReportResultDisplayModeChange: (ReportResultDisplayMode) -> Unit,
    val reportParameterSection: ReportParameterSection,
    val onReportParameterSectionChange: (ReportParameterSection) -> Unit,
    val reportTimeParametersExpanded: Boolean,
    val onReportTimeParametersExpandedChange: (Boolean) -> Unit,
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
    val onPersistRecordQuickAccessCardExpanded: (Boolean) -> Unit,
    val onPersistRecordAssistSettingsExpanded: (Boolean) -> Unit,
    val onPersistRecordCanonicalCatalogDisplayMode: (RecordSuggestionOutputMode) -> Unit,
    val onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    val onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    val onPersistRecordSuggestLookbackDays: (Int) -> Unit,
    val onPersistRecordSuggestOutputMode: (RecordSuggestionOutputMode) -> Unit,
    val onPersistRecordSuggestTopN: (Int) -> Unit,
    val onImportDataFolder: () -> Unit,
    val onImportSingleTracer: () -> Unit,
    val onExportAllMonthsTracer: () -> Unit,
    val onExportCurrentTxtTracer: () -> Unit,
    val isTracerExportInProgress: Boolean,
    val selectedTracerSecurityLevel: FileCryptoSecurityLevel,
    val onTracerSecurityLevelChange: (FileCryptoSecurityLevel) -> Unit,
    val onCopyDiagnosticsPayload: () -> Unit
)

internal data class TracerTabLifecycleArgs(
    val queryGateway: QueryGateway,
    val queryReportViewModel: QueryReportViewModel,
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
                    modifier = modifier,
                    onImportDataFolder = args.onImportDataFolder,
                    onImportSingleTracer = args.onImportSingleTracer,
                    canExportAllMonthsTracer = args.recordUiState.availableMonths.isNotEmpty(),
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
                    cryptoCurrentProgress = args.recordUiState.cryptoProgress.currentProgress,
                    cryptoCurrentText = args.recordUiState.cryptoProgress.currentText,
                    cryptoDetailsText = args.recordUiState.cryptoProgress.detailsText,
                    cryptoAdvancedDetailsText = args.recordUiState.cryptoProgress.advancedDetailsText,
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
                        args.dataViewModel.clearDataAndReinitialize(clearAllDataStatusText)
                    }
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
            scrollBehavior = TracerTabScrollBehavior.NONE,
            // The ViewModel defers its first refresh until the report presentation preferences
            // are available, preventing the default Text query from racing the persisted Chart.
            onEnter = { args -> args.queryReportViewModel.onReportTabEntered() },
            statusText = { args -> args.queryStatusText },
            statusEvent = { args ->
                if (args.statusText.startsWith("query data ", ignoreCase = true) ||
                    // Markdown report output is rendered directly in the Report result card.
                    // Showing the nativeReportJson progress/result text as a global snackbar
                    // duplicates that feedback and obscures the floating navigation.
                    args.statusText.startsWith("nativeReportJson(", ignoreCase = true)) {
                    null
                } else {
                    defaultStatusUiEvent(args)
                }
            },
            content = { modifier, args ->
                QueryReportTabContent(
                    modifier = modifier,
                    queryUiState = args.queryUiState,
                    queryReportViewModel = args.queryReportViewModel,
                    preferredReportMode = args.reportMode,
                    onPreferredReportModeChange = args.onReportModeChange,
                    preferredResultDisplayMode = args.reportResultDisplayMode,
                    onPreferredResultDisplayModeChange = args.onReportResultDisplayModeChange,
                    preferredParameterSection = args.reportParameterSection,
                    onPreferredParameterSectionChange = args.onReportParameterSectionChange,
                    timeParametersExpanded = args.reportTimeParametersExpanded,
                    onTimeParametersExpandedChange = args.onReportTimeParametersExpandedChange,
                    preferredChartSemanticMode = args.reportChartSemanticMode,
                    onPreferredChartSemanticModeChange = args.onReportChartSemanticModeChange,
                    preferredChartVisualMode = args.reportChartVisualMode,
                    onPreferredChartVisualModeChange = args.onReportChartVisualModeChange,
                    chartShowAverageLine = args.reportChartShowAverageLine,
                    piePalettePreset = args.reportPiePalettePreset,
                    onChartShowAverageLineChange = args.onReportChartShowAverageLineChange,
                    heatmapTomlConfig = args.reportHeatmapTomlConfig,
                    heatmapStylePreference = args.reportHeatmapStylePreference,
                    onHeatmapThemePolicyChange = args.onReportHeatmapThemePolicyChange,
                    onHeatmapPaletteNameChange = args.onReportHeatmapPaletteNameChange,
                    heatmapApplyMessage = args.reportHeatmapApplyMessage,
                    isAppDarkThemeActive = args.isAppDarkThemeActive,
                    bottomContentPadding = floatingBottomNavScrollPadding()
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
                    txtStorageGateway = args.txtStorageGateway,
                    validAuthorableEventTokens = args.validAuthorableEventTokens,
                    onPersistQuickActivities = args.onPersistRecordQuickActivities,
                    onPersistQuickAccessCardExpanded =
                        args.onPersistRecordQuickAccessCardExpanded,
                    onPersistAssistSettingsExpanded = args.onPersistRecordAssistSettingsExpanded,
                    onPersistCanonicalCatalogDisplayMode =
                        args.onPersistRecordCanonicalCatalogDisplayMode,
                    onPersistCollapsedCanonicalRootPaths =
                        args.onPersistRecordCollapsedCanonicalRootPaths,
                    onPersistOrderedCanonicalRootPaths =
                        args.onPersistRecordOrderedCanonicalRootPaths,
                    onPersistSuggestionLookbackDays = args.onPersistRecordSuggestLookbackDays,
                    onPersistSuggestionOutputMode = args.onPersistRecordSuggestOutputMode,
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
            },
            // TXT remains a file-backed editor surface: if users leave the tab without ingesting,
            // return to the last saved month content instead of preserving a hidden in-memory
            // draft that looks like persisted data on the next open.
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
                    onDiscardUnsavedHistoryDraft = args.recordViewModel::discardUnsavedHistoryDraft,
                    onSaveHistoryFile = args.recordViewModel::saveHistoryFileAndSync,
                    onSaveHistoryRepresentationOnly = args.recordViewModel::saveHistoryFileRepresentationOnly,
                    initialOutputMode = args.recordUiState.txtOutputMode,
                    onOutputModePersist = args.recordViewModel::onTxtOutputModeChange,
                    bottomContentPadding = floatingBottomNavScrollPadding(),
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
            onEnter = { args -> args.configViewModel.refreshConfigFiles(showStatus = false) },
            statusText = { args -> args.configStatusText },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { _, args ->
                ConfigSection(
                    selectedCategory = args.configUiState.selectedCategory,
                    aliasFiles = args.configUiState.aliasFiles,
                    chartFiles = args.configUiState.chartFiles,
                    metaFiles = args.configUiState.metaFiles,
                    reportFiles = args.configUiState.reportFiles,
                    selectedFilePath = args.configUiState.selectedFilePath,
                    selectedFileDisplayName = args.configUiState.selectedFileDisplayName,
                    selectedFileContent = args.configUiState.selectedFileContent,
                    editableContent = args.configUiState.editableContent,
                    aliasEditorMode = args.configUiState.aliasEditorMode,
                    aliasDocumentDraft = args.configUiState.aliasDocumentDraft,
                    aliasEntryMovePlan = args.configUiState.aliasEntryMovePlan,
                    aliasEntryMoveDestinations = args.configUiState.aliasEntryMoveDestinations,
                    aliasEntryMoveDestinationsLoading = args.configUiState.aliasEntryMoveDestinationsLoading,
                    aliasParentOptions = args.configUiState.aliasParentOptions,
                    aliasAdvancedTomlDraft = args.configUiState.aliasAdvancedTomlDraft,
                    aliasEditorErrorMessage = args.configUiState.aliasEditorErrorMessage,
                    autoSaveStatus = args.configUiState.autoSaveStatus,
                    themeConfig = args.themeConfig,
                    onSelectAlias = { args.configViewModel.selectCategory(ConfigCategory.ALIAS) },
                    onSelectCharts = { args.configViewModel.selectCategory(ConfigCategory.CHARTS) },
                    onSelectMeta = { args.configViewModel.selectCategory(ConfigCategory.META) },
                    onSelectReports = { args.configViewModel.selectCategory(ConfigCategory.REPORTS) },
                    onRefreshFiles = args.configViewModel::refreshConfigFiles,
                    onOpenFile = args.configViewModel::openFile,
                    onCreateAliasTomlFile = args.configViewModel::createAliasTomlFile,
                    onDeleteAliasTomlFile = args.configViewModel::deleteCurrentAliasTomlFile,
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
                    onDeleteAliasGroup = args.configViewModel::deleteAliasGroup,
                    onRenameAliasGroup = args.configViewModel::renameAliasGroup,
                    onAddAliasEntry = args.configViewModel::addAliasEntry,
                    onUpdateAliasEntry = args.configViewModel::updateAliasEntry,
                    onPromoteAliasEntry = args.configViewModel::promoteAliasEntryToGroup,
                    onRenameGroupAlias = args.configViewModel::renameGroupAlias,
                    onAddGroupAlias = args.configViewModel::addGroupAlias,
                    onUpdateGroupAliases = args.configViewModel::updateGroupAliases,
                    onDeleteAliasEntry = args.configViewModel::deleteAliasEntry,
                    onPrepareAliasEntryMove = args.configViewModel::prepareAliasEntryMove,
                    onPrepareAliasGroupMove = args.configViewModel::prepareAliasGroupMove,
                    onPreviewAliasEntryMove = args.configViewModel::previewAliasEntryMove,
                    onPreviewAliasGroupMove = args.configViewModel::previewAliasGroupMove,
                    onConfirmAliasEntryMovePlan = args.configViewModel::confirmAliasEntryMovePlan,
                    onDiscardAliasEntryMovePlan = args.configViewModel::discardAliasEntryMovePlan,
                    onSaveCurrentFile = args.configViewModel::saveCurrentFile,
                    onThemeEvent = args.onThemeEvent,
                    reportPiePalettePreset = args.reportPiePalettePreset,
                    onReportPiePalettePresetChange = args.onReportPiePalettePresetChange,
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
    if (lines.size != 2) {
        return null
    }
    return TracerSnackbarVisuals(
        message = lines[0],
        supportingText = lines[1],
        duration = SnackbarDuration.Short,
        withDismissAction = true
    )
}
