package com.example.tracer

import androidx.compose.ui.text.buildAnnotatedString
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
}
