package com.example.tracer.ui.viewmodel

import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import com.example.tracer.data.AppLanguage
import com.example.tracer.data.ThemeColor
import com.example.tracer.data.ThemeConfig
import com.example.tracer.data.ThemeMode
import com.example.tracer.data.UserPreferencesRepository
import kotlinx.coroutines.flow.SharingStarted
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.stateIn
import kotlinx.coroutines.launch

class ThemeViewModel(private val repository: UserPreferencesRepository) : ViewModel() {
    val themeConfig: StateFlow<ThemeConfig?> = repository.themeConfig
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = null
        )
    val appLanguage: StateFlow<AppLanguage> = repository.appLanguage
        .stateIn(
            scope = viewModelScope,
            started = SharingStarted.WhileSubscribed(5000),
            initialValue = AppLanguage.English
        )

    fun setThemeColor(color: ThemeColor) {
        viewModelScope.launch {
            repository.setThemeColor(color)
        }
    }

    fun setThemeMode(mode: ThemeMode) {
        viewModelScope.launch {
            repository.setThemeMode(mode)
        }
    }

    fun setUseDynamicColor(useDynamic: Boolean) {
        viewModelScope.launch {
            repository.setUseDynamicColor(useDynamic)
        }
    }

    fun setDarkThemeStyle(style: com.example.tracer.data.DarkThemeStyle) {
        viewModelScope.launch {
            repository.setDarkThemeStyle(style)
        }
    }

    fun setAppLanguage(language: AppLanguage) {
        viewModelScope.launch {
            repository.setAppLanguage(language)
        }
    }
}

class ThemeViewModelFactory(private val repository: UserPreferencesRepository) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(ThemeViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return ThemeViewModel(repository) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class")
    }
}
