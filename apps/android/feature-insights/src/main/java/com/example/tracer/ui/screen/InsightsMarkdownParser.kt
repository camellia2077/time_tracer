package com.example.tracer

import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontStyle
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

internal fun parseInsightsMarkdown(text: String): List<MarkdownBlock> {
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
        } else if (line.isNotBlank() && line.firstOrNull()?.isWhitespace() == true &&
            listBuffer.isNotEmpty()
        ) {
            // Formatter-generated indented lines are hard-break continuations
            // of the preceding list item, not separate paragraphs. Keeping
            // them in one item prevents the list renderer's paragraph spacing
            // from adding an extra blank line around each <br>.
            val continuation = line.trim()
            val lastIndex = listBuffer.lastIndex
            listBuffer[lastIndex] = listBuffer[lastIndex].copy(
                text = listBuffer[lastIndex].text + "\n" + continuation
            )
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

internal fun parseInsightsMarkdownSections(text: String): List<MarkdownSection> {
    val blocks = parseInsightsMarkdown(text)
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
    // The shared Markdown formatter uses <br> for intentional hard breaks.
    // The in-app renderer is deliberately lightweight, so normalize the tag
    // here instead of showing it as literal text in the preview.
    // Core's Markdown formatter emits `<br>` at the end of each activity-remark line,
    // followed by the physical newline separating the next Markdown source line. The
    // Android preview must treat that pair as one visual line break; converting both
    // independently would turn one remark continuation into an unintended blank line.
    val normalizedText = text
        .replace(Regex("(?i)<br\\s*/?>[ \\t]*\\n"), "\n")
        .replace(Regex("(?i)<br\\s*/?>"), "\n")
    appendInlineMarkdown(normalizedText, builder)
}

private fun appendInlineMarkdown(
    text: String,
    builder: androidx.compose.ui.text.AnnotatedString.Builder
) {
    var index = 0
    while (index < text.length) {
        when {
            text.startsWith("**", index) -> {
                val closingIndex = text.indexOf("**", index + 2)
                if (closingIndex < 0) {
                    builder.append("**")
                    index += 2
                } else {
                    builder.withStyle(SpanStyle(fontWeight = FontWeight.Bold)) {
                        appendInlineMarkdown(text.substring(index + 2, closingIndex), this)
                    }
                    index = closingIndex + 2
                }
            }

            text[index] == '*' -> {
                val closingIndex = findClosingSingleAsterisk(text, index + 1)
                if (closingIndex < 0) {
                    builder.append('*')
                    index += 1
                } else {
                    builder.withStyle(SpanStyle(fontStyle = FontStyle.Italic)) {
                        appendInlineMarkdown(text.substring(index + 1, closingIndex), this)
                    }
                    index = closingIndex + 1
                }
            }

            text[index] == '`' -> {
                val closingIndex = text.indexOf('`', index + 1)
                if (closingIndex < 0) {
                    builder.append('`')
                    index += 1
                } else {
                    builder.withStyle(
                        SpanStyle(
                            fontFamily = FontFamily.Monospace,
                            background = Color.LightGray.copy(alpha = 0.3f)
                        )
                    ) {
                        append(text.substring(index + 1, closingIndex))
                    }
                    index = closingIndex + 1
                }
            }

            else -> {
                builder.append(text[index])
                index += 1
            }
        }
    }
}

private fun findClosingSingleAsterisk(text: String, startIndex: Int): Int {
    var index = startIndex
    while (index < text.length) {
        if (text[index] == '*') {
            val isPartOfDoubleMarker =
                (index > 0 && text[index - 1] == '*') ||
                    (index + 1 < text.length && text[index + 1] == '*')
            if (!isPartOfDoubleMarker) {
                return index
            }
        }
        index += 1
    }
    return -1
}
