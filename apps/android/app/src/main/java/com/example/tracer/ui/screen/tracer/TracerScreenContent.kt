package com.example.tracer

import androidx.compose.animation.AnimatedContent
import androidx.compose.animation.SizeTransform
import androidx.compose.animation.fadeIn
import androidx.compose.animation.fadeOut
import androidx.compose.animation.slideInHorizontally
import androidx.compose.animation.slideOutHorizontally
import androidx.compose.animation.togetherWith
import androidx.compose.animation.core.tween
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.material3.SnackbarHostState
import androidx.compose.runtime.Composable
import androidx.compose.runtime.saveable.rememberSaveableStateHolder
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp

@Composable
internal fun TracerScreenContent(
    selectedTab: TracerTab,
    snackbarHostState: SnackbarHostState,
    onCoordinatorEvent: (TracerCoordinatorEvent) -> Unit,
    dataViewModel: DataViewModel,
    queryUiState: QueryInsightsUiState,
    queryInsightsViewModel: QueryInsightsViewModel,
    txtStorageGateway: TxtStorageGateway,
    recordUiState: RecordUiState,
    recordViewModel: RecordViewModel,
    themeConfig: com.example.tracer.data.ThemeConfig,
    onThemeEvent: (com.example.tracer.ui.viewmodel.ThemeEvent) -> Unit,
    insightsPiePalettePreset: InsightsPiePalettePreset,
    onInsightsPiePalettePresetChange: (InsightsPiePalettePreset) -> Unit,
    insightsComparisonColorScheme: InsightsComparisonColorScheme,
    onInsightsComparisonColorSchemeChange: (InsightsComparisonColorScheme) -> Unit,
    insightsComparisonIndicatorStyle: InsightsComparisonIndicatorStyle,
    onInsightsComparisonIndicatorStyleChange: (InsightsComparisonIndicatorStyle) -> Unit,
    insightsChartShowAverageLine: Boolean,
    onInsightsChartShowAverageLineChange: (Boolean) -> Unit,
    insightsChartSemanticMode: InsightsChartSemanticMode,
    onInsightsChartSemanticModeChange: (InsightsChartSemanticMode) -> Unit,
    insightsChartVisualMode: InsightsChartVisualMode,
    onInsightsChartVisualModeChange: (InsightsChartVisualMode) -> Unit,
    insightsChartTrendRoot: String,
    onInsightsChartTrendRootChange: (String) -> Unit,
    insightsAverageDayBasis: InsightsAverageDayBasis,
    onInsightsAverageDayBasisChange: (InsightsAverageDayBasis) -> Unit,
    insightsMode: InsightsMode,
    onInsightsModeChange: (InsightsMode) -> Unit,
    insightsResultDisplayMode: InsightsResultDisplayMode,
    onInsightsResultDisplayModeChange: (InsightsResultDisplayMode) -> Unit,
    insightsParameterSection: InsightsParameterSection,
    onInsightsParameterSectionChange: (InsightsParameterSection) -> Unit,
    insightsDayActivitiesView: InsightsActivityView,
    insightsPeriodActivitiesView: InsightsActivityView,
    onInsightsDayActivitiesViewChange: (InsightsActivityView) -> Unit,
    onInsightsPeriodActivitiesViewChange: (InsightsActivityView) -> Unit,
    insightsTimeParametersExpanded: Boolean,
    onInsightsTimeParametersExpandedChange: (Boolean) -> Unit,
    insightsHeatmapTomlConfig: InsightsHeatmapTomlConfig,
    insightsHeatmapStylePreference: InsightsHeatmapStylePreference,
    onInsightsHeatmapThemePolicyChange: (InsightsHeatmapThemePolicy) -> Unit,
    onInsightsHeatmapPaletteNameChange: (String) -> Unit,
    insightsHeatmapApplyMessage: String,
    isAppDarkThemeActive: Boolean,
    appLanguage: com.example.tracer.data.AppLanguage,
    onSetAppLanguage: (com.example.tracer.data.AppLanguage) -> Unit,
    configCardExpansionPreferences: com.example.tracer.data.ConfigCardExpansionPreferences,
    onPersistConfigCardExpanded: (com.example.tracer.data.ConfigCard, Boolean) -> Unit,
    promptBeforeUnconfiguredActivityRecord: Boolean,
    onPromptBeforeUnconfiguredActivityRecordChange: (Boolean) -> Unit,
    pageTransitionsEnabled: Boolean,
    onPageTransitionsEnabledChange: (Boolean) -> Unit,
    pageTransitionStyle: com.example.tracer.data.PageTransitionStyle,
    onPageTransitionStyleChange: (com.example.tracer.data.PageTransitionStyle) -> Unit,
    validAuthorableEventTokens: Set<String>,
    onPersistRecordQuickActivities: (List<String>) -> Unit,
    onClearQuickAccessCache: () -> Unit,
    onPersistRecordQuickAccessCardExpanded: (Boolean) -> Unit,
    onPersistRecordQuickAccessEditorVisibility: (Boolean) -> Unit,
    onPersistRecordCanonicalCatalogDisplayMode: (RecordFrequentOutputMode) -> Unit,
    onPersistRecordCanonicalCatalogSource: (CanonicalCatalogSource) -> Unit,
    onPersistRecordCollapsedCanonicalRootPaths: (Set<String>) -> Unit,
    onPersistRecordOrderedCanonicalRootPaths: (List<String>) -> Unit,
    onPersistRecordFrequentLookbackDays: (Int) -> Unit,
    onPersistRecordFrequentOutputMode: (RecordFrequentOutputMode) -> Unit,
    onPersistRecordFrequentTopN: (Int) -> Unit,
    activityCategoriesContent: @Composable () -> Unit,
    onImportDataFolder: () -> Unit,
    onImportSingleTracer: () -> Unit,
    onExportAllMonthsTracer: () -> Unit,
    onExportCurrentTxtTracer: () -> Unit,
    isTracerExportInProgress: Boolean,
    selectedTracerSecurityLevel: TracerExchangeSecurityLevel,
    onTracerSecurityLevelChange: (TracerExchangeSecurityLevel) -> Unit,
    onCopyDiagnosticsPayload: () -> Unit,
    onEditDailyStatuses: () -> Unit
) {
    val tabStateHolder = rememberSaveableStateHolder()
    TracerBottomNavShell(
        selectedTab = selectedTab,
        onTabSelected = { nextTab ->
            onCoordinatorEvent(TracerCoordinatorEvent.SelectTab(nextTab))
        },
        snackbarHostState = snackbarHostState
    ) { innerPadding ->
        val renderTabContent: @Composable (TracerTab) -> Unit = { displayedTab ->
            tabStateHolder.SaveableStateProvider(displayedTab) {
                val tabContentModifier = Modifier.tracerTabContentModifier(displayedTab, innerPadding)
                Column(
                    modifier = tabContentModifier,
                    verticalArrangement = Arrangement.spacedBy(12.dp)
                ) {
                    TracerTabRouteContent(
                modifier = Modifier,
                selectedTab = displayedTab,
                dataViewModel = dataViewModel,
                queryUiState = queryUiState,
                queryInsightsViewModel = queryInsightsViewModel,
                txtStorageGateway = txtStorageGateway,
                recordUiState = recordUiState,
                recordViewModel = recordViewModel,
                themeConfig = themeConfig,
                onThemeEvent = onThemeEvent,
                insightsPiePalettePreset = insightsPiePalettePreset,
                onInsightsPiePalettePresetChange = onInsightsPiePalettePresetChange,
                insightsComparisonColorScheme = insightsComparisonColorScheme,
                onInsightsComparisonColorSchemeChange = onInsightsComparisonColorSchemeChange,
                insightsComparisonIndicatorStyle = insightsComparisonIndicatorStyle,
                onInsightsComparisonIndicatorStyleChange = onInsightsComparisonIndicatorStyleChange,
                insightsChartShowAverageLine = insightsChartShowAverageLine,
                onInsightsChartShowAverageLineChange = onInsightsChartShowAverageLineChange,
                insightsChartSemanticMode = insightsChartSemanticMode,
                onInsightsChartSemanticModeChange = onInsightsChartSemanticModeChange,
                insightsChartVisualMode = insightsChartVisualMode,
                onInsightsChartVisualModeChange = onInsightsChartVisualModeChange,
                insightsChartTrendRoot = insightsChartTrendRoot,
                onInsightsChartTrendRootChange = onInsightsChartTrendRootChange,
                insightsAverageDayBasis = insightsAverageDayBasis,
                onInsightsAverageDayBasisChange = onInsightsAverageDayBasisChange,
                insightsMode = insightsMode,
                onInsightsModeChange = onInsightsModeChange,
                insightsResultDisplayMode = insightsResultDisplayMode,
                onInsightsResultDisplayModeChange = onInsightsResultDisplayModeChange,
                insightsParameterSection = insightsParameterSection,
                onInsightsParameterSectionChange = onInsightsParameterSectionChange,
                insightsDayActivitiesView = insightsDayActivitiesView,
                insightsPeriodActivitiesView = insightsPeriodActivitiesView,
                onInsightsDayActivitiesViewChange = onInsightsDayActivitiesViewChange,
                onInsightsPeriodActivitiesViewChange = onInsightsPeriodActivitiesViewChange,
                insightsTimeParametersExpanded = insightsTimeParametersExpanded,
                onInsightsTimeParametersExpandedChange = onInsightsTimeParametersExpandedChange,
                insightsHeatmapTomlConfig = insightsHeatmapTomlConfig,
                insightsHeatmapStylePreference = insightsHeatmapStylePreference,
                onInsightsHeatmapThemePolicyChange = onInsightsHeatmapThemePolicyChange,
                onInsightsHeatmapPaletteNameChange = onInsightsHeatmapPaletteNameChange,
                insightsHeatmapApplyMessage = insightsHeatmapApplyMessage,
                isAppDarkThemeActive = isAppDarkThemeActive,
                appLanguage = appLanguage,
                onSetAppLanguage = onSetAppLanguage,
                configCardExpansionPreferences = configCardExpansionPreferences,
                onPersistConfigCardExpanded = onPersistConfigCardExpanded,
                promptBeforeUnconfiguredActivityRecord = promptBeforeUnconfiguredActivityRecord,
                onPromptBeforeUnconfiguredActivityRecordChange =
                    onPromptBeforeUnconfiguredActivityRecordChange,
                pageTransitionsEnabled = pageTransitionsEnabled,
                onPageTransitionsEnabledChange = onPageTransitionsEnabledChange,
                pageTransitionStyle = pageTransitionStyle,
                onPageTransitionStyleChange = onPageTransitionStyleChange,
                validAuthorableEventTokens = validAuthorableEventTokens,
                onPersistRecordQuickActivities = onPersistRecordQuickActivities,
                onClearQuickAccessCache = onClearQuickAccessCache,
                onPersistRecordQuickAccessCardExpanded =
                    onPersistRecordQuickAccessCardExpanded,
                onPersistRecordQuickAccessEditorVisibility = onPersistRecordQuickAccessEditorVisibility,
                onPersistRecordCanonicalCatalogDisplayMode =
                    onPersistRecordCanonicalCatalogDisplayMode,
                onPersistRecordCanonicalCatalogSource = onPersistRecordCanonicalCatalogSource,
                onPersistRecordCollapsedCanonicalRootPaths =
                    onPersistRecordCollapsedCanonicalRootPaths,
                onPersistRecordOrderedCanonicalRootPaths =
                    onPersistRecordOrderedCanonicalRootPaths,
                onPersistRecordFrequentLookbackDays = onPersistRecordFrequentLookbackDays,
                onPersistRecordFrequentOutputMode = onPersistRecordFrequentOutputMode,
                onPersistRecordFrequentTopN = onPersistRecordFrequentTopN,
                activityCategoriesContent = activityCategoriesContent,
                onImportDataFolder = onImportDataFolder,
                onImportSingleTracer = onImportSingleTracer,
                onExportAllMonthsTracer = onExportAllMonthsTracer,
                onExportCurrentTxtTracer = onExportCurrentTxtTracer,
                isTracerExportInProgress = isTracerExportInProgress,
                selectedTracerSecurityLevel = selectedTracerSecurityLevel,
                onTracerSecurityLevelChange = onTracerSecurityLevelChange,
                onCopyDiagnosticsPayload = onCopyDiagnosticsPayload,
                onEditDailyStatuses = onEditDailyStatuses
                    )
                }
            }
        }
        if (!pageTransitionsEnabled) {
            // Compose only the selected route, as the pre-animation implementation did. Keeping
            // it out of AnimatedContent prevents transient outgoing-content frames from flashing.
            renderTabContent(selectedTab)
        } else {
            AnimatedContent(
                targetState = selectedTab,
                transitionSpec = {
                    when (pageTransitionStyle) {
                        com.example.tracer.data.PageTransitionStyle.FADE -> {
                            fadeIn(animationSpec = tween(durationMillis = 140, delayMillis = 40))
                                .togetherWith(fadeOut(animationSpec = tween(durationMillis = 100)))
                        }
                        com.example.tracer.data.PageTransitionStyle.SLIDE -> {
                            val movingForward = TracerTabRegistry.indexOf(targetState) >
                                TracerTabRegistry.indexOf(initialState)
                            val enterOffset: (Int) -> Int = { width ->
                                if (movingForward) width / 16 else -width / 16
                            }
                            val exitOffset: (Int) -> Int = { width ->
                                if (movingForward) -width / 16 else width / 16
                            }
                            (slideInHorizontally(animationSpec = tween(120), initialOffsetX = enterOffset) +
                                fadeIn(animationSpec = tween(120)))
                                .togetherWith(
                                    slideOutHorizontally(
                                        animationSpec = tween(80),
                                        targetOffsetX = exitOffset
                                    ) + fadeOut(animationSpec = tween(80))
                                )
                        }
                    }
                        .using(SizeTransform(clip = false) { _, _ -> tween(durationMillis = 0) })
                },
                label = "main_tab_content"
            ) { displayedTab ->
                renderTabContent(displayedTab)
            }
        }
    }
}
