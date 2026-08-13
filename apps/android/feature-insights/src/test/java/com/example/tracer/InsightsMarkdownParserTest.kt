package com.example.tracer

import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontStyle
import org.junit.Assert.assertEquals
import org.junit.Test

class InsightsMarkdownParserTest {
    @Test
    fun parseInsightsMarkdown_keepsIndentedRemarkLinesInOneListItem() {
        val blocks = parseInsightsMarkdown(
            "  - **Activity Remark**:\n    测试<br>\n    测试1<br>\n"
        )

        val list = blocks.single() as MarkdownBlock.ListBlock
        assertEquals(1, list.items.size)
        assertEquals(
            "**Activity Remark**:\n测试<br>\n测试1<br>",
            list.items.single().text
        )
    }

    @Test
    fun parseInlineMarkdown_rendersHtmlBreakAsLineBreak() {
        val rendered = buildAnnotatedString {
            parseInlineMarkdown("你好<br>测试<br/>测试", this)
        }

        assertEquals("你好\n测试\n测试", rendered.text)
    }

    @Test
    fun parseInlineMarkdown_doesNotDoubleBreakHtmlBreakBeforePhysicalNewline() {
        val rendered = buildAnnotatedString {
            parseInlineMarkdown("内容<br>\n内容", this)
        }

        assertEquals("内容\n内容", rendered.text)
    }

    @Test
    fun parseInlineMarkdown_rendersSingleAsterisksAsItalicWithoutMarkers() {
        val rendered = buildAnnotatedString {
            parseInlineMarkdown("before *emphasized* after", this)
        }

        assertEquals("before emphasized after", rendered.text)
        assertEquals(1, rendered.spanStyles.size)
        assertEquals(FontStyle.Italic, rendered.spanStyles.single().item.fontStyle)
    }

    @Test
    fun parseInsightsMarkdown_recognizesSingleAsteriskListMarkers() {
        val blocks = parseInsightsMarkdown("* Item")

        val list = blocks.single() as MarkdownBlock.ListBlock
        assertEquals("Item", list.items.single().text)
    }

    @Test
    fun buildMarkdownListTree_nestsActivitiesByListIndentation() {
        val tree = buildMarkdownListTree(
            listOf(
                MarkdownListItem(text = "study", level = 0),
                MarkdownListItem(text = "math", level = 1),
                MarkdownListItem(text = "algebra", level = 2),
                MarkdownListItem(text = "english", level = 1),
                MarkdownListItem(text = "sleep", level = 0)
            )
        )

        assertEquals(listOf("study", "sleep"), tree.map { it.item.text })
        assertEquals(listOf("math", "english"), tree.first().children.map { it.item.text })
        assertEquals("algebra", tree.first().children.first().children.single().item.text)
    }
}
