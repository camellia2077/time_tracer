@file:Suppress("TooManyFunctions")

package com.example.tracer.data

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.intPreferencesKey
import androidx.datastore.preferences.core.longPreferencesKey
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import com.example.tracer.PersistedRecordInputDraft
import com.example.tracer.PersistedRecordInputSnapshot
import com.example.tracer.RecordAuthoringMode

import com.example.tracer.TxtOutputMode
import com.example.tracer.RecordLogicalDayTarget
import com.example.tracer.RecordFrequentOutputMode
import com.example.tracer.CanonicalCatalogSource
import com.example.tracer.InsightsChartSemanticMode
import com.example.tracer.InsightsChartVisualMode
import com.example.tracer.InsightsAverageDayBasis
import com.example.tracer.InsightsComparisonColorScheme
import com.example.tracer.InsightsComparisonIndicatorStyle
import com.example.tracer.InsightsParameterSection
import com.example.tracer.InsightsPiePalettePreset
import com.example.tracer.InsightsResultDisplayMode
import com.example.tracer.InsightsMode
import com.example.tracer.InsightsActivityView
import com.example.tracer.defaultInsightsPiePalettePreset
import com.example.tracer.defaultInsightsComparisonColorScheme
import com.example.tracer.defaultInsightsComparisonIndicatorStyle
import java.nio.charset.StandardCharsets
import java.util.Base64
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

private const val DAILY_STATUS_FIELD_COUNT = 3

val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "settings")

enum class ThemeMode {
    System, Light, Dark
}

enum class DarkThemeStyle {
    Tinted, Neutral, Black
}

enum class ThemePalette(val supportsLightDarkMode: Boolean) {
    Indigo(true), GraphiteAmber(true), Teal(true), Orange(true), Rose(true), Amber(true),
    Parchment(false), Snowfield(false), Blueprint(false), Newsprint(false), InkWash(false), Kraft(false)
}

enum class AppLanguage {
    System,
    Chinese,
    English,
    Japanese
}

enum class PageTransitionStyle {
    FADE,
    SLIDE
}

data class ThemeConfig(
    val themeMode: ThemeMode,
    val darkThemeStyle: DarkThemeStyle = DarkThemeStyle.Tinted,
    val palette: ThemePalette = ThemePalette.Indigo
)

data class RecordFrequentPreferences(
    val lookbackDays: Int,
    val topN: Int,
    val outputMode: RecordFrequentOutputMode,
    val canonicalCatalogDisplayMode: RecordFrequentOutputMode,
    val canonicalCatalogSource: CanonicalCatalogSource,
    val quickActivities: List<String>,
    val quickAccessCardExpanded: Boolean,
    val quickAccessEditorVisible: Boolean,
    val collapsedCanonicalRootPaths: Set<String>,
    val orderedCanonicalRootPaths: List<String>
)

enum class ConfigCard {
    APPLICATION_PREFERENCES,
    APPEARANCE,
    THEME_PALETTE,
    INSIGHTS_SETTINGS,
    INSIGHTS_CHART_STYLE,
    INSIGHTS_COMPARISON,
    DATA_MANAGEMENT,
    ABOUT
}

data class ConfigCardExpansionPreferences(
    val applicationPreferencesExpanded: Boolean,
    val appearanceExpanded: Boolean,
    val themePaletteExpanded: Boolean = false,
    val insightsSettingsExpanded: Boolean,
    val insightsChartStyleExpanded: Boolean = false,
    val insightsComparisonExpanded: Boolean = false,
    val dataManagementExpanded: Boolean,
    val aboutExpanded: Boolean
) {
    fun isExpanded(card: ConfigCard): Boolean = when (card) {
        ConfigCard.APPLICATION_PREFERENCES -> applicationPreferencesExpanded
        ConfigCard.APPEARANCE -> appearanceExpanded
        ConfigCard.THEME_PALETTE -> themePaletteExpanded
        ConfigCard.INSIGHTS_SETTINGS -> insightsSettingsExpanded
        ConfigCard.INSIGHTS_CHART_STYLE -> insightsChartStyleExpanded
        ConfigCard.INSIGHTS_COMPARISON -> insightsComparisonExpanded
        ConfigCard.DATA_MANAGEMENT -> dataManagementExpanded
        ConfigCard.ABOUT -> aboutExpanded
    }
}

