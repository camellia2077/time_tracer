package com.example.tracer

import androidx.annotation.StringRes
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AddCircle
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Settings
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.UserPreferencesRepository

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
    val appLanguage: AppLanguage,
    val onSetAppLanguage: (AppLanguage) -> Unit,
    val validMappingNames: Set<String>,
    val userPreferencesRepository: UserPreferencesRepository,
    val coroutineScope: kotlinx.coroutines.CoroutineScope,
    val onImportSingleTxt: () -> Unit,
    val onExportAllMonthsTxt: () -> Unit,
    val isTxtExportInProgress: Boolean,
    val onImportAndroidConfigFullReplace: () -> Unit,
    val onImportAndroidConfigPartialUpdate: () -> Unit,
    val onExportAndroidConfigBundle: () -> Unit
)

internal data class TracerTabEntry(
    val meta: TabMeta,
    val scrollBehavior: TracerTabScrollBehavior,
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
            content = { modifier, args ->
                DataManagementSection(
                    modifier = modifier,
                    onIngestSmoke = args.dataViewModel::ingestSmoke,
                    onIngestFull = args.dataViewModel::ingestFull,
                    onImportSingleTxt = args.onImportSingleTxt,
                    canExportAllMonthsTxt = args.recordUiState.availableMonths.isNotEmpty(),
                    onExportAllMonthsTxt = args.onExportAllMonthsTxt,
                    isTxtExportInProgress = args.isTxtExportInProgress,
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
            content = { _, args ->
                QueryReportSection(
                    reportDate = args.queryUiState.reportDate,
                    onReportDateChange = args.queryReportViewModel::onReportDateChange,
                    reportMonth = args.queryUiState.reportMonth,
                    onReportMonthChange = args.queryReportViewModel::onReportMonthChange,
                    reportYear = args.queryUiState.reportYear,
                    onReportYearChange = args.queryReportViewModel::onReportYearChange,
                    reportWeek = args.queryUiState.reportWeek,
                    onReportWeekChange = args.queryReportViewModel::onReportWeekChange,
                    reportRangeStartDate = args.queryUiState.reportRangeStartDate,
                    onReportRangeStartDateChange = args.queryReportViewModel::onReportRangeStartDateChange,
                    reportRangeEndDate = args.queryUiState.reportRangeEndDate,
                    onReportRangeEndDateChange = args.queryReportViewModel::onReportRangeEndDateChange,
                    reportRecentDays = args.queryUiState.reportRecentDays,
                    onReportRecentDaysChange = args.queryReportViewModel::onReportRecentDaysChange,
                    onReportDay = args.queryReportViewModel::reportDay,
                    onReportMonth = args.queryReportViewModel::reportMonth,
                    onReportYear = args.queryReportViewModel::reportYear,
                    onReportWeek = args.queryReportViewModel::reportWeek,
                    onReportRange = args.queryReportViewModel::reportRange,
                    onReportRecent = args.queryReportViewModel::reportRecent,
                    resultDisplayMode = args.queryUiState.resultDisplayMode,
                    onResultDisplayModeChange = args.queryReportViewModel::onResultDisplayModeChange,
                    analysisLoading = args.queryUiState.analysisLoading,
                    onLoadDayStats = args.queryReportViewModel::loadDayStats,
                    onLoadTree = args.queryReportViewModel::loadTree
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
            content = { _, args ->
                RecordTabContent(
                    recordUiState = args.recordUiState,
                    recordViewModel = args.recordViewModel,
                    validMappingNames = args.validMappingNames,
                    userPreferencesRepository = args.userPreferencesRepository,
                    coroutineScope = args.coroutineScope
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
}

internal fun TracerTab.toLegacyTabIndex(): Int {
    val index = TracerTabRegistry.indexOf(this)
    return if (index >= 0) index else 0
}

internal fun tracerTabFromLegacyIndex(index: Int): TracerTab {
    return TracerTabRegistry.entries.getOrNull(index)?.meta?.id ?: DefaultTracerTab
}
