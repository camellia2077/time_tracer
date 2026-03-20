package com.example.tracer

import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.withStyle

internal sealed class MarkdownBlock {
    data class Header(val text: String, val level: Int) : MarkdownBlock()
    data class Paragraph(val text: String) : MarkdownBlock()
    data class CodeBlock(val content: String) : MarkdownBlock()
    data class ListBlock(val items: List<MarkdownListItem>) : MarkdownBlock()
}

internal data class MarkdownListItem(
    val text: String,
    val level: Int
)

internal data class MarkdownSection(
    val header: MarkdownBlock.Header?,
    val content: List<MarkdownBlock>
)

internal fun parseReportMarkdown(text: String): List<MarkdownBlock> {
    val lines = text.lines()
    val blocks = mutableListOf<MarkdownBlock>()
    var inCodeBlock = false
    val codeBlockBuffer = StringBuilder()
    val listBuffer = mutableListOf<MarkdownListItem>()

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
            val rawHeader = trimmed.drop(level).trim()
            val headerText = rawHeader.replace(Regex("\\s+#+\\s*$"), "").trim()
            blocks.add(MarkdownBlock.Header(headerText, level))
        } else if (isMarkdownListLine(line)) {
            parseMarkdownListItem(line)?.let { listBuffer.add(it) }
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

private fun isMarkdownListLine(line: String): Boolean {
    val trimmedStart = line.trimStart()
    return trimmedStart.startsWith("- ") || trimmedStart.startsWith("* ")
}

private fun parseMarkdownListItem(line: String): MarkdownListItem? {
    val match = Regex("^([ \\t]*)([-*])\\s+(.*)$").find(line) ?: return null
    val indentToken = match.groupValues[1]
    val text = match.groupValues[3].trim()
    val indentSpaces = indentToken.fold(0) { acc, ch ->
        acc + if (ch == '\t') 4 else 1
    }
    return MarkdownListItem(
        text = text,
        level = (indentSpaces / 2).coerceAtLeast(0)
    )
}

internal fun parseReportMarkdownSections(text: String): List<MarkdownSection> {
    val blocks = parseReportMarkdown(text)
    if (blocks.isEmpty()) {
        return emptyList()
    }

    val sections = mutableListOf<MarkdownSection>()
    var currentHeader: MarkdownBlock.Header? = null
    val currentContent = mutableListOf<MarkdownBlock>()

    fun flushSection() {
        if (currentHeader != null || currentContent.isNotEmpty()) {
            sections.add(
                MarkdownSection(
                    header = currentHeader,
                    content = currentContent.toList()
                )
            )
            currentHeader = null
            currentContent.clear()
        }
    }

    blocks.forEach { block ->
        if (block is MarkdownBlock.Header) {
            flushSection()
            currentHeader = block
        } else {
            currentContent.add(block)
        }
    }

    flushSection()
    return sections
}

internal fun parseInlineMarkdown(
    text: String,
    builder: androidx.compose.ui.text.AnnotatedString.Builder
) {
    val parts = text.split("**")
    parts.forEachIndexed { index, part ->
        if (index % 2 == 1) {
            builder.withStyle(SpanStyle(fontWeight = FontWeight.Bold)) {
                append(part)
            }
        } else {
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
