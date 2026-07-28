package com.example.tracer

import androidx.compose.ui.text.font.FontWeight
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults

internal fun segmentedButtonLabelWeight(selected: Boolean): FontWeight =
    if (selected) {
        TracerSegmentedButtonDefaults.activeLabelFontWeight
    } else {
        TracerSegmentedButtonDefaults.inactiveLabelFontWeight
    }
