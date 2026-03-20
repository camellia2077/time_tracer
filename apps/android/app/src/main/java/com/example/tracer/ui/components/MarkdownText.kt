package com.example.tracer.ui.components

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.text.selection.SelectionContainer
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp

@Composable
fun MarkdownText(
    markdown: String,
    modifier: Modifier = Modifier,
    style: TextStyle = MaterialTheme.typography.bodyMedium,
    color: Color = MaterialTheme.colorScheme.onSurface
) {
    val blocks = remember(markdown) { parseMarkdown(markdown) }

    SelectionContainer(modifier = modifier) {
        Column(verticalArrangement = Arrangement.spacedBy(8.dp)) {
            blocks.forEach { block ->
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
                                parseInline(block.text, this)
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
                                Row {
                                    Text(
                                        text = "• ",
                                        style = style,
                                        color = color,
                                        fontWeight = FontWeight.Bold
                                    )
                                    Text(
                                        text = buildAnnotatedString {
                                            parseInline(item, this)
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
        }
    }
}

private sealed class MarkdownBlock {
    data class Header(val text: String, val level: Int) : MarkdownBlock()
    data class Paragraph(val text: String) : MarkdownBlock()
    data class CodeBlock(val content: String) : MarkdownBlock()
    data class ListBlock(val items: List<String>) : MarkdownBlock()
}

private fun parseMarkdown(text: String): List<MarkdownBlock> {
    val lines = text.lines()
    val blocks = mutableListOf<MarkdownBlock>()
    var inCodeBlock = false
    val codeBlockBuffer = StringBuilder()
    val listBuffer = mutableListOf<String>()

    fun flushList() {
        if (listBuffer.isNotEmpty()) {
            blocks.add(MarkdownBlock.ListBlock(listBuffer.toList()))
            listBuffer.clear()
        }
    }

    for (line in lines) {
        if (line.trim().startsWith("```")) {
            flushList()
            if (inCodeBlock) {
                blocks.add(MarkdownBlock.CodeBlock(codeBlockBuffer.toString().trimEnd()))
                codeBlockBuffer.clear()
                inCodeBlock = false
            } else {
                inCodeBlock = true
            }
            continue
        }

        if (inCodeBlock) {
            codeBlockBuffer.append(line).append("\n")
            continue
        }

        val trimmed = line.trim()
        if (trimmed.startsWith("#")) {
            flushList()
            val level = trimmed.takeWhile { it == '#' }.length
            blocks.add(MarkdownBlock.Header(trimmed.drop(level).trim(), level))
        } else if (trimmed.startsWith("- ") || trimmed.startsWith("* ")) {
            listBuffer.add(trimmed.drop(2))
        } else if (trimmed.isBlank()) {
            flushList()
        } else {
            flushList()
            blocks.add(MarkdownBlock.Paragraph(trimmed))
        }
    }
    flushList()
    return blocks
}

private fun parseInline(text: String, builder: androidx.compose.ui.text.AnnotatedString.Builder) {
    // Very simple bold and code parsing
    // Supports **bold** and `code`

    // We can't easily nest regexes in a simple loop without a proper tokenizer.
    // For this MVP, we'll just handle Bold, then plain text.
    // A better approach for the future: full AST parser.

    // Simple split for now:
    val parts = text.split("**")
    parts.forEachIndexed { index, part ->
        if (index % 2 == 1) {
            // Bold part
            builder.withStyle(SpanStyle(fontWeight = FontWeight.Bold)) {
                append(part)
            }
        } else {
            // Normal part (check for code)
            val subParts = part.split("`")
            subParts.forEachIndexed { subIndex, subPart ->
                if (subIndex % 2 == 1) {
                    builder.withStyle(
                        SpanStyle(
                            fontFamily = FontFamily.Monospace,
                            background = Color.LightGray.copy(alpha = 0.3f)
                        )
                    ) {
                        append(subPart)
                    }
                } else {
                    builder.append(subPart)
                }
            }
        }
    }
}
