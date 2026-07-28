package com.example.tracer.ui.viewmodel

import com.example.tracer.data.DarkThemeStyle
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.ThemePalette

sealed interface ThemeEvent {
    data class SetMode(val mode: ThemeMode) : ThemeEvent
    data class SetDarkStyle(val style: DarkThemeStyle) : ThemeEvent
    data class SetPalette(val palette: ThemePalette) : ThemeEvent
}