class UserPreferencesRepository(private val dataStore: DataStore<Preferences>) {
    companion object {
        const val DEFAULT_RECORD_FREQUENT_LOOKBACK_DAYS: Int = 7
        const val DEFAULT_RECORD_FREQUENT_TOP_N: Int = 5
        val DEFAULT_RECORD_FREQUENT_OUTPUT_MODE: RecordFrequentOutputMode =
            RecordFrequentOutputMode.CANONICAL
        val DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE: RecordFrequentOutputMode =
            RecordFrequentOutputMode.CANONICAL
        val DEFAULT_RECORD_CANONICAL_CATALOG_SOURCE: CanonicalCatalogSource =
            CanonicalCatalogSource.TREE
        val DEFAULT_RECORD_QUICK_ACTIVITIES: List<String> = emptyList()
        const val DEFAULT_INSIGHTS_CHART_SHOW_AVERAGE_LINE: Boolean = false
        val DEFAULT_INSIGHTS_CHART_SEMANTIC_MODE: InsightsChartSemanticMode =
            InsightsChartSemanticMode.COMPOSITION
        val DEFAULT_INSIGHTS_CHART_VISUAL_MODE: InsightsChartVisualMode =
            InsightsChartVisualMode.LINE
        val DEFAULT_INSIGHTS_MODE: InsightsMode = InsightsMode.DAY
        val DEFAULT_INSIGHTS_RESULT_DISPLAY_MODE: InsightsResultDisplayMode =
            InsightsResultDisplayMode.DETAILS
        val DEFAULT_INSIGHTS_PARAMETER_SECTION: InsightsParameterSection =
            InsightsParameterSection.DAY
        val DEFAULT_INSIGHTS_DAY_ACTIVITIES_VIEW: InsightsActivityView = InsightsActivityView.RECORDS
        val DEFAULT_INSIGHTS_PERIOD_ACTIVITIES_VIEW: InsightsActivityView = InsightsActivityView.OVERVIEW
        const val DEFAULT_INSIGHTS_TIME_PARAMETERS_EXPANDED: Boolean = true
        val DEFAULT_INSIGHTS_PIE_PALETTE_PRESET: InsightsPiePalettePreset =
            defaultInsightsPiePalettePreset()
        val DEFAULT_INSIGHTS_AVERAGE_DAY_BASIS: InsightsAverageDayBasis =
            InsightsAverageDayBasis.ACTIVE_DAYS
        val DEFAULT_INSIGHTS_COMPARISON_COLOR_SCHEME: InsightsComparisonColorScheme =
            defaultInsightsComparisonColorScheme()
        val DEFAULT_INSIGHTS_COMPARISON_INDICATOR_STYLE: InsightsComparisonIndicatorStyle =
            defaultInsightsComparisonIndicatorStyle()
        const val DEFAULT_INSIGHTS_CHART_TREND_ROOT: String = ""
        private const val MIN_RECORD_FREQUENT_LOOKBACK_DAYS: Int = 0
        private const val MAX_RECORD_FREQUENT_LOOKBACK_DAYS: Int = 60
        private const val MIN_RECORD_FREQUENT_TOP_N: Int = 0
        private const val MAX_RECORD_FREQUENT_TOP_N: Int = 20
        private const val MAX_RECORD_QUICK_ACTIVITY_COUNT: Int = 12
        private const val MAX_RECORD_QUICK_ACTIVITY_LENGTH: Int = 40
        const val DEFAULT_RECORD_QUICK_ACCESS_EDITOR_VISIBLE: Boolean = false
        const val DEFAULT_RECORD_QUICK_ACCESS_CARD_EXPANDED: Boolean = true
        const val DEFAULT_PROMPT_BEFORE_UNCONFIGURED_ACTIVITY_RECORD: Boolean = false
        const val DEFAULT_PAGE_TRANSITIONS_ENABLED: Boolean = true
        val DEFAULT_PAGE_TRANSITION_STYLE: PageTransitionStyle = PageTransitionStyle.FADE
        const val DEFAULT_CONFIG_CARD_EXPANDED: Boolean = true
        const val DEFAULT_CONFIG_SECTION_EXPANDED: Boolean = false
        val DEFAULT_COLLAPSED_CANONICAL_ROOT_PATHS: Set<String> = emptySet()
        val DEFAULT_ORDERED_CANONICAL_ROOT_PATHS: List<String> = emptyList()
        private const val MAX_COLLAPSED_CANONICAL_ROOT_COUNT: Int = 64
        private const val MAX_COLLAPSED_CANONICAL_ROOT_LENGTH: Int = 120
        private const val MAX_ORDERED_CANONICAL_ROOT_COUNT: Int = 64
        private const val MAX_ORDERED_CANONICAL_ROOT_LENGTH: Int = 120
    }

