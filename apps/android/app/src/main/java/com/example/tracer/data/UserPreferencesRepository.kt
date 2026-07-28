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
import com.example.tracer.RecordSuggestionOutputMode
import com.example.tracer.ReportChartSemanticMode
import com.example.tracer.ReportChartVisualMode
import com.example.tracer.ReportParameterSection
import com.example.tracer.ReportPiePalettePreset
import com.example.tracer.ReportResultDisplayMode
import com.example.tracer.ReportMode
import com.example.tracer.defaultReportPiePalettePreset
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

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
    Chinese,
    English,
    Japanese
}

data class ThemeConfig(
    val themeMode: ThemeMode,
    val darkThemeStyle: DarkThemeStyle = DarkThemeStyle.Tinted,
    val palette: ThemePalette = ThemePalette.Indigo
)

data class RecordSuggestionPreferences(
    val lookbackDays: Int,
    val topN: Int,
    val outputMode: RecordSuggestionOutputMode,
    val canonicalCatalogDisplayMode: RecordSuggestionOutputMode,
    val quickActivities: List<String>,
    val quickAccessCardExpanded: Boolean,
    val assistSettingsExpanded: Boolean,
    val collapsedCanonicalRootPaths: Set<String>,
    val orderedCanonicalRootPaths: List<String>
)

class UserPreferencesRepository(private val dataStore: DataStore<Preferences>) {
    companion object {
        const val DEFAULT_RECORD_SUGGEST_LOOKBACK_DAYS: Int = 7
        const val DEFAULT_RECORD_SUGGEST_TOP_N: Int = 5
        val DEFAULT_RECORD_SUGGEST_OUTPUT_MODE: RecordSuggestionOutputMode =
            RecordSuggestionOutputMode.CANONICAL
        val DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE: RecordSuggestionOutputMode =
            RecordSuggestionOutputMode.CANONICAL
        val DEFAULT_RECORD_QUICK_ACTIVITIES: List<String> = emptyList()
        const val DEFAULT_REPORT_CHART_SHOW_AVERAGE_LINE: Boolean = false
        val DEFAULT_REPORT_CHART_SEMANTIC_MODE: ReportChartSemanticMode =
            ReportChartSemanticMode.COMPOSITION
        val DEFAULT_REPORT_CHART_VISUAL_MODE: ReportChartVisualMode =
            ReportChartVisualMode.LINE
        val DEFAULT_REPORT_MODE: ReportMode = ReportMode.DAY
        val DEFAULT_REPORT_RESULT_DISPLAY_MODE: ReportResultDisplayMode =
            ReportResultDisplayMode.TEXT
        val DEFAULT_REPORT_PARAMETER_SECTION: ReportParameterSection =
            ReportParameterSection.DAY
        const val DEFAULT_REPORT_TIME_PARAMETERS_EXPANDED: Boolean = true
        val DEFAULT_REPORT_PIE_PALETTE_PRESET: ReportPiePalettePreset =
            defaultReportPiePalettePreset()
        private const val MIN_RECORD_SUGGEST_LOOKBACK_DAYS: Int = 0
        private const val MAX_RECORD_SUGGEST_LOOKBACK_DAYS: Int = 60
        private const val MIN_RECORD_SUGGEST_TOP_N: Int = 0
        private const val MAX_RECORD_SUGGEST_TOP_N: Int = 20
        private const val MAX_RECORD_QUICK_ACTIVITY_COUNT: Int = 12
        private const val MAX_RECORD_QUICK_ACTIVITY_LENGTH: Int = 40
        const val DEFAULT_RECORD_ASSIST_SETTINGS_EXPANDED: Boolean = false
        const val DEFAULT_RECORD_QUICK_ACCESS_CARD_EXPANDED: Boolean = true
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
        val RECORD_SUGGEST_LOOKBACK_DAYS = intPreferencesKey("record_suggest_lookback_days")
        val RECORD_SUGGEST_TOP_N = intPreferencesKey("record_suggest_top_n")
        val RECORD_SUGGEST_OUTPUT_MODE = stringPreferencesKey("record_suggest_output_mode")
        val RECORD_CANONICAL_CATALOG_DISPLAY_MODE =
            stringPreferencesKey("record_canonical_catalog_display_mode")
        val RECORD_QUICK_ACTIVITIES = stringPreferencesKey("record_quick_activities")
        val RECORD_QUICK_ACCESS_CARD_EXPANDED =
            booleanPreferencesKey("record_quick_access_card_expanded")
        val RECORD_ASSIST_SETTINGS_EXPANDED = booleanPreferencesKey("record_assist_settings_expanded")
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
        val REPORT_CHART_SHOW_AVERAGE_LINE = booleanPreferencesKey("report_chart_show_average_line")
        val REPORT_CHART_SEMANTIC_MODE = stringPreferencesKey("report_chart_semantic_mode")
        val REPORT_CHART_VISUAL_MODE = stringPreferencesKey("report_chart_visual_mode")
        val REPORT_MODE = stringPreferencesKey("report_mode")
        val REPORT_RESULT_DISPLAY_MODE = stringPreferencesKey("report_result_display_mode")
        val REPORT_PARAMETER_SECTION = stringPreferencesKey("report_parameter_section")
        val REPORT_TIME_PARAMETERS_EXPANDED = booleanPreferencesKey("report_time_parameters_expanded")
        val REPORT_PIE_PALETTE_PRESET = stringPreferencesKey("report_pie_palette_preset")
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
        val languageName = preferences[PreferencesKeys.APP_LANGUAGE] ?: AppLanguage.English.name
        runCatching { AppLanguage.valueOf(languageName) }.getOrDefault(AppLanguage.English)
    }

