package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.FlowRow
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.Check
import androidx.compose.material.icons.filled.Close
import androidx.compose.material.icons.filled.Delete
import androidx.compose.material3.FilterChip
import androidx.compose.material3.FilledTonalButton
import androidx.compose.material3.Icon
import androidx.compose.material3.IconButton
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.res.stringResource
import androidx.compose.ui.semantics.contentDescription
import androidx.compose.ui.semantics.semantics
import androidx.compose.ui.unit.dp
import androidx.compose.ui.window.Dialog
import androidx.compose.ui.window.DialogProperties

@Composable
internal fun ActivityHierarchyParentColorEditor(
    draftValue: String,
    persistedValue: String,
    onDraftValueChange: (String) -> Unit,
    onSaveColor: (String) -> Unit
) {
    val normalizedDraft = normalizeParentColorInput(draftValue)
    val storedValue = parentColorForStorage(normalizedDraft)
    val isSaved = storedValue == parentColorForStorage(persistedValue)
    val preview = previewParentColor(normalizedDraft)
    var showPalette by remember { mutableStateOf(false) }
    Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(8.dp)
        ) {
            OutlinedTextField(
                value = normalizedDraft,
                onValueChange = { onDraftValueChange(normalizeParentColorInput(it)) },
                modifier = Modifier.weight(1f),
                label = { Text(stringResource(R.string.config_alias_parent_color_label)) },
                supportingText = { Text(stringResource(R.string.config_alias_parent_color_hint)) },
                singleLine = true,
                prefix = { Text("#") }
            )
            preview?.let { color ->
                Box(
                    modifier = Modifier
                        .size(32.dp)
                        .border(1.dp, MaterialTheme.colorScheme.outlineVariant)
                        .background(color)
                )
            }
            IconButton(
                onClick = { onDraftValueChange("") },
                enabled = normalizedDraft.isNotEmpty()
            ) {
                Icon(
                    imageVector = Icons.Default.Delete,
                    contentDescription = stringResource(R.string.config_alias_action_clear_parent_color)
                )
            }
        }
        OutlinedButton(onClick = { showPalette = true }, modifier = Modifier.fillMaxWidth()) {
            Text(stringResource(R.string.config_alias_action_choose_parent_color))
        }
        FilledTonalButton(
            onClick = { onSaveColor(storedValue) },
            enabled = !isSaved,
            modifier = Modifier.fillMaxWidth()
        ) {
            if (isSaved) {
                Icon(Icons.Default.Check, contentDescription = null)
                Spacer(modifier = Modifier.width(8.dp))
                Text(stringResource(R.string.config_alias_parent_color_saved))
            } else {
                Text(stringResource(R.string.config_alias_action_save_parent_color))
            }
        }
    }
    if (showPalette) {
        ParentColorPaletteScreen(
            currentColor = storedValue,
            onDismiss = { showPalette = false },
            onColorSelected = {
                onDraftValueChange(it)
                showPalette = false
            }
        )
    }
}