    private object PreferencesKeys {
        val THEME_MODE = stringPreferencesKey("theme_mode")
        val DARK_THEME_STYLE = stringPreferencesKey("dark_theme_style")
        val THEME_PALETTE = stringPreferencesKey("theme_palette")
        val APP_LANGUAGE = stringPreferencesKey("app_language")
        val RECORD_FREQUENT_LOOKBACK_DAYS = intPreferencesKey("record_frequent_lookback_days")
        val RECORD_FREQUENT_TOP_N = intPreferencesKey("record_frequent_top_n")
        val RECORD_FREQUENT_OUTPUT_MODE = stringPreferencesKey("record_frequent_output_mode")
        val RECORD_CANONICAL_CATALOG_DISPLAY_MODE =
            stringPreferencesKey("record_canonical_catalog_display_mode")
        val RECORD_CANONICAL_CATALOG_SOURCE =
            stringPreferencesKey("record_canonical_catalog_source")
        val RECORD_QUICK_ACTIVITIES = stringPreferencesKey("record_quick_activities")
        val RECORD_QUICK_ACCESS_CARD_EXPANDED =
            booleanPreferencesKey("record_quick_access_card_expanded")
        val RECORD_QUICK_ACCESS_EDITOR_VISIBLE =
            booleanPreferencesKey("record_quick_access_editor_visible")
        val PROMPT_BEFORE_UNCONFIGURED_ACTIVITY_RECORD =
            booleanPreferencesKey("prompt_before_unconfigured_activity_record")
        val PAGE_TRANSITIONS_ENABLED = booleanPreferencesKey("page_transitions_enabled")
        val PAGE_TRANSITION_STYLE = stringPreferencesKey("page_transition_style")
        val CONFIG_APPLICATION_PREFERENCES_EXPANDED =
            booleanPreferencesKey("config_application_preferences_expanded")
        val CONFIG_APPEARANCE_EXPANDED = booleanPreferencesKey("config_appearance_expanded")
        val CONFIG_THEME_PALETTE_EXPANDED = booleanPreferencesKey("config_theme_palette_expanded")
        val CONFIG_INSIGHTS_SETTINGS_EXPANDED =
            booleanPreferencesKey("config_insights_settings_expanded")
        val CONFIG_INSIGHTS_CHART_STYLE_EXPANDED =
            booleanPreferencesKey("config_insights_chart_style_expanded")
        val CONFIG_INSIGHTS_COMPARISON_EXPANDED =
            booleanPreferencesKey("config_insights_comparison_expanded")
        val CONFIG_DATA_MANAGEMENT_EXPANDED = booleanPreferencesKey("config_data_management_expanded")
        val CONFIG_ABOUT_EXPANDED = booleanPreferencesKey("config_about_expanded")
        val RECORD_COLLAPSED_CANONICAL_ROOT_PATHS =
            stringPreferencesKey("record_collapsed_canonical_root_paths")
        val RECORD_ORDERED_CANONICAL_ROOT_PATHS =
            stringPreferencesKey("record_ordered_canonical_root_paths")
        val RECORD_LAST_AUTHORING_MODE = stringPreferencesKey("record_last_authoring_mode")
        val RECORD_LAST_TXT_OUTPUT_MODE = stringPreferencesKey("record_last_txt_output_mode")
        val RECORD_DRAFT_PRESENT = booleanPreferencesKey("record_draft_present")
        val RECORD_DRAFT_CONTENT = stringPreferencesKey("record_draft_content")
        val RECORD_DRAFT_REMARK = stringPreferencesKey("record_draft_remark")
        val RECORD_DRAFT_INTERVAL_START = stringPreferencesKey("record_draft_interval_start")
        val RECORD_DRAFT_INTERVAL_END = stringPreferencesKey("record_draft_interval_end")
        val RECORD_DRAFT_INTERVAL_STARTED_AT = longPreferencesKey("record_draft_interval_started_at")
        val RECORD_DRAFT_ATTRIBUTION_DATE = stringPreferencesKey("record_draft_attribution_date")
        val RECORD_DRAFT_LOGICAL_DAY_TARGET = stringPreferencesKey("record_draft_logical_day_target")
        val INSIGHTS_CHART_SHOW_AVERAGE_LINE = booleanPreferencesKey("insights_chart_show_average_line")
        val INSIGHTS_CHART_SEMANTIC_MODE = stringPreferencesKey("insights_chart_semantic_mode")
        val INSIGHTS_CHART_VISUAL_MODE = stringPreferencesKey("insights_chart_visual_mode")
        val INSIGHTS_MODE = stringPreferencesKey("insights_mode")
        val INSIGHTS_RESULT_DISPLAY_MODE_DAY = stringPreferencesKey("insights_result_display_mode_day")
        val INSIGHTS_RESULT_DISPLAY_MODE_WEEK = stringPreferencesKey("insights_result_display_mode_week")
        val INSIGHTS_RESULT_DISPLAY_MODE_MONTH = stringPreferencesKey("insights_result_display_mode_month")
        val INSIGHTS_RESULT_DISPLAY_MODE_YEAR = stringPreferencesKey("insights_result_display_mode_year")
        val INSIGHTS_RESULT_DISPLAY_MODE_RECENT = stringPreferencesKey("insights_result_display_mode_recent")
        val INSIGHTS_RESULT_DISPLAY_MODE_RANGE = stringPreferencesKey("insights_result_display_mode_range")
        val INSIGHTS_PARAMETER_SECTION = stringPreferencesKey("insights_parameter_section")
        val INSIGHTS_DAY_ACTIVITIES_VIEW = stringPreferencesKey("insights_day_activities_view")
        val INSIGHTS_PERIOD_ACTIVITIES_VIEW = stringPreferencesKey("insights_period_activities_view")
        val INSIGHTS_TIME_PARAMETERS_EXPANDED = booleanPreferencesKey("insights_time_parameters_expanded")
        val INSIGHTS_PIE_PALETTE_PRESET = stringPreferencesKey("insights_pie_palette_preset")
        val INSIGHTS_AVERAGE_DAY_BASIS = stringPreferencesKey("insights_average_day_basis")
        val INSIGHTS_COMPARISON_COLOR_SCHEME =
            stringPreferencesKey("insights_comparison_color_scheme")
        val INSIGHTS_COMPARISON_INDICATOR_STYLE =
            stringPreferencesKey("insights_comparison_indicator_style")
        val INSIGHTS_CHART_TREND_ROOT = stringPreferencesKey("insights_chart_trend_root")
        val INSIGHTS_HEATMAP_PALETTE_NAME = stringPreferencesKey("insights_heatmap_palette_name")
        val INSIGHTS_STATUSES_DAY = stringPreferencesKey("insights_statuses_day")
        val INSIGHTS_STATUSES_WEEK = stringPreferencesKey("insights_statuses_week")
        val INSIGHTS_STATUSES_MONTH = stringPreferencesKey("insights_statuses_month")
        val INSIGHTS_STATUSES_YEAR = stringPreferencesKey("insights_statuses_year")
        val INSIGHTS_STATUSES_RECENT = stringPreferencesKey("insights_statuses_recent")
        val INSIGHTS_STATUSES_RANGE = stringPreferencesKey("insights_statuses_range")
    }

    val themeConfig: Flow<ThemeConfig> = dataStore.data.map { preferences ->
        val modeName = preferences[PreferencesKeys.THEME_MODE] ?: ThemeMode.System.name
        val darkThemeStyleName = preferences[PreferencesKeys.DARK_THEME_STYLE] ?: DarkThemeStyle.Tinted.name
        val paletteName = preferences[PreferencesKeys.THEME_PALETTE] ?: ThemePalette.Indigo.name
        
        ThemeConfig(
            themeMode = runCatching { ThemeMode.valueOf(modeName) }.getOrDefault(ThemeMode.System),
            darkThemeStyle = runCatching { DarkThemeStyle.valueOf(darkThemeStyleName) }
                .getOrDefault(DarkThemeStyle.Tinted),
            palette = runCatching { ThemePalette.valueOf(paletteName) }
                .getOrDefault(ThemePalette.Indigo)
        )
    }

    val appLanguage: Flow<AppLanguage> = dataStore.data.map { preferences ->
        val languageName = preferences[PreferencesKeys.APP_LANGUAGE] ?: AppLanguage.System.name
        runCatching { AppLanguage.valueOf(languageName) }.getOrDefault(AppLanguage.System)
    }

