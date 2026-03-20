package com.example.tracer

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.mutableStateListOf
import androidx.compose.runtime.remember
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp

@Composable
internal fun ReportMarkdownText(
    markdown: String,
    modifier: Modifier = Modifier,
    style: TextStyle = MaterialTheme.typography.bodyMedium,
    color: Color = MaterialTheme.colorScheme.onSurface
) {
    val sections = remember(markdown) { parseReportMarkdownSections(markdown) }
    val expandedStates = remember(markdown) {
        mutableStateListOf<Boolean>().apply {
            repeat(sections.size) {
                add(true)
            }
        }
    }

    SelectionContainer(modifier = modifier) {
        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            val parentExpansionStack = mutableListOf<Pair<Int, Boolean>>()
            sections.forEachIndexed { index, section ->
                val header = section.header
                if (header != null) {
                    while (parentExpansionStack.isNotEmpty() &&
                        parentExpansionStack.last().first >= header.level
                    ) {
                        parentExpansionStack.removeAt(parentExpansionStack.lastIndex)
                    }
                    val parentExpanded = parentExpansionStack.all { it.second }
                    val isExpanded = expandedStates.getOrNull(index) ?: true

                    if (parentExpanded) {
                        ReportMarkdownHeader(
                            header = header,
                            expanded = isExpanded,
                            onToggle = {
                                expandedStates[index] = !expandedStates[index]
                            }
                        )
                        if (isExpanded) {
                            section.content.forEach { block ->
                                ReportMarkdownBlock(
                                    block = block,
                                    style = style,
                                    color = color
                                )
                            }
                        }
                    }
                    parentExpansionStack.add(header.level to isExpanded)
                } else if (parentExpansionStack.all { it.second }) {
                    section.content.forEach { block ->
                        ReportMarkdownBlock(
                            block = block,
                            style = style,
                            color = color
                        )
                    }
                }
            }
        }
    }
}

@Composable
private fun ReportMarkdownHeader(
    header: MarkdownBlock.Header,
    expanded: Boolean,
    onToggle: () -> Unit
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(top = 8.dp)
            .clickable(onClick = onToggle),
        verticalAlignment = Alignment.CenterVertically
    ) {
        Text(
            text = if (expanded) "▼" else "▶",
            style = MaterialTheme.typography.bodySmall,
            color = MaterialTheme.colorScheme.primary,
            modifier = Modifier.padding(end = 6.dp)
        )
        Text(
            text = header.text,
            style = when (header.level) {
                1 -> MaterialTheme.typography.headlineMedium
                2 -> MaterialTheme.typography.titleLarge
                else -> MaterialTheme.typography.titleMedium
            },
            color = MaterialTheme.colorScheme.primary,
            fontWeight = FontWeight.Bold
        )
    }
}

@Composable
private fun ReportMarkdownBlock(
    block: MarkdownBlock,
    style: TextStyle,
    color: Color
) {
    when (block) {
        is MarkdownBlock.Header -> {
            Text(
                text = block.text,
                style = when (block.level) {
                    1 -> MaterialTheme.typography.headlineMedium
                    2 -> MaterialTheme.typography.titleLarge
                    else -> MaterialTheme.typography.titleMedium
                },
                color = MaterialTheme.colorScheme.primary,
                fontWeight = FontWeight.Bold,
                modifier = Modifier.padding(top = 8.dp)
            )
        }

        is MarkdownBlock.Paragraph -> {
            Text(
                text = buildAnnotatedString {
                    parseInlineMarkdown(block.text, this)
                },
                style = style,
                color = color
            )
        }

        is MarkdownBlock.CodeBlock -> {
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(
                        MaterialTheme.colorScheme.surfaceVariant,
                        shape = MaterialTheme.shapes.small
                    )
                    .padding(12.dp)
            ) {
                Text(
                    text = block.content,
                    style = MaterialTheme.typography.bodyMedium.copy(
                        fontFamily = FontFamily.Monospace,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                )
            }
        }

        is MarkdownBlock.ListBlock -> {
            Column(verticalArrangement = Arrangement.spacedBy(4.dp)) {
                block.items.forEach { item ->
                    Row(
                        modifier = Modifier.padding(start = (item.level * 16).dp)
                    ) {
                        Text(
                            text = "• ",
                            style = style,
                            color = color,
                            fontWeight = FontWeight.Bold
                        )
                        Text(
                            text = buildAnnotatedString {
                                parseInlineMarkdown(item.text, this)
                            },
                            style = style,
                            color = color
                        )
                    }
                }
            }
        }
    }
}
