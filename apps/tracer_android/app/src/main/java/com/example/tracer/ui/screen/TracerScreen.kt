package com.example.tracer

import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.AddCircle
import androidx.compose.material.icons.filled.DateRange
import androidx.compose.material.icons.filled.Edit
import androidx.compose.material.icons.filled.Home
import androidx.compose.material.icons.filled.Settings
import androidx.compose.material3.Icon
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.NavigationBar
import androidx.compose.material3.NavigationBarItem
import androidx.compose.material3.NavigationBarItemDefaults
import androidx.compose.material3.Scaffold
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.collectAsState
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.rememberCoroutineScope
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.Dp
import androidx.compose.ui.unit.dp
import androidx.lifecycle.viewmodel.compose.viewModel
import com.mikepenz.markdown.m3.Markdown
import kotlinx.coroutines.launch

private data class MainTabItem(
    val title: String,
    val icon: ImageVector
)

private val MAIN_TABS = listOf(
    MainTabItem(title = "Data", icon = Icons.Default.Home),
    MainTabItem(title = "Report", icon = Icons.Default.DateRange),
    MainTabItem(title = "Record", icon = Icons.Default.AddCircle),
    MainTabItem(title = "TXT", icon = Icons.Default.Edit),
    MainTabItem(title = "Config", icon = Icons.Default.Settings)
)

private val ScreenOuterPadding: Dp = 16.dp

private fun treeResultTitle(period: DataTreePeriod): String {
    return when (period) {
        DataTreePeriod.DAY -> "Day Tree Result"
        DataTreePeriod.WEEK -> "Week Tree Result"
        DataTreePeriod.MONTH -> "Month Tree Result"
        DataTreePeriod.YEAR -> "Year Tree Result"
        DataTreePeriod.RECENT -> "Recent Tree Result"
        DataTreePeriod.RANGE -> "Range Tree Result"
    }
}

private fun statsResultTitle(period: DataTreePeriod): String {
    return when (period) {
        DataTreePeriod.DAY -> "Day Stats Result"
        DataTreePeriod.WEEK -> "Week Stats Result"
        DataTreePeriod.MONTH -> "Month Stats Result"
        DataTreePeriod.YEAR -> "Year Stats Result"
        DataTreePeriod.RECENT -> "Recent Stats Result"
        DataTreePeriod.RANGE -> "Range Stats Result"
    }
}

