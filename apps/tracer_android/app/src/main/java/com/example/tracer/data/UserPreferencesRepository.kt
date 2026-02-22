package com.example.tracer.data

import android.content.Context
import androidx.datastore.core.DataStore
import androidx.datastore.preferences.core.Preferences
import androidx.datastore.preferences.core.booleanPreferencesKey
import androidx.datastore.preferences.core.edit
import androidx.datastore.preferences.core.intPreferencesKey
import androidx.datastore.preferences.core.stringPreferencesKey
import androidx.datastore.preferences.preferencesDataStore
import kotlinx.coroutines.flow.Flow
import kotlinx.coroutines.flow.map

val Context.dataStore: DataStore<Preferences> by preferencesDataStore(name = "settings")

enum class ThemeColor {
    Rose,
    Orange,
    Peach,
    Amber,
    Gold,
    Mint,
    Emerald,
    Teal,
    Turquoise,
    Cyan,
    Sky,
    Periwinkle,
    Lavender,
    Violet,
    Pink,
    Sakura,
    Magenta,
    Cobalt,
    Navy,
    Crimson,
    Burgundy,
    Lime,
    Cocoa,
    Graphite,
    Slate
}

enum class ThemeMode {
    System, Light, Dark
}

enum class DarkThemeStyle {
    Tinted, Neutral, Black
}

enum class AppLanguage {
    Chinese,
    English,
    Japanese
}

data class ThemeConfig(
    val themeColor: ThemeColor,
    val themeMode: ThemeMode,
    val useDynamicColor: Boolean,
    val darkThemeStyle: DarkThemeStyle = DarkThemeStyle.Tinted
)

data class RecordSuggestionPreferences(
    val lookbackDays: Int,
    val topN: Int,
    val quickActivities: List<String>,
    val assistExpanded: Boolean,
    val assistSettingsExpanded: Boolean
)

class UserPreferencesRepository(private val dataStore: DataStore<Preferences>) {
    companion object {
        const val DEFAULT_RECORD_SUGGEST_LOOKBACK_DAYS: Int = 7
        const val DEFAULT_RECORD_SUGGEST_TOP_N: Int = 5
        val DEFAULT_RECORD_QUICK_ACTIVITIES: List<String> = listOf("meal", "洗漱", "上厕所")
        const val DEFAULT_REPORT_CHART_SHOW_AVERAGE_LINE: Boolean = false
        private const val MIN_RECORD_SUGGEST_LOOKBACK_DAYS: Int = 1
        private const val MAX_RECORD_SUGGEST_LOOKBACK_DAYS: Int = 60
        private const val MIN_RECORD_SUGGEST_TOP_N: Int = 1
        private const val MAX_RECORD_SUGGEST_TOP_N: Int = 20
        private const val MAX_RECORD_QUICK_ACTIVITY_COUNT: Int = 12
        private const val MAX_RECORD_QUICK_ACTIVITY_LENGTH: Int = 40
        const val DEFAULT_RECORD_ASSIST_EXPANDED: Boolean = false
        const val DEFAULT_RECORD_ASSIST_SETTINGS_EXPANDED: Boolean = false
    }

    private object PreferencesKeys {
        val THEME_COLOR = stringPreferencesKey("theme_color")
        val THEME_MODE = stringPreferencesKey("theme_mode")
        val USE_DYNAMIC_COLOR = booleanPreferencesKey("use_dynamic_color")
        val DARK_THEME_STYLE = stringPreferencesKey("dark_theme_style")
        val APP_LANGUAGE = stringPreferencesKey("app_language")
        val RECORD_SUGGEST_LOOKBACK_DAYS = intPreferencesKey("record_suggest_lookback_days")
        val RECORD_SUGGEST_TOP_N = intPreferencesKey("record_suggest_top_n")
        val RECORD_QUICK_ACTIVITIES = stringPreferencesKey("record_quick_activities")
        val RECORD_ASSIST_EXPANDED = booleanPreferencesKey("record_assist_expanded")
        val RECORD_ASSIST_SETTINGS_EXPANDED = booleanPreferencesKey("record_assist_settings_expanded")
        val REPORT_CHART_SHOW_AVERAGE_LINE = booleanPreferencesKey("report_chart_show_average_line")
    }