    val recordSuggestionPreferences: Flow<RecordSuggestionPreferences> = dataStore.data.map { preferences ->
        val storedLookbackDays = preferences[PreferencesKeys.RECORD_SUGGEST_LOOKBACK_DAYS]
            ?: DEFAULT_RECORD_SUGGEST_LOOKBACK_DAYS
        val storedTopN = preferences[PreferencesKeys.RECORD_SUGGEST_TOP_N]
            ?: DEFAULT_RECORD_SUGGEST_TOP_N
        val storedOutputMode = preferences[PreferencesKeys.RECORD_SUGGEST_OUTPUT_MODE]
            ?: DEFAULT_RECORD_SUGGEST_OUTPUT_MODE.name
        val storedCanonicalCatalogDisplayMode =
            preferences[PreferencesKeys.RECORD_CANONICAL_CATALOG_DISPLAY_MODE]
                ?: DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE.name
        val hasStoredQuickActivities = preferences.contains(PreferencesKeys.RECORD_QUICK_ACTIVITIES)
        val quickActivities = parseQuickActivities(
            raw = preferences[PreferencesKeys.RECORD_QUICK_ACTIVITIES],
            hasStoredValue = hasStoredQuickActivities
        )
        val assistSettingsExpanded = preferences[PreferencesKeys.RECORD_ASSIST_SETTINGS_EXPANDED]
            ?: DEFAULT_RECORD_ASSIST_SETTINGS_EXPANDED
        val quickAccessCardExpanded = preferences[PreferencesKeys.RECORD_QUICK_ACCESS_CARD_EXPANDED]
            ?: DEFAULT_RECORD_QUICK_ACCESS_CARD_EXPANDED
        val collapsedCanonicalRootPaths = parseCollapsedCanonicalRootPaths(
            preferences[PreferencesKeys.RECORD_COLLAPSED_CANONICAL_ROOT_PATHS]
        )
        val orderedCanonicalRootPaths = parseOrderedCanonicalRootPaths(
            preferences[PreferencesKeys.RECORD_ORDERED_CANONICAL_ROOT_PATHS]
        )

        RecordSuggestionPreferences(
            lookbackDays = normalizeLookbackDays(storedLookbackDays),
            topN = normalizeTopN(storedTopN),
            outputMode = runCatching { RecordSuggestionOutputMode.valueOf(storedOutputMode) }
                .getOrDefault(DEFAULT_RECORD_SUGGEST_OUTPUT_MODE),
            canonicalCatalogDisplayMode = runCatching {
                RecordSuggestionOutputMode.valueOf(storedCanonicalCatalogDisplayMode)
            }.getOrDefault(DEFAULT_RECORD_CANONICAL_CATALOG_DISPLAY_MODE),
            quickActivities = quickActivities,
            quickAccessCardExpanded = quickAccessCardExpanded,
            assistSettingsExpanded = assistSettingsExpanded,
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

    val reportChartShowAverageLine: Flow<Boolean> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.REPORT_CHART_SHOW_AVERAGE_LINE]
            ?: DEFAULT_REPORT_CHART_SHOW_AVERAGE_LINE
    }