@Composable
private fun ParentColorPaletteScreen(
    currentColor: String,
    onDismiss: () -> Unit,
    onColorSelected: (String) -> Unit
) {
    var selectedFamily by remember(currentColor) {
        mutableStateOf(ParentColorFamilies.firstOrNull { family ->
            family.colors.any { it.hex.equals(currentColor, ignoreCase = true) }
        } ?: ParentColorFamilies.first())
    }
    Dialog(
        onDismissRequest = onDismiss,
        properties = DialogProperties(usePlatformDefaultWidth = false)
    ) {
        Surface(modifier = Modifier.fillMaxSize(), color = MaterialTheme.colorScheme.surface) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(horizontal = 24.dp, vertical = 16.dp),
                verticalArrangement = Arrangement.spacedBy(16.dp)
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        text = stringResource(R.string.config_alias_parent_color_palette_title),
                        style = MaterialTheme.typography.titleLarge
                    )
                    IconButton(onClick = onDismiss) {
                        Icon(
                            imageVector = Icons.Default.Close,
                            contentDescription = stringResource(
                                R.string.config_alias_action_close_parent_color_picker
                            )
                        )
                    }
                }
                Text(
                    text = stringResource(R.string.config_alias_parent_color_family_label),
                    style = MaterialTheme.typography.labelLarge
                )
                FlowRow(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    ParentColorFamilies.forEach { family ->
                        val categoryColor = family.categoryColor.hex.toComposeColorOrNull()
                        FilterChip(
                            onClick = { selectedFamily = family },
                            selected = selectedFamily == family,
                            label = { Text(stringResource(family.labelRes)) },
                            leadingIcon = categoryColor?.let { color ->
                                {
                                    Box(
                                        modifier = Modifier
                                            .size(16.dp)
                                            .border(
                                                1.dp,
                                                MaterialTheme.colorScheme.outlineVariant,
                                                CircleShape
                                            )
                                            .background(color, CircleShape)
                                    )
                                }
                            }
                        )
                    }
                }
                Text(
                    text = stringResource(
                        R.string.config_alias_parent_color_palette_label,
                        stringResource(selectedFamily.labelRes)
                    ),
                    style = MaterialTheme.typography.labelLarge
                )
                FlowRow(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(10.dp),
                    verticalArrangement = Arrangement.spacedBy(10.dp)
                ) {
                    selectedFamily.colors.forEach { cssColor ->
                        val color = cssColor.hex.toComposeColorOrNull() ?: return@forEach
                        val selected = currentColor.equals(cssColor.hex, ignoreCase = true)
                        Column(
                            modifier = Modifier
                                .border(
                                    width = if (selected) 2.dp else 1.dp,
                                    color = if (selected) MaterialTheme.colorScheme.primary
                                    else MaterialTheme.colorScheme.outlineVariant,
                                    shape = MaterialTheme.shapes.medium
                                )
                                .clickable { onColorSelected(cssColor.hex) }
                                .padding(12.dp)
                                .semantics { contentDescription = "${cssColor.name} ${cssColor.hex}" }
                        ) {
                            Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                                Box(
                                    modifier = Modifier
                                        .size(40.dp)
                                        .border(
                                            1.dp,
                                            MaterialTheme.colorScheme.outlineVariant,
                                            CircleShape
                                        )
                                        .background(color, CircleShape)
                                )
                                Column {
                                    Text(cssColor.name, style = MaterialTheme.typography.titleSmall)
                                    Text(
                                        cssColor.hex,
                                        style = MaterialTheme.typography.bodySmall,
                                        color = MaterialTheme.colorScheme.onSurfaceVariant
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

private data class CssNamedColor(val name: String, val hex: String)

private data class ParentColorFamily(
    val labelRes: Int,
    val categoryColor: CssNamedColor,
    val colors: List<CssNamedColor>
)

private val ParentColorFamilies = listOf(
    family(R.string.config_alias_parent_color_family_red, css("Red", "#FF0000"), css("LightCoral", "#F08080"), css("IndianRed", "#CD5C5C"), css("Crimson", "#DC143C"), css("FireBrick", "#B22222")),
    family(R.string.config_alias_parent_color_family_orange, css("Orange", "#FFA500"), css("PeachPuff", "#FFDAB9"), css("Coral", "#FF7F50"), css("DarkOrange", "#FF8C00"), css("Chocolate", "#D2691E")),
    family(R.string.config_alias_parent_color_family_yellow, css("Yellow", "#FFFF00"), css("Khaki", "#F0E68C"), css("Gold", "#FFD700"), css("Goldenrod", "#DAA520"), css("DarkGoldenrod", "#B8860B")),
    family(R.string.config_alias_parent_color_family_green, css("Green", "#008000"), css("PaleGreen", "#98FB98"), css("LimeGreen", "#32CD32"), css("ForestGreen", "#228B22"), css("DarkGreen", "#006400"), css("GreenYellow", "#ADFF2F"), css("YellowGreen", "#9ACD32"), css("OliveDrab", "#6B8E23"), css("Olive", "#808000")),
    family(R.string.config_alias_parent_color_family_teal, css("Teal", "#008080"), css("PaleTurquoise", "#AFEEEE"), css("MediumTurquoise", "#48D1CC"), css("DarkSlateGray", "#2F4F4F")),
    family(R.string.config_alias_parent_color_family_blue, css("Blue", "#0000FF"), css("LightSkyBlue", "#87CEFA"), css("DodgerBlue", "#1E90FF"), css("RoyalBlue", "#4169E1"), css("Navy", "#000080"), css("Aqua", "#00FFFF"), css("LightCyan", "#E0FFFF"), css("DeepSkyBlue", "#00BFFF"), css("DarkCyan", "#008B8B"), css("CadetBlue", "#5F9EA0")),
    family(R.string.config_alias_parent_color_family_purple, css("Purple", "#800080"), css("Plum", "#DDA0DD"), css("MediumOrchid", "#BA55D3"), css("BlueViolet", "#8A2BE2"), css("RebeccaPurple", "#663399"), css("Indigo", "#4B0082"), css("MediumSlateBlue", "#7B68EE"), css("SlateBlue", "#6A5ACD"), css("MidnightBlue", "#191970")),
    family(R.string.config_alias_parent_color_family_pink, css("Pink", "#FFC0CB"), css("LightPink", "#FFB6C1"), css("HotPink", "#FF69B4"), css("DeepPink", "#FF1493"), css("PaleVioletRed", "#DB7093")),
    family(R.string.config_alias_parent_color_family_brown, css("Brown", "#A52A2A"), css("Wheat", "#F5DEB3"), css("Peru", "#CD853F"), css("SaddleBrown", "#8B4513"), css("Maroon", "#800000")),
    family(R.string.config_alias_parent_color_family_gray, css("Gray", "#808080"), css("Gainsboro", "#DCDCDC"), css("DarkGray", "#A9A9A9"), css("DimGray", "#696969"), css("Black", "#000000"))
)

private fun css(name: String, hex: String) = CssNamedColor(name, hex)

private fun family(labelRes: Int, categoryColor: CssNamedColor, vararg colors: CssNamedColor) =
    ParentColorFamily(labelRes, categoryColor, listOf(categoryColor, *colors))

internal fun normalizeParentColorInput(value: String): String =
    value.filter { it.isDigit() || it.lowercaseChar() in 'a'..'f' }
        .take(6)
        .uppercase()

internal fun parentColorForStorage(value: String): String =
    normalizeParentColorInput(value)
        .takeIf { it.isNotEmpty() }
        ?.padEnd(6, '0')
        ?.let { "#$it" }
        .orEmpty()

internal fun previewParentColor(value: String): Color? =
    parentColorForStorage(value).toComposeColorOrNull()

private fun isHexParentColor(value: String): Boolean =
    value.length == 7 && value.firstOrNull() == '#' &&
        value.substring(1).all { it.isDigit() || it.lowercaseChar() in 'a'..'f' }

private fun String.toComposeColorOrNull(): Color? =
    takeIf(::isHexParentColor)?.substring(1)?.toLongOrNull(16)?.let { Color(0xFF000000L or it) }