    val promptBeforeUnconfiguredActivityRecord: Flow<Boolean> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.PROMPT_BEFORE_UNCONFIGURED_ACTIVITY_RECORD]
            ?: DEFAULT_PROMPT_BEFORE_UNCONFIGURED_ACTIVITY_RECORD
    }

    val recordFrequentPreferences: Flow<RecordFrequentPreferences> = dataStore.data.map { preferences ->
        val storedLookbackDays = preferences[PreferencesKeys.RECORD_FREQUENT_LOOKBACK_DAYS]
            ?: DEFAULT_RECORD_FREQUENT_LOOKBACK_DAYS
        val storedTopN = preferences[PreferencesKeys.RECORD_FREQUENT_TOP_N]
            ?: DEFAULT_RECORD_FREQUENT_TOP_N
        val storedOutputMode = preferences[PreferencesKeys.RECORD_FREQUENT_OUTPUT_MODE]
            ?: DEFAULT_RECORD_FREQUENT_OUTPUT_MODE.name
        val storedCanonicalCatalogDisplayMode =
            preferences[PreferencesKeys.RECORD_CANONICAL_CATALOG_DISPLAY_MODE]
                ?: DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE.name
        val storedCanonicalCatalogSource =
            preferences[PreferencesKeys.RECORD_CANONICAL_CATALOG_SOURCE]
                ?: DEFAULT_RECORD_CANONICAL_CATALOG_SOURCE.name
        val hasStoredQuickActivities = preferences.contains(PreferencesKeys.RECORD_QUICK_ACTIVITIES)
        val quickActivities = parseQuickActivities(
            raw = preferences[PreferencesKeys.RECORD_QUICK_ACTIVITIES],
            hasStoredValue = hasStoredQuickActivities
        )
        val quickAccessEditorVisible = preferences[PreferencesKeys.RECORD_QUICK_ACCESS_EDITOR_VISIBLE]
            ?: DEFAULT_RECORD_QUICK_ACCESS_EDITOR_VISIBLE
        val quickAccessCardExpanded = preferences[PreferencesKeys.RECORD_QUICK_ACCESS_CARD_EXPANDED]
            ?: DEFAULT_RECORD_QUICK_ACCESS_CARD_EXPANDED
        val collapsedCanonicalRootPaths = parseCollapsedCanonicalRootPaths(
            preferences[PreferencesKeys.RECORD_COLLAPSED_CANONICAL_ROOT_PATHS]
        )
        val orderedCanonicalRootPaths = parseOrderedCanonicalRootPaths(
            preferences[PreferencesKeys.RECORD_ORDERED_CANONICAL_ROOT_PATHS]
        )

        RecordFrequentPreferences(
            lookbackDays = normalizeLookbackDays(storedLookbackDays),
            topN = normalizeTopN(storedTopN),
            outputMode = runCatching { RecordFrequentOutputMode.valueOf(storedOutputMode) }
                .getOrDefault(DEFAULT_RECORD_FREQUENT_OUTPUT_MODE),
            canonicalCatalogDisplayMode = runCatching {
                RecordFrequentOutputMode.valueOf(storedCanonicalCatalogDisplayMode)
            }.getOrDefault(DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE),
            canonicalCatalogSource = runCatching {
                CanonicalCatalogSource.valueOf(storedCanonicalCatalogSource)
            }.getOrDefault(DEFAULT_RECORD_CANONICAL_CATALOG_SOURCE),
            quickActivities = quickActivities,
            quickAccessCardExpanded = quickAccessCardExpanded,
            quickAccessEditorVisible = quickAccessEditorVisible,
            collapsedCanonicalRootPaths = collapsedCanonicalRootPaths,
            orderedCanonicalRootPaths = orderedCanonicalRootPaths
        )
    }

    val recordPersistedInput: Flow<PersistedRecordInputSnapshot> = dataStore.data.map { preferences ->
        val modeName = preferences[PreferencesKeys.RECORD_LAST_AUTHORING_MODE]
            ?: RecordAuthoringMode.INTERVAL.name
        val lastAuthoringMode = runCatching { RecordAuthoringMode.valueOf(modeName) }
            .getOrDefault(RecordAuthoringMode.INTERVAL)
        val txtOutputModeName = preferences[PreferencesKeys.RECORD_LAST_TXT_OUTPUT_MODE]
            // DAY is the first-open default. Once the user changes the capsule, the selected
            // value is stored under RECORD_LAST_TXT_OUTPUT_MODE and takes precedence here.
            ?: TxtOutputMode.DAY.name
        val lastTxtOutputMode = runCatching { TxtOutputMode.valueOf(txtOutputModeName) }
            .getOrDefault(TxtOutputMode.DAY)
        val hasDraft = preferences[PreferencesKeys.RECORD_DRAFT_PRESENT] ?: false
        val draftLogicalDayName = preferences[PreferencesKeys.RECORD_DRAFT_LOGICAL_DAY_TARGET]
            ?: RecordLogicalDayTarget.TODAY.name
        val draftLogicalDayTarget = runCatching { RecordLogicalDayTarget.valueOf(draftLogicalDayName) }
            .getOrDefault(RecordLogicalDayTarget.TODAY)

        PersistedRecordInputSnapshot(
            lastAuthoringMode = lastAuthoringMode,
            lastTxtOutputMode = lastTxtOutputMode,
            draft = if (hasDraft) {
                PersistedRecordInputDraft(
                    recordContent = preferences[PreferencesKeys.RECORD_DRAFT_CONTENT].orEmpty(),
                    recordRemark = preferences[PreferencesKeys.RECORD_DRAFT_REMARK].orEmpty(),
                    intervalStart = preferences[PreferencesKeys.RECORD_DRAFT_INTERVAL_START].orEmpty(),
                    intervalEnd = preferences[PreferencesKeys.RECORD_DRAFT_INTERVAL_END].orEmpty(),
                    intervalStartedAtEpochMs = preferences[
                        PreferencesKeys.RECORD_DRAFT_INTERVAL_STARTED_AT
                    ] ?: 0L,
                    attributionDateIso = preferences[
                        PreferencesKeys.RECORD_DRAFT_ATTRIBUTION_DATE
                    ].orEmpty(),
                    logicalDayTarget = draftLogicalDayTarget
                )
            } else {
                null
            }
        )
    }

    val configCardExpansionPreferences: Flow<ConfigCardExpansionPreferences> = dataStore.data.map {
            preferences ->
        ConfigCardExpansionPreferences(
            applicationPreferencesExpanded = preferences[
                PreferencesKeys.CONFIG_APPLICATION_PREFERENCES_EXPANDED
            ] ?: DEFAULT_CONFIG_CARD_EXPANDED,
            appearanceExpanded = preferences[PreferencesKeys.CONFIG_APPEARANCE_EXPANDED]
                ?: DEFAULT_CONFIG_CARD_EXPANDED,
            themePaletteExpanded = preferences[PreferencesKeys.CONFIG_THEME_PALETTE_EXPANDED]
                ?: DEFAULT_CONFIG_SECTION_EXPANDED,
            insightsSettingsExpanded = preferences[PreferencesKeys.CONFIG_INSIGHTS_SETTINGS_EXPANDED]
                ?: DEFAULT_CONFIG_CARD_EXPANDED,
            insightsChartStyleExpanded = preferences[
                PreferencesKeys.CONFIG_INSIGHTS_CHART_STYLE_EXPANDED
            ] ?: DEFAULT_CONFIG_SECTION_EXPANDED,
            insightsComparisonExpanded = preferences[PreferencesKeys.CONFIG_INSIGHTS_COMPARISON_EXPANDED]
                ?: DEFAULT_CONFIG_SECTION_EXPANDED,
            dataManagementExpanded = preferences[PreferencesKeys.CONFIG_DATA_MANAGEMENT_EXPANDED]
                ?: DEFAULT_CONFIG_CARD_EXPANDED,
            aboutExpanded = preferences[PreferencesKeys.CONFIG_ABOUT_EXPANDED]
                ?: DEFAULT_CONFIG_CARD_EXPANDED
        )
    }

    suspend fun setConfigCardExpanded(card: ConfigCard, value: Boolean) {
        dataStore.edit { preferences ->
            preferences[when (card) {
                ConfigCard.APPLICATION_PREFERENCES ->
                    PreferencesKeys.CONFIG_APPLICATION_PREFERENCES_EXPANDED
                ConfigCard.APPEARANCE -> PreferencesKeys.CONFIG_APPEARANCE_EXPANDED
                ConfigCard.THEME_PALETTE -> PreferencesKeys.CONFIG_THEME_PALETTE_EXPANDED
                ConfigCard.INSIGHTS_SETTINGS -> PreferencesKeys.CONFIG_INSIGHTS_SETTINGS_EXPANDED
                ConfigCard.INSIGHTS_CHART_STYLE ->
                    PreferencesKeys.CONFIG_INSIGHTS_CHART_STYLE_EXPANDED
                ConfigCard.INSIGHTS_COMPARISON -> PreferencesKeys.CONFIG_INSIGHTS_COMPARISON_EXPANDED
                ConfigCard.DATA_MANAGEMENT -> PreferencesKeys.CONFIG_DATA_MANAGEMENT_EXPANDED
                ConfigCard.ABOUT -> PreferencesKeys.CONFIG_ABOUT_EXPANDED
            }] = value
        }
    }

    suspend fun setThemeMode(mode: ThemeMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.THEME_MODE] = mode.name
        }
    }

    suspend fun setDarkThemeStyle(style: DarkThemeStyle) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.DARK_THEME_STYLE] = style.name
        }
    }

    val insightsChartShowAverageLine: Flow<Boolean> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.INSIGHTS_CHART_SHOW_AVERAGE_LINE]
            ?: DEFAULT_INSIGHTS_CHART_SHOW_AVERAGE_LINE
    }

    val insightsPiePalettePreset: Flow<InsightsPiePalettePreset> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.INSIGHTS_PIE_PALETTE_PRESET]
            ?: DEFAULT_INSIGHTS_PIE_PALETTE_PRESET.name
        runCatching { InsightsPiePalettePreset.valueOf(rawValue) }
            .getOrDefault(DEFAULT_INSIGHTS_PIE_PALETTE_PRESET)
    }

    val insightsHeatmapPaletteName: Flow<String> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.INSIGHTS_HEATMAP_PALETTE_NAME].orEmpty()
    }

    val insightsStatusConfigs: Flow<InsightsStatusConfigs> = dataStore.data.map { preferences ->
        InsightsStatusConfigs(
            day = preferences.statusConfigFor(InsightsMode.DAY),
            week = preferences.statusConfigFor(InsightsMode.WEEK),
            month = preferences.statusConfigFor(InsightsMode.MONTH),
            year = preferences.statusConfigFor(InsightsMode.YEAR),
            recent = preferences.statusConfigFor(InsightsMode.RECENT),
            range = preferences.statusConfigFor(InsightsMode.RANGE)
        )
    }

    val insightsAverageDayBasis: Flow<InsightsAverageDayBasis> = dataStore.data.map { preferences ->
        runCatching {
            InsightsAverageDayBasis.valueOf(
                preferences[PreferencesKeys.INSIGHTS_AVERAGE_DAY_BASIS]
                    ?: DEFAULT_INSIGHTS_AVERAGE_DAY_BASIS.name
            )
        }.getOrDefault(DEFAULT_INSIGHTS_AVERAGE_DAY_BASIS)
    }

    val pageTransitionsEnabled: Flow<Boolean> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.PAGE_TRANSITIONS_ENABLED]
            ?: DEFAULT_PAGE_TRANSITIONS_ENABLED
    }

    val pageTransitionStyle: Flow<PageTransitionStyle> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.PAGE_TRANSITION_STYLE]
            ?: DEFAULT_PAGE_TRANSITION_STYLE.name
        runCatching { PageTransitionStyle.valueOf(rawValue) }
            .getOrDefault(DEFAULT_PAGE_TRANSITION_STYLE)
    }

    val insightsComparisonColorScheme: Flow<InsightsComparisonColorScheme> = dataStore.data.map {
            preferences ->
        runCatching {
            InsightsComparisonColorScheme.valueOf(
                preferences[PreferencesKeys.INSIGHTS_COMPARISON_COLOR_SCHEME]
                    ?: DEFAULT_INSIGHTS_COMPARISON_COLOR_SCHEME.name
            )
        }.getOrDefault(DEFAULT_INSIGHTS_COMPARISON_COLOR_SCHEME)
    }

    val insightsComparisonIndicatorStyle: Flow<InsightsComparisonIndicatorStyle> = dataStore.data.map {
            preferences ->
        runCatching {
            InsightsComparisonIndicatorStyle.valueOf(
                preferences[PreferencesKeys.INSIGHTS_COMPARISON_INDICATOR_STYLE]
                    ?: DEFAULT_INSIGHTS_COMPARISON_INDICATOR_STYLE.name
            )
        }.getOrDefault(DEFAULT_INSIGHTS_COMPARISON_INDICATOR_STYLE)
    }

    suspend fun setAppLanguage(language: AppLanguage) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.APP_LANGUAGE] = language.name
        }
    }

    suspend fun setPromptBeforeUnconfiguredActivityRecord(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.PROMPT_BEFORE_UNCONFIGURED_ACTIVITY_RECORD] = value
        }
    }

    suspend fun setRecordFrequentLookbackDays(value: Int) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_FREQUENT_LOOKBACK_DAYS] = normalizeLookbackDays(value)
        }
    }

    suspend fun setRecordFrequentTopN(value: Int) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_FREQUENT_TOP_N] = normalizeTopN(value)
        }
    }

    suspend fun setRecordFrequentOutputMode(value: RecordFrequentOutputMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_FREQUENT_OUTPUT_MODE] = value.name
        }
    }

    val insightsChartSemanticMode: Flow<InsightsChartSemanticMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.INSIGHTS_CHART_SEMANTIC_MODE]
            ?: DEFAULT_INSIGHTS_CHART_SEMANTIC_MODE.name
        runCatching { InsightsChartSemanticMode.valueOf(rawValue) }
            .getOrDefault(DEFAULT_INSIGHTS_CHART_SEMANTIC_MODE)
    }

    val insightsResultDisplayMode: Flow<InsightsResultDisplayMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[preferences.insightsResultDisplayModeKey()]
            ?: DEFAULT_INSIGHTS_RESULT_DISPLAY_MODE.name
        runCatching { InsightsResultDisplayMode.valueOf(rawValue) }
            .getOrDefault(DEFAULT_INSIGHTS_RESULT_DISPLAY_MODE)
    }

    val insightsParameterSection: Flow<InsightsParameterSection> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.INSIGHTS_PARAMETER_SECTION]
            ?: DEFAULT_INSIGHTS_PARAMETER_SECTION.name
        runCatching { InsightsParameterSection.valueOf(rawValue) }
            .getOrDefault(DEFAULT_INSIGHTS_PARAMETER_SECTION)
    }

    val insightsDayActivitiesView: Flow<InsightsActivityView> = dataStore.data.map { preferences ->
        preferences.insightsActivitiesView(
            key = PreferencesKeys.INSIGHTS_DAY_ACTIVITIES_VIEW,
            defaultValue = DEFAULT_INSIGHTS_DAY_ACTIVITIES_VIEW
        )
    }

    val insightsPeriodActivitiesView: Flow<InsightsActivityView> = dataStore.data.map { preferences ->
        preferences.insightsActivitiesView(
            key = PreferencesKeys.INSIGHTS_PERIOD_ACTIVITIES_VIEW,
            defaultValue = DEFAULT_INSIGHTS_PERIOD_ACTIVITIES_VIEW
        )
    }

    suspend fun setRecordCanonicalCatalogDisplayMode(value: RecordFrequentOutputMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_CANONICAL_CATALOG_DISPLAY_MODE] = value.name
        }
    }

    suspend fun setRecordQuickActivities(values: List<String>) {
        dataStore.edit { preferences ->
            val normalized = normalizeQuickActivities(values)
            preferences[PreferencesKeys.RECORD_QUICK_ACTIVITIES] =
                normalized.joinToString(separator = "\n")
        }
    }

    suspend fun setRecordQuickAccessEditorVisible(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_QUICK_ACCESS_EDITOR_VISIBLE] = value
        }
    }

    suspend fun setRecordCollapsedCanonicalRootPaths(values: Set<String>) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_COLLAPSED_CANONICAL_ROOT_PATHS] =
                normalizeCollapsedCanonicalRootPaths(values).joinToString(separator = "\n")
        }
    }

    suspend fun setRecordOrderedCanonicalRootPaths(values: List<String>) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_ORDERED_CANONICAL_ROOT_PATHS] =
                normalizeOrderedCanonicalRootPaths(values).joinToString(separator = "\n")
        }
    }

    suspend fun setRecordLastAuthoringMode(value: RecordAuthoringMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_LAST_AUTHORING_MODE] = value.name
        }
    }

    suspend fun setRecordQuickAccessCardExpanded(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_QUICK_ACCESS_CARD_EXPANDED] = value
        }
    }

    suspend fun setThemePalette(palette: ThemePalette) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.THEME_PALETTE] = palette.name
        }
    }

    val insightsTimeParametersExpanded: Flow<Boolean> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.INSIGHTS_TIME_PARAMETERS_EXPANDED]
            ?: DEFAULT_INSIGHTS_TIME_PARAMETERS_EXPANDED
    }

    val insightsChartVisualMode: Flow<InsightsChartVisualMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.INSIGHTS_CHART_VISUAL_MODE]
            ?: DEFAULT_INSIGHTS_CHART_VISUAL_MODE.name
        runCatching { InsightsChartVisualMode.valueOf(rawValue) }
            .getOrDefault(DEFAULT_INSIGHTS_CHART_VISUAL_MODE)
    }

    val insightsMode: Flow<InsightsMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.INSIGHTS_MODE] ?: DEFAULT_INSIGHTS_MODE.name
        runCatching { InsightsMode.valueOf(rawValue) }.getOrDefault(DEFAULT_INSIGHTS_MODE)
    }

    suspend fun setRecordLastTxtOutputMode(value: TxtOutputMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_LAST_TXT_OUTPUT_MODE] = value.name
        }
    }

    suspend fun saveRecordDraft(draft: PersistedRecordInputDraft) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_DRAFT_PRESENT] = true
            preferences[PreferencesKeys.RECORD_DRAFT_CONTENT] = draft.recordContent
            preferences[PreferencesKeys.RECORD_DRAFT_REMARK] = draft.recordRemark
            preferences[PreferencesKeys.RECORD_DRAFT_INTERVAL_START] = draft.intervalStart
            preferences[PreferencesKeys.RECORD_DRAFT_INTERVAL_END] = draft.intervalEnd
            preferences[PreferencesKeys.RECORD_DRAFT_INTERVAL_STARTED_AT] =
                draft.intervalStartedAtEpochMs
            preferences[PreferencesKeys.RECORD_DRAFT_ATTRIBUTION_DATE] = draft.attributionDateIso
            preferences[PreferencesKeys.RECORD_DRAFT_LOGICAL_DAY_TARGET] =
                draft.logicalDayTarget.name
        }
    }

    suspend fun clearRecordDraft() {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_DRAFT_PRESENT] = false
            preferences.remove(PreferencesKeys.RECORD_DRAFT_CONTENT)
            preferences.remove(PreferencesKeys.RECORD_DRAFT_REMARK)
            preferences.remove(PreferencesKeys.RECORD_DRAFT_INTERVAL_START)
            preferences.remove(PreferencesKeys.RECORD_DRAFT_INTERVAL_END)
            preferences.remove(PreferencesKeys.RECORD_DRAFT_INTERVAL_STARTED_AT)
            preferences.remove(PreferencesKeys.RECORD_DRAFT_ATTRIBUTION_DATE)
            preferences.remove(PreferencesKeys.RECORD_DRAFT_LOGICAL_DAY_TARGET)
        }
    }

    suspend fun setInsightsChartShowAverageLine(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_CHART_SHOW_AVERAGE_LINE] = value
        }
    }

    suspend fun setInsightsPiePalettePreset(value: InsightsPiePalettePreset) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_PIE_PALETTE_PRESET] = value.name
        }
    }

    suspend fun setInsightsHeatmapPaletteName(value: String) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_HEATMAP_PALETTE_NAME] = value.trim()
        }
    }

    suspend fun setStatusConfig(mode: InsightsMode, value: DailyStatusConfig) {
        dataStore.edit { preferences ->
            preferences[mode.statusConfigKey()] =
                serializeDailyStatusDefinitions(value.statuses)
        }
    }

    val insightsChartTrendRoot: Flow<String> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.INSIGHTS_CHART_TREND_ROOT]
            ?: DEFAULT_INSIGHTS_CHART_TREND_ROOT
    }

    suspend fun setInsightsAverageDayBasis(value: InsightsAverageDayBasis) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_AVERAGE_DAY_BASIS] = value.name
        }
    }

    suspend fun setPageTransitionsEnabled(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.PAGE_TRANSITIONS_ENABLED] = value
        }
    }

    suspend fun setPageTransitionStyle(value: PageTransitionStyle) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.PAGE_TRANSITION_STYLE] = value.name
        }
    }

    suspend fun setInsightsComparisonColorScheme(value: InsightsComparisonColorScheme) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_COMPARISON_COLOR_SCHEME] = value.name
        }
    }

    suspend fun setInsightsComparisonIndicatorStyle(value: InsightsComparisonIndicatorStyle) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_COMPARISON_INDICATOR_STYLE] = value.name
        }
    }

    private fun normalizeLookbackDays(value: Int): Int {
        return value.coerceIn(MIN_RECORD_FREQUENT_LOOKBACK_DAYS, MAX_RECORD_FREQUENT_LOOKBACK_DAYS)
    }

    private fun normalizeTopN(value: Int): Int {
        return value.coerceIn(MIN_RECORD_FREQUENT_TOP_N, MAX_RECORD_FREQUENT_TOP_N)
    }

    private fun serializeDailyStatusDefinitions(values: List<DailyStatusDefinition>): String {
        return values.joinToString(separator = "\n") { status ->
            listOf(status.id, status.label, status.parent)
                .joinToString(separator = "\t", transform = ::encodeDailyStatusPart)
        }
    }

    private fun deserializeDailyStatusDefinitions(raw: String?): List<DailyStatusDefinition> {
        if (raw.isNullOrBlank()) {
            return emptyList()
        }
        return raw.lineSequence().mapNotNull { line ->
            val parts = line.split('\t')
            if (parts.size != DAILY_STATUS_FIELD_COUNT) {
                return@mapNotNull null
            }
            val (id, label, parent) = parts.map(::decodeDailyStatusPart)
            if (id.isBlank() || label.isBlank() || parent.isBlank()) {
                null
            } else {
                DailyStatusDefinition(id = id, label = label, parent = parent)
            }
        }.toList()
    }

    private fun encodeDailyStatusPart(value: String): String =
        Base64.getUrlEncoder().withoutPadding()
            .encodeToString(value.toByteArray(StandardCharsets.UTF_8))

    private fun decodeDailyStatusPart(value: String): String = runCatching {
        String(Base64.getUrlDecoder().decode(value), StandardCharsets.UTF_8)
    }.getOrDefault("")

    private fun parseQuickActivities(raw: String?, hasStoredValue: Boolean): List<String> {
        // An unconfigured list must stay empty so the UI does not render placeholder activities
        // before the persisted quick-access preference has been loaded.
        if (!hasStoredValue) {
            return DEFAULT_RECORD_QUICK_ACTIVITIES
        }
        if (raw.isNullOrBlank()) {
            return emptyList()
        }
        val parsed = raw
            .split(Regex("""[,\n;，]+"""))
            .map { it.trim() }
            .filter { it.isNotEmpty() }

        return normalizeQuickActivities(parsed)
    }

    private fun normalizeQuickActivities(values: List<String>): List<String> {
        val unique = linkedSetOf<String>()
        for (raw in values) {
            val trimmed = raw.trim()
            if (trimmed.isEmpty()) {
                continue
            }
            val limited = if (trimmed.length > MAX_RECORD_QUICK_ACTIVITY_LENGTH) {
                trimmed.take(MAX_RECORD_QUICK_ACTIVITY_LENGTH)
            } else {
                trimmed
            }
            unique += limited
            if (unique.size >= MAX_RECORD_QUICK_ACTIVITY_COUNT) {
                break
            }
        }
        return unique.toList()
    }

    suspend fun setInsightsChartSemanticMode(value: InsightsChartSemanticMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_CHART_SEMANTIC_MODE] = value.name
        }
    }

    suspend fun setInsightsChartVisualMode(value: InsightsChartVisualMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_CHART_VISUAL_MODE] = value.name
        }
    }

    suspend fun setInsightsChartTrendRoot(value: String) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_CHART_TREND_ROOT] = value.trim()
        }
    }

    suspend fun setInsightsResultDisplayMode(value: InsightsResultDisplayMode) {
        dataStore.edit { preferences ->
            preferences[preferences.insightsResultDisplayModeKey()] = value.name
        }
    }

    private fun Preferences.insightsResultDisplayModeKey() = when (
        runCatching {
            InsightsMode.valueOf(this[PreferencesKeys.INSIGHTS_MODE].orEmpty())
        }.getOrDefault(DEFAULT_INSIGHTS_MODE)
    ) {
        InsightsMode.DAY -> PreferencesKeys.INSIGHTS_RESULT_DISPLAY_MODE_DAY
        InsightsMode.WEEK -> PreferencesKeys.INSIGHTS_RESULT_DISPLAY_MODE_WEEK
        InsightsMode.MONTH -> PreferencesKeys.INSIGHTS_RESULT_DISPLAY_MODE_MONTH
        InsightsMode.YEAR -> PreferencesKeys.INSIGHTS_RESULT_DISPLAY_MODE_YEAR
        InsightsMode.RECENT -> PreferencesKeys.INSIGHTS_RESULT_DISPLAY_MODE_RECENT
        InsightsMode.RANGE -> PreferencesKeys.INSIGHTS_RESULT_DISPLAY_MODE_RANGE
    }

    private fun Preferences.statusConfigFor(mode: InsightsMode): DailyStatusConfig =
        DailyStatusConfig(statuses = deserializeDailyStatusDefinitions(this[mode.statusConfigKey()]))

    private fun InsightsMode.statusConfigKey() = when (this) {
        InsightsMode.DAY -> PreferencesKeys.INSIGHTS_STATUSES_DAY
        InsightsMode.WEEK -> PreferencesKeys.INSIGHTS_STATUSES_WEEK
        InsightsMode.MONTH -> PreferencesKeys.INSIGHTS_STATUSES_MONTH
        InsightsMode.YEAR -> PreferencesKeys.INSIGHTS_STATUSES_YEAR
        InsightsMode.RECENT -> PreferencesKeys.INSIGHTS_STATUSES_RECENT
        InsightsMode.RANGE -> PreferencesKeys.INSIGHTS_STATUSES_RANGE
    }

    suspend fun setInsightsParameterSection(value: InsightsParameterSection) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_PARAMETER_SECTION] = value.name
        }
    }

    suspend fun setRecordCanonicalCatalogSource(value: CanonicalCatalogSource) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_CANONICAL_CATALOG_SOURCE] = value.name
        }
    }

    suspend fun setInsightsDayActivitiesView(value: InsightsActivityView) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_DAY_ACTIVITIES_VIEW] = value.name
        }
    }

    suspend fun setInsightsPeriodActivitiesView(value: InsightsActivityView) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_PERIOD_ACTIVITIES_VIEW] = value.name
        }
    }

    private fun Preferences.insightsActivitiesView(
        key: Preferences.Key<String>,
        defaultValue: InsightsActivityView
    ): InsightsActivityView = runCatching {
        InsightsActivityView.valueOf(this[key] ?: defaultValue.name)
    }.getOrDefault(defaultValue)

    suspend fun setInsightsTimeParametersExpanded(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_TIME_PARAMETERS_EXPANDED] = value
        }
    }

    suspend fun setInsightsMode(value: InsightsMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.INSIGHTS_MODE] = value.name
        }
    }

    private fun parseCollapsedCanonicalRootPaths(raw: String?): Set<String> {
        if (raw.isNullOrBlank()) {
            return DEFAULT_COLLAPSED_CANONICAL_ROOT_PATHS
        }
        return normalizeCollapsedCanonicalRootPaths(raw.lineSequence().toList())
    }

    private fun normalizeCollapsedCanonicalRootPaths(values: Iterable<String>): Set<String> {
        val unique = linkedSetOf<String>()
        for (raw in values) {
            val trimmed = raw.trim()
            if (trimmed.isEmpty()) {
                continue
            }
            val limited = if (trimmed.length > MAX_COLLAPSED_CANONICAL_ROOT_LENGTH) {
                trimmed.take(MAX_COLLAPSED_CANONICAL_ROOT_LENGTH)
            } else {
                trimmed
            }
            unique += limited
            if (unique.size >= MAX_COLLAPSED_CANONICAL_ROOT_COUNT) {
                break
            }
        }
        return unique
    }

    private fun parseOrderedCanonicalRootPaths(raw: String?): List<String> {
        if (raw.isNullOrBlank()) {
            return DEFAULT_ORDERED_CANONICAL_ROOT_PATHS
        }
        return normalizeOrderedCanonicalRootPaths(raw.lineSequence().toList())
    }

    private fun normalizeOrderedCanonicalRootPaths(values: Iterable<String>): List<String> {
        val unique = linkedSetOf<String>()
        for (raw in values) {
            val trimmed = raw.trim()
            if (trimmed.isEmpty()) {
                continue
            }
            val limited = if (trimmed.length > MAX_ORDERED_CANONICAL_ROOT_LENGTH) {
                trimmed.take(MAX_ORDERED_CANONICAL_ROOT_LENGTH)
            } else {
                trimmed
            }
            unique += limited
            if (unique.size >= MAX_ORDERED_CANONICAL_ROOT_COUNT) {
                break
            }
        }
        return unique.toList()
    }
}