    val reportPiePalettePreset: Flow<ReportPiePalettePreset> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.REPORT_PIE_PALETTE_PRESET]
            ?: DEFAULT_REPORT_PIE_PALETTE_PRESET.name
        runCatching { ReportPiePalettePreset.valueOf(rawValue) }
            .getOrDefault(DEFAULT_REPORT_PIE_PALETTE_PRESET)
    }

    suspend fun setAppLanguage(language: AppLanguage) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.APP_LANGUAGE] = language.name
        }
    }

    suspend fun setRecordSuggestLookbackDays(value: Int) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_SUGGEST_LOOKBACK_DAYS] = normalizeLookbackDays(value)
        }
    }

    suspend fun setRecordSuggestTopN(value: Int) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_SUGGEST_TOP_N] = normalizeTopN(value)
        }
    }

    suspend fun setRecordSuggestOutputMode(value: RecordSuggestionOutputMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_SUGGEST_OUTPUT_MODE] = value.name
        }
    }

    val reportChartSemanticMode: Flow<ReportChartSemanticMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.REPORT_CHART_SEMANTIC_MODE]
            ?: DEFAULT_REPORT_CHART_SEMANTIC_MODE.name
        runCatching { ReportChartSemanticMode.valueOf(rawValue) }
            .getOrDefault(DEFAULT_REPORT_CHART_SEMANTIC_MODE)
    }

    val reportResultDisplayMode: Flow<ReportResultDisplayMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.REPORT_RESULT_DISPLAY_MODE]
            ?: DEFAULT_REPORT_RESULT_DISPLAY_MODE.name
        runCatching { ReportResultDisplayMode.valueOf(rawValue) }
            .getOrDefault(DEFAULT_REPORT_RESULT_DISPLAY_MODE)
    }

    val reportParameterSection: Flow<ReportParameterSection> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.REPORT_PARAMETER_SECTION]
            ?: DEFAULT_REPORT_PARAMETER_SECTION.name
        runCatching { ReportParameterSection.valueOf(rawValue) }
            .getOrDefault(DEFAULT_REPORT_PARAMETER_SECTION)
    }

    suspend fun setRecordCanonicalCatalogDisplayMode(value: RecordSuggestionOutputMode) {
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

    suspend fun setRecordAssistSettingsExpanded(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_ASSIST_SETTINGS_EXPANDED] = value
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

    val reportTimeParametersExpanded: Flow<Boolean> = dataStore.data.map { preferences ->
        preferences[PreferencesKeys.REPORT_TIME_PARAMETERS_EXPANDED]
            ?: DEFAULT_REPORT_TIME_PARAMETERS_EXPANDED
    }

    val reportChartVisualMode: Flow<ReportChartVisualMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.REPORT_CHART_VISUAL_MODE]
            ?: DEFAULT_REPORT_CHART_VISUAL_MODE.name
        runCatching { ReportChartVisualMode.valueOf(rawValue) }
            .getOrDefault(DEFAULT_REPORT_CHART_VISUAL_MODE)
    }

    val reportMode: Flow<ReportMode> = dataStore.data.map { preferences ->
        val rawValue = preferences[PreferencesKeys.REPORT_MODE] ?: DEFAULT_REPORT_MODE.name
        runCatching { ReportMode.valueOf(rawValue) }.getOrDefault(DEFAULT_REPORT_MODE)
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

    suspend fun setReportChartShowAverageLine(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_CHART_SHOW_AVERAGE_LINE] = value
        }
    }

    suspend fun setReportPiePalettePreset(value: ReportPiePalettePreset) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_PIE_PALETTE_PRESET] = value.name
        }
    }

    private fun normalizeLookbackDays(value: Int): Int {
        return value.coerceIn(MIN_RECORD_SUGGEST_LOOKBACK_DAYS, MAX_RECORD_SUGGEST_LOOKBACK_DAYS)
    }

    private fun normalizeTopN(value: Int): Int {
        return value.coerceIn(MIN_RECORD_SUGGEST_TOP_N, MAX_RECORD_SUGGEST_TOP_N)
    }

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

    suspend fun setReportChartSemanticMode(value: ReportChartSemanticMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_CHART_SEMANTIC_MODE] = value.name
        }
    }

    suspend fun setReportChartVisualMode(value: ReportChartVisualMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_CHART_VISUAL_MODE] = value.name
        }
    }

    suspend fun setReportResultDisplayMode(value: ReportResultDisplayMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_RESULT_DISPLAY_MODE] = value.name
        }
    }

    suspend fun setReportParameterSection(value: ReportParameterSection) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_PARAMETER_SECTION] = value.name
        }
    }

    suspend fun setReportTimeParametersExpanded(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_TIME_PARAMETERS_EXPANDED] = value
        }
    }

    suspend fun setReportMode(value: ReportMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_MODE] = value.name
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
