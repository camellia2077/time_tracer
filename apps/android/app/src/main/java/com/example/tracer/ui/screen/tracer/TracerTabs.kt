package com.example.tracer

import androidx.annotation.StringRes
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.BarChart
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.SnackbarDuration
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import androidx.compose.foundation.layout.padding
import com.example.tracer.feature.data.R as DataFeatureR
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.ThemeConfig
import com.example.tracer.ui.viewmodel.ThemeEvent

internal enum class TracerTab {
    INSIGHTS,
    RECORD,
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
    val queryUiState: QueryInsightsUiState,
    val queryInsightsViewModel: QueryInsightsViewModel,
    val txtStorageGateway: TxtStorageGateway,
    val recordUiState: RecordUiState,
    val recordViewModel: RecordViewModel,
    val configUiState: ConfigUiState,
    val configViewModel: ConfigViewModel,
    val themeConfig: ThemeConfig,
    val onThemeEvent: (ThemeEvent) -> Unit,
    val insightsPiePalettePreset: InsightsPiePalettePreset,
    val onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
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
    val validAuthorableEventTokens: Set<String>,
    val onPersistRecordQuickActivities: (List<String>) -> Unit,
    val onClearQuickAccessCache: () -> Unit,
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
    val selectedTracerSecurityLevel: TracerExchangeSecurityLevel,
    val onTracerSecurityLevelChange: (TracerExchangeSecurityLevel) -> Unit,
    val onCopyDiagnosticsPayload: () -> Unit,
    val onEditDailyStatuses: () -> Unit
)

internal data class TracerTabLifecycleArgs(
    val queryGateway: QueryGateway,
    val queryInsightsViewModel: QueryInsightsViewModel,
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
                id = TracerTab.INSIGHTS,
                titleRes = R.string.tracer_tab_insights,
                icon = Icons.Default.BarChart,
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
                icon = Icons.Default.Edit,
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
                id = TracerTab.CONFIG,
                titleRes = R.string.tracer_tab_config,
                icon = Icons.Default.Settings,
                testTag = "tab_config"
            ),
            scrollBehavior = TracerTabScrollBehavior.VERTICAL,
            onEnter = { args ->
                args.configViewModel.refreshConfigFiles(showStatus = false)
                refreshRecordMappingValidation(args)
            },
            onLeave = { args -> args.recordViewModel.discardUnsavedHistoryDraft() },
            statusText = { args -> args.configStatusText.ifBlank { args.dataStatusText } },
            statusEvent = { args -> defaultStatusUiEvent(args) },
            content = { modifier, args ->
                Column(
                    modifier = modifier,
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    ConfigSection(
                        selectedCategory = args.configUiState.selectedCategory,
                        aliasFiles = args.configUiState.aliasFiles,
                        chartFiles = args.configUiState.chartFiles,
                        metaFiles = args.configUiState.metaFiles,
                        insightsFiles = args.configUiState.insightsFiles,
                        selectedFilePath = args.configUiState.selectedFilePath,
                        selectedFileDisplayName = args.configUiState.selectedFileDisplayName,
                        selectedFileContent = args.configUiState.selectedFileContent,
                        editableContent = args.configUiState.editableContent,
                        aliasEditorMode = args.configUiState.aliasEditorMode,
                        aliasDocumentDraft = args.configUiState.aliasDocumentDraft,
                        aliasEntryMovePlan = args.configUiState.aliasEntryMovePlan,
                        aliasEntryMoveDestinations = args.configUiState.aliasEntryMoveDestinations,
                        aliasEntryMoveDestinationsLoading = args.configUiState.aliasEntryMoveDestinationsLoading,
                        aliasAdvancedTomlDraft = args.configUiState.aliasAdvancedTomlDraft,
                        aliasEditorErrorMessage = args.configUiState.aliasEditorErrorMessage,
                        autoSaveStatus = args.configUiState.autoSaveStatus,
                        themeConfig = args.themeConfig,
                        onSelectAlias = { args.configViewModel.selectCategory(ConfigCategory.ALIAS) },
                        onSelectCharts = { args.configViewModel.selectCategory(ConfigCategory.CHARTS) },
                        onSelectMeta = { args.configViewModel.selectCategory(ConfigCategory.META) },
                        onSelectInsights = { args.configViewModel.selectCategory(ConfigCategory.INSIGHTS) },
                        onRefreshFiles = args.configViewModel::refreshConfigFiles,
                        onOpenFile = args.configViewModel::openFile,
                        onCreateAliasTomlFile = args.configViewModel::createAliasTomlFile,
                        onDeleteAliasTomlFile = args.configViewModel::deleteCurrentAliasTomlFile,
                        onRenameAliasCategory = args.configViewModel::renameAliasCategory,
                        onCopyDiagnosticsPayload = args.onCopyDiagnosticsPayload,
                        onEditableContentChange = args.configViewModel::onEditableContentChange,
                        onSelectAliasStructuredMode = {
                            args.configViewModel.selectAliasEditorMode(AliasEditorMode.STRUCTURED)
                        },
                        onSelectAliasAdvancedMode = {
                            args.configViewModel.selectAliasEditorMode(AliasEditorMode.ADVANCED)
                        },
                        onAliasAdvancedTomlChange = args.configViewModel::onAliasAdvancedTomlChange,
                        onAddAliasGroup = args.configViewModel::addAliasGroup,
                        onDeleteAliasGroup = args.configViewModel::deleteAliasGroup,
                        onRenameAliasGroup = args.configViewModel::renameAliasGroup,
                        onAddAliasEntry = args.configViewModel::addAliasEntry,
                        onUpdateAliasEntry = args.configViewModel::updateAliasEntry,
                        onMergeAliasEntry = args.configViewModel::mergeAliasEntry,
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
                        insightsPiePalettePreset = args.insightsPiePalettePreset,
                        onInsightsPiePalettePresetChange = args.onInsightsPiePalettePresetChange,
                        insightsAverageDayBasis = args.insightsAverageDayBasis,
                        onInsightsAverageDayBasisChange = args.onInsightsAverageDayBasisChange,
                        appLanguage = args.appLanguage,
                        onSetAppLanguage = args.onSetAppLanguage,
                        extraContent = {
                            DataManagementRouteContent(args)
                            Text(
                                text = stringResource(R.string.config_title_advanced_files),
                                style = MaterialTheme.typography.titleMedium,
                                color = MaterialTheme.colorScheme.primary,
                                modifier = Modifier.padding(top = 4.dp)
                            )
                            TxtEditorRouteContent(args)
                        }
                    )
                }
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
        embeddedInScrollableParent = true,
        inlineStatusText = args.recordUiState.statusText,
        onCreateCurrentMonthTxt = args.recordViewModel::createCurrentMonthTxt
    )
}

private const val ActivityAuthorableTokenValidationUnavailablePrefix =
    "Activity authorable token validation unavailable:"

private suspend fun refreshRecordMappingValidation(args: TracerTabLifecycleArgs) {
    val mappingResult = args.queryGateway.listActivityHierarchyLeafKeys()
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