    val themeConfig: Flow<ThemeConfig> = dataStore.data.map { preferences ->
        val colorName = preferences[PreferencesKeys.THEME_COLOR] ?: ThemeColor.Slate.name
        val modeName = preferences[PreferencesKeys.THEME_MODE] ?: ThemeMode.System.name
        val useDynamicColor = preferences[PreferencesKeys.USE_DYNAMIC_COLOR] ?: false
        val darkThemeStyleName = preferences[PreferencesKeys.DARK_THEME_STYLE] ?: DarkThemeStyle.Tinted.name
        
        ThemeConfig(
            themeColor = runCatching { ThemeColor.valueOf(colorName) }.getOrDefault(ThemeColor.Slate),
            themeMode = runCatching { ThemeMode.valueOf(modeName) }.getOrDefault(ThemeMode.System),
            useDynamicColor = useDynamicColor,
            darkThemeStyle = runCatching { DarkThemeStyle.valueOf(darkThemeStyleName) }
                .getOrDefault(DarkThemeStyle.Tinted)
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
        val quickActivities = parseQuickActivities(
            preferences[PreferencesKeys.RECORD_QUICK_ACTIVITIES]
        )
        val assistExpanded = preferences[PreferencesKeys.RECORD_ASSIST_EXPANDED]
            ?: DEFAULT_RECORD_ASSIST_EXPANDED
        val assistSettingsExpanded = preferences[PreferencesKeys.RECORD_ASSIST_SETTINGS_EXPANDED]
            ?: DEFAULT_RECORD_ASSIST_SETTINGS_EXPANDED

        RecordSuggestionPreferences(
            lookbackDays = normalizeLookbackDays(storedLookbackDays),
            topN = normalizeTopN(storedTopN),
            quickActivities = quickActivities,
            assistExpanded = assistExpanded,
            assistSettingsExpanded = assistSettingsExpanded
        )
    }

    suspend fun setThemeColor(color: ThemeColor) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.THEME_COLOR] = color.name
        }
    }

    suspend fun setThemeMode(mode: ThemeMode) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.THEME_MODE] = mode.name
        }
    }

    suspend fun setUseDynamicColor(useDynamic: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.USE_DYNAMIC_COLOR] = useDynamic
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

    suspend fun setRecordQuickActivities(values: List<String>) {
        dataStore.edit { preferences ->
            val normalized = normalizeQuickActivities(values)
            preferences[PreferencesKeys.RECORD_QUICK_ACTIVITIES] =
                normalized.joinToString(separator = "\n")
        }
    }

    suspend fun setRecordAssistExpanded(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_ASSIST_EXPANDED] = value
        }
    }

    suspend fun setRecordAssistSettingsExpanded(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.RECORD_ASSIST_SETTINGS_EXPANDED] = value
        }
    }

    suspend fun setReportChartShowAverageLine(value: Boolean) {
        dataStore.edit { preferences ->
            preferences[PreferencesKeys.REPORT_CHART_SHOW_AVERAGE_LINE] = value
        }
    }

    private fun normalizeLookbackDays(value: Int): Int {
        return value.coerceIn(MIN_RECORD_SUGGEST_LOOKBACK_DAYS, MAX_RECORD_SUGGEST_LOOKBACK_DAYS)
    }

    private fun normalizeTopN(value: Int): Int {
        return value.coerceIn(MIN_RECORD_SUGGEST_TOP_N, MAX_RECORD_SUGGEST_TOP_N)
    }

    private fun parseQuickActivities(raw: String?): List<String> {
        if (raw.isNullOrBlank()) {
            return DEFAULT_RECORD_QUICK_ACTIVITIES
        }
        val parsed = raw
            .split(Regex("""[,\n;，]+"""))
            .map { it.trim() }
            .filter { it.isNotEmpty() }

        val normalized = normalizeQuickActivities(parsed)
        if (normalized.isEmpty()) {
            return DEFAULT_RECORD_QUICK_ACTIVITIES
        }
        return normalized
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
        if (unique.isEmpty()) {
            return DEFAULT_RECORD_QUICK_ACTIVITIES
        }
        return unique.toList()
    }
}
