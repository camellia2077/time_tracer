package com.example.tracer

import android.util.Log
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.heightIn
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ChevronRight
import androidx.compose.material3.Icon
import androidx.compose.material3.SegmentedButton
import androidx.compose.material3.SegmentedButtonDefaults
import androidx.compose.material3.SingleChoiceSegmentedButtonRow
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.material3.TextButton
import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.unit.dp
import com.example.tracer.ui.components.TracerSegmentedButtonDefaults
@Composable
internal fun InsightsCompositionMeasureSelector(
    compositionMeasure: InsightsCompositionMeasure,
    onCompositionMeasureChange: (InsightsCompositionMeasure) -> Unit
) {
    val measures = InsightsCompositionMeasure.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        measures.forEachIndexed { index, item ->
            val selected = compositionMeasure == item
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(index, measures.size),
                onClick = { onCompositionMeasureChange(item) },
                selected = selected,
                colors = TracerSegmentedButtonDefaults.colors(),
                label = { Text(stringResource(item.labelRes())) }
            )
        }
    }
}

@Composable
internal fun InsightsCompositionVisualModeSelector(
    compositionVisualMode: InsightsCompositionVisualMode,
    onCompositionVisualModeChange: (InsightsCompositionVisualMode) -> Unit
) {
    val modes = InsightsCompositionVisualMode.entries
    SingleChoiceSegmentedButtonRow(modifier = Modifier.fillMaxWidth()) {
        modes.forEachIndexed { index, item ->
            val selected = compositionVisualMode == item
            SegmentedButton(
                shape = SegmentedButtonDefaults.itemShape(
                    index = index,
                    count = modes.size
                ),
                onClick = { onCompositionVisualModeChange(item) },
                selected = selected,
                colors = TracerSegmentedButtonDefaults.colors(),
                label = {
                    Text(
                        text = stringResource(item.labelRes()),
                        fontWeight = if (selected) {
                            TracerSegmentedButtonDefaults.activeLabelFontWeight
                        } else {
                            TracerSegmentedButtonDefaults.inactiveLabelFontWeight
                        }
                    )
                }
            )
        }
    }
}