@Composable
fun TracerScreen(
    runtimeInitializer: RuntimeInitializer,
    recordGateway: RecordGateway,
    txtStorageGateway: TxtStorageGateway,
    reportGateway: ReportGateway,
    queryGateway: QueryGateway,
    controller: RuntimeGateway,
    userPreferencesRepository: com.example.tracer.data.UserPreferencesRepository,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onSetThemeColor: (com.example.tracer.data.ThemeColor) -> Unit,
    onSetThemeMode: (com.example.tracer.data.ThemeMode) -> Unit,
    onSetUseDynamicColor: (Boolean) -> Unit,
    onSetDarkThemeStyle: (com.example.tracer.data.DarkThemeStyle) -> Unit
) {
    var selectedTab by remember { mutableStateOf(2) }
    var validMappingNames by remember { mutableStateOf<Set<String>>(emptySet()) }
    val coroutineScope = rememberCoroutineScope()
    val dataViewModel: DataViewModel = viewModel(
        factory = remember(runtimeInitializer, recordGateway) {
            DataViewModelFactory(runtimeInitializer, recordGateway)
        }
    )
    val queryReportViewModel: QueryReportViewModel = viewModel(
        factory = remember(reportGateway, queryGateway) {
            QueryReportViewModelFactory(reportGateway, queryGateway)
        }
    )
    val recordViewModel: RecordViewModel = viewModel(
        factory = remember(recordGateway, txtStorageGateway, queryGateway) {
            RecordViewModelFactory(recordGateway, txtStorageGateway, queryGateway)
        }
    )
    val configViewModel: ConfigViewModel = viewModel(
        factory = remember(controller) {
            ConfigViewModelFactory(controller)
        }
    )

    val dataUiState = dataViewModel.uiState
    val queryUiState = queryReportViewModel.uiState
    val recordUiState = recordViewModel.uiState
    val configUiState = configViewModel.uiState
    val recordSuggestionPreferences by userPreferencesRepository.recordSuggestionPreferences.collectAsState(
        initial = com.example.tracer.data.RecordSuggestionPreferences(
            lookbackDays = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_SUGGEST_LOOKBACK_DAYS,
            topN = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_SUGGEST_TOP_N,
            quickActivities = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_QUICK_ACTIVITIES,
            assistExpanded = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_ASSIST_EXPANDED,
            assistSettingsExpanded = com.example.tracer.data.UserPreferencesRepository.DEFAULT_RECORD_ASSIST_SETTINGS_EXPANDED
        )
    )

    LaunchedEffect(
        recordSuggestionPreferences.lookbackDays,
        recordSuggestionPreferences.topN,
        recordSuggestionPreferences.quickActivities,
        recordSuggestionPreferences.assistExpanded,
        recordSuggestionPreferences.assistSettingsExpanded
    ) {
        recordViewModel.updateSuggestionPreferences(
            lookbackDays = recordSuggestionPreferences.lookbackDays,
            topN = recordSuggestionPreferences.topN
        )
        recordViewModel.updateQuickActivities(recordSuggestionPreferences.quickActivities)
        recordViewModel.updateAssistUiState(
            assistExpanded = recordSuggestionPreferences.assistExpanded,
            assistSettingsExpanded = recordSuggestionPreferences.assistSettingsExpanded
        )
    }

    LaunchedEffect(controller) {
        val mappingResult = controller.listActivityMappingNames()
        if (mappingResult.ok) {
            validMappingNames = mappingResult.names.toSet()
        } else {
            validMappingNames = emptySet()
            recordViewModel.setStatusText(
                "Activity mapping validation unavailable: ${mappingResult.message}"
            )
        }
    }

    val statusText = when (selectedTab) {
        0 -> dataUiState.statusText
        1 -> queryUiState.statusText
        2 -> recordUiState.statusText
        3 -> recordUiState.statusText
        else -> configUiState.statusText
    }

    Scaffold(
        bottomBar = {
            val navItemColors = NavigationBarItemDefaults.colors(
                selectedIconColor = MaterialTheme.colorScheme.onPrimary,
                selectedTextColor = MaterialTheme.colorScheme.primary,
                unselectedIconColor = MaterialTheme.colorScheme.onSurfaceVariant,
                unselectedTextColor = MaterialTheme.colorScheme.onSurfaceVariant,
                indicatorColor = MaterialTheme.colorScheme.primary
            )

            NavigationBar(
                containerColor = MaterialTheme.colorScheme.surfaceContainer,
                tonalElevation = 0.dp
            ) {
                MAIN_TABS.forEachIndexed { index, tab ->
                    val isSelected = selectedTab == index
                    NavigationBarItem(
                        selected = isSelected,
                        onClick = { selectedTab = index },
                        icon = { Icon(tab.icon, contentDescription = tab.title) },
                        label = {
                            Text(
                                text = tab.title,
                                style = MaterialTheme.typography.labelMedium.copy(
                                    fontWeight = if (isSelected) FontWeight.SemiBold else FontWeight.Medium
                                )
                            )
                        },
                        alwaysShowLabel = true,
                        colors = navItemColors
                    )
                }
            }
        }
    ) { innerPadding ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(innerPadding)
                .padding(
                    start = ScreenOuterPadding,
                    end = ScreenOuterPadding,
                    bottom = ScreenOuterPadding
                )
                .verticalScroll(rememberScrollState()),
            verticalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            // Debug headers removed for cleaner UI
            // Text("Time Tracer Android JNI Smoke", ...)
            // Text("initialized: ...", ...)

            if (selectedTab == 0) {
                DataManagementSection(
                    onInit = dataViewModel::initializeRuntime,
                    onIngestSmoke = dataViewModel::ingestSmoke,
                    onIngestFull = dataViewModel::ingestFull,
                    onQuerySmoke = dataViewModel::querySmoke,
                    onClearLiveTxt = {
                        dataViewModel.clearLiveTxt()
                        recordViewModel.clearLiveTxtEditorState()
                    },
                    onClearData = dataViewModel::clearDataAndReinitialize
                )
            } else if (selectedTab == 1) {
                QueryReportSection(
                    reportDate = queryUiState.reportDate,
                    onReportDateChange = queryReportViewModel::onReportDateChange,
                    reportMonth = queryUiState.reportMonth,
                    onReportMonthChange = queryReportViewModel::onReportMonthChange,
                    reportYear = queryUiState.reportYear,
                    onReportYearChange = queryReportViewModel::onReportYearChange,
                    reportWeek = queryUiState.reportWeek,
                    onReportWeekChange = queryReportViewModel::onReportWeekChange,
                    reportRangeStartDate = queryUiState.reportRangeStartDate,
                    onReportRangeStartDateChange = queryReportViewModel::onReportRangeStartDateChange,
                    reportRangeEndDate = queryUiState.reportRangeEndDate,
                    onReportRangeEndDateChange = queryReportViewModel::onReportRangeEndDateChange,
                    reportRecentDays = queryUiState.reportRecentDays,
                    onReportRecentDaysChange = queryReportViewModel::onReportRecentDaysChange,
                    onReportDay = queryReportViewModel::reportDay,
                    onReportMonth = queryReportViewModel::reportMonth,
                    onReportYear = queryReportViewModel::reportYear,
                    onReportWeek = queryReportViewModel::reportWeek,
                    onReportRange = queryReportViewModel::reportRange,
                    onReportRecent = queryReportViewModel::reportRecent,
                    analysisLoading = queryUiState.analysisLoading,
                    onLoadDayStats = queryReportViewModel::loadDayStats,
                    onLoadTree = queryReportViewModel::loadTree
                )
            } else if (selectedTab == 2) {
                RecordSection(
                    recordContent = recordUiState.recordContent,
                    onRecordContentChange = recordViewModel::onRecordContentChange,
                    recordRemark = recordUiState.recordRemark,
                    onRecordRemarkChange = recordViewModel::onRecordRemarkChange,
                    quickActivities = recordUiState.quickActivities,
                    availableActivityNames = remember(validMappingNames) { validMappingNames.toList().sorted() },
                    onQuickActivitiesUpdate = { targetActivities ->
                        if (validMappingNames.isEmpty()) {
                            recordViewModel.setStatusText(
                                "Cannot save quick activities: mapping_config.toml validation set is empty."
                            )
                            return@RecordSection
                        }
                        val normalized = targetActivities
                            .map { it.trim() }
                            .filter { it.isNotEmpty() }
                            .distinct()
                        if (normalized.isEmpty()) {
                            recordViewModel.setStatusText(
                                "Quick activities cannot be empty."
                            )
                            return@RecordSection
                        }
                        if (normalized.size > 12) {
                            recordViewModel.setStatusText(
                                "Quick activities exceed limit: 12."
                            )
                            return@RecordSection
                        }
                        val invalidActivities = normalized.filter { !validMappingNames.contains(it) }
                        if (invalidActivities.isNotEmpty()) {
                            recordViewModel.setStatusText(
                                "Invalid quick activities: ${invalidActivities.joinToString(", ")}. " +
                                    "Must match mapping_config.toml [text_mappings] key or value."
                            )
                            return@RecordSection
                        }
                        recordViewModel.updateQuickActivities(normalized)
                        recordViewModel.setStatusText("Quick activities saved (${normalized.size}).")
                        coroutineScope.launch {
                            userPreferencesRepository.setRecordQuickActivities(normalized)
                        }
                    },
                    assistExpanded = recordUiState.assistExpanded,
                    assistSettingsExpanded = recordUiState.assistSettingsExpanded,
                    onToggleAssist = {
                        val nextValue = !recordUiState.assistExpanded
                        recordViewModel.updateAssistUiState(
                            assistExpanded = nextValue,
                            assistSettingsExpanded = recordUiState.assistSettingsExpanded
                        )
                        coroutineScope.launch {
                            userPreferencesRepository.setRecordAssistExpanded(nextValue)
                        }
                    },
                    onToggleAssistSettings = {
                        val nextValue = !recordUiState.assistSettingsExpanded
                        recordViewModel.updateAssistUiState(
                            assistExpanded = recordUiState.assistExpanded,
                            assistSettingsExpanded = nextValue
                        )
                        coroutineScope.launch {
                            userPreferencesRepository.setRecordAssistSettingsExpanded(nextValue)
                        }
                    },
                    suggestionLookbackDays = recordUiState.suggestionLookbackDays,
                    suggestionTopN = recordUiState.suggestionTopN,
                    onSuggestionLookbackDaysChange = { rawValue ->
                        val parsed = rawValue.trim().toIntOrNull()
                        if (parsed == null || parsed <= 0) {
                            return@RecordSection
                        }
                        recordViewModel.updateSuggestionPreferences(
                            lookbackDays = parsed,
                            topN = recordUiState.suggestionTopN
                        )
                        coroutineScope.launch {
                            userPreferencesRepository.setRecordSuggestLookbackDays(parsed)
                        }
                    },
                    onSuggestionTopNChange = { rawValue ->
                        val parsed = rawValue.trim().toIntOrNull()
                        if (parsed == null || parsed <= 0) {
                            return@RecordSection
                        }
                        recordViewModel.updateSuggestionPreferences(
                            lookbackDays = recordUiState.suggestionLookbackDays,
                            topN = parsed
                        )
                        coroutineScope.launch {
                            userPreferencesRepository.setRecordSuggestTopN(parsed)
                        }
                    },
                    suggestedActivities = recordUiState.suggestedActivities,
                    suggestionsVisible = recordUiState.suggestionsVisible,
                    isSuggestionsLoading = recordUiState.isSuggestionsLoading,
                    useManualDate = recordUiState.useManualDate,
                    manualDate = recordUiState.manualDate,
                    onUseAutoDate = recordViewModel::useAutoDate,
                    onUseManualDate = recordViewModel::useManualDate,
                    onManualDateChange = recordViewModel::onManualDateChange,
                    onToggleSuggestions = recordViewModel::toggleSuggestions,
                    onSuggestedActivityClick = { activity ->
                        if (validMappingNames.isNotEmpty() && !validMappingNames.contains(activity)) {
                            recordViewModel.setStatusText(
                                "Invalid suggested activity: $activity. " +
                                    "Must match mapping_config.toml [text_mappings] key or value."
                            )
                            return@RecordSection
                        }
                        recordViewModel.applySuggestedActivity(activity)
                    },
                    onRecordNow = recordViewModel::recordNow
                )
            } else if (selectedTab == 3) {
                TxtEditorSection(
                    availableMonths = recordUiState.availableMonths,
                    selectedMonth = recordUiState.selectedMonth,
                    onOpenPreviousMonth = recordViewModel::openPreviousMonth,
                    onOpenNextMonth = recordViewModel::openNextMonth,
                    onOpenMonth = recordViewModel::openMonth,
                    onCreateCurrentMonthTxt = recordViewModel::createCurrentMonthTxt,
                    selectedHistoryFile = recordUiState.selectedHistoryFile,
                    onRefreshHistory = recordViewModel::refreshHistory,
                    editableHistoryContent = recordUiState.editableHistoryContent,
                    onEditableHistoryContentChange = recordViewModel::updateEditableHistoryContent,
                    onSaveHistoryFile = recordViewModel::saveHistoryFileAndSync,
                    inlineStatusText = recordUiState.statusText
                )
            } else {
                ConfigSection(
                    selectedCategory = configUiState.selectedCategory,
                    converterFiles = configUiState.converterFiles,
                    reportFiles = configUiState.reportFiles,
                    selectedFile = configUiState.selectedFile,
                    editableContent = configUiState.editableContent,
                    themeConfig = themeConfig,
                    onSelectConverter = { configViewModel.selectCategory(ConfigCategory.CONVERTER) },
                    onSelectReports = { configViewModel.selectCategory(ConfigCategory.REPORTS) },
                    onRefreshFiles = configViewModel::refreshConfigFiles,
                    onOpenFile = configViewModel::openFile,
                    onEditableContentChange = configViewModel::onEditableContentChange,
                    onSaveCurrentFile = configViewModel::saveCurrentFile,
                    onSetThemeColor = onSetThemeColor,
                    onSetThemeMode = onSetThemeMode,
                    onSetUseDynamicColor = onSetUseDynamicColor,
                    onSetDarkThemeStyle = onSetDarkThemeStyle
                )
            }

            Spacer(modifier = Modifier.height(8.dp))

            Text(
                text = statusText,
                style = MaterialTheme.typography.bodySmall,
                modifier = Modifier.fillMaxWidth()
            )

            val activeResult = queryUiState.activeResult
            if (selectedTab == 1 && activeResult != null) {
                androidx.compose.material3.ElevatedCard(
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Column(modifier = Modifier.padding(16.dp)) {
                        when (activeResult) {
                            is QueryResult.Report -> {
                                Text(
                                    text = "Report Result",
                                    style = MaterialTheme.typography.titleMedium,
                                    color = MaterialTheme.colorScheme.primary
                                )
                                Spacer(modifier = Modifier.height(8.dp))
                                com.example.tracer.ui.components.MarkdownText(
                                    markdown = activeResult.text,
                                    modifier = Modifier.fillMaxWidth()
                                )
                            }

                            is QueryResult.Stats -> {
                                Text(
                                    text = statsResultTitle(activeResult.period),
                                    style = MaterialTheme.typography.titleMedium,
                                    color = MaterialTheme.colorScheme.primary
                                )
                                Spacer(modifier = Modifier.height(8.dp))
                                com.example.tracer.ui.components.MarkdownText(
                                    markdown = activeResult.text,
                                    modifier = Modifier.fillMaxWidth()
                                )
                            }

                            is QueryResult.Tree -> {
                                Text(
                                    text = treeResultTitle(activeResult.period),
                                    style = MaterialTheme.typography.titleMedium,
                                    color = MaterialTheme.colorScheme.primary
                                )
                                Spacer(modifier = Modifier.height(8.dp))
                                Text(
                                    text = activeResult.text,
                                    style = MaterialTheme.typography.bodyMedium,
                                    fontFamily = FontFamily.Monospace,
                                    modifier = Modifier.fillMaxWidth()
                                )
                            }
                        }
                    }
                }
            }
            if (selectedTab == 1 && queryUiState.analysisError.isNotBlank()) {
                androidx.compose.material3.ElevatedCard(
                    modifier = Modifier.fillMaxWidth()
                ) {
                    Column(modifier = Modifier.padding(16.dp)) {
                        Text(
                            text = "Analysis Error",
                            style = MaterialTheme.typography.titleMedium,
                            color = MaterialTheme.colorScheme.error
                        )
                        Spacer(modifier = Modifier.height(8.dp))
                        Text(
                            text = queryUiState.analysisError,
                            style = MaterialTheme.typography.bodyMedium,
                            modifier = Modifier.fillMaxWidth()
                        )
                    }
                }
            }
        }
    }
}
