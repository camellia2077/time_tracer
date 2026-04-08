package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.ExperimentalCoroutinesApi
import kotlinx.coroutines.test.StandardTestDispatcher
import kotlinx.coroutines.test.advanceUntilIdle
import kotlinx.coroutines.test.resetMain
import kotlinx.coroutines.test.runTest
import kotlinx.coroutines.test.setMain
import org.junit.After
import org.junit.Assert.assertEquals
import org.junit.Assert.assertNotNull
import org.junit.Assert.assertNull
import org.junit.Assert.assertTrue
import org.junit.Before
import org.junit.Test

@OptIn(ExperimentalCoroutinesApi::class)
class ConfigViewModelTest {
    private val dispatcher = StandardTestDispatcher()

    @Before
    fun setUp() {
        Dispatchers.setMain(dispatcher)
    }

    @After
    fun tearDown() {
        Dispatchers.resetMain()
    }

    @Test
    fun converter_defaults_to_aliases_and_can_switch_to_rules() = runTest(dispatcher) {
        val gateway = FakeConfigGateway()

        val viewModel = ConfigViewModel(gateway)
        advanceUntilIdle()

        assertEquals(ConfigCategory.CONVERTER, viewModel.uiState.selectedCategory)
        assertEquals(ConverterSubcategory.ALIASES, viewModel.uiState.selectedConverterSubcategory)
        assertEquals("aliases/meal.toml", viewModel.uiState.selectedFileDisplayName)
        assertEquals("converter/aliases/meal.toml", viewModel.uiState.selectedFilePath)
        assertEquals(AliasEditorMode.STRUCTURED, viewModel.uiState.aliasEditorMode)
        assertNotNull(viewModel.uiState.aliasDocumentDraft)

        viewModel.selectConverterSubcategory(ConverterSubcategory.RULES)
        advanceUntilIdle()

        assertEquals(ConverterSubcategory.RULES, viewModel.uiState.selectedConverterSubcategory)
        assertEquals("alias_mapping.toml", viewModel.uiState.selectedFileDisplayName)
        assertEquals("converter/alias_mapping.toml", viewModel.uiState.selectedFilePath)
        assertNull(viewModel.uiState.aliasDocumentDraft)
        assertTrue(viewModel.uiState.editableContent.contains("includes"))
    }

    @Test
    fun selecting_advanced_mode_uses_current_structured_draft() = runTest(dispatcher) {
        val viewModel = ConfigViewModel(FakeConfigGateway())
        advanceUntilIdle()

        // Parent updates are now async because selection can trigger file switch.
        viewModel.updateAliasParent("meal-updated")
        advanceUntilIdle()
        viewModel.selectAliasEditorMode(AliasEditorMode.ADVANCED)

        assertEquals(AliasEditorMode.ADVANCED, viewModel.uiState.aliasEditorMode)
        assertTrue(viewModel.uiState.aliasAdvancedTomlDraft.contains("parent = \"meal-updated\""))
        assertTrue(viewModel.uiState.aliasAdvancedTomlDraft.contains("[aliases.breakfast]"))
    }

    @Test
    fun selecting_parent_switches_to_matching_alias_file_content() = runTest(dispatcher) {
        val viewModel = ConfigViewModel(FakeConfigGateway())
        advanceUntilIdle()

        assertEquals("converter/aliases/meal.toml", viewModel.uiState.selectedFilePath)

        // Contract: choosing parent means switching to the corresponding alias file.
        viewModel.updateAliasParent("recreation")
        advanceUntilIdle()

        assertEquals("converter/aliases/recreation.toml", viewModel.uiState.selectedFilePath)
        assertEquals("recreation", viewModel.uiState.aliasDocumentDraft?.parent)
        assertTrue(viewModel.uiState.selectedFileContent.contains("[aliases.online-platforms]"))
    }

    @Test
    fun alias_parent_options_are_collected_from_existing_alias_files() = runTest(dispatcher) {
        val viewModel = ConfigViewModel(FakeConfigGateway())
        advanceUntilIdle()

        assertEquals(listOf("meal", "recreation"), viewModel.uiState.aliasParentOptions)
    }

    @Test
    fun invalid_advanced_toml_blocks_return_to_structured_mode() = runTest(dispatcher) {
        val viewModel = ConfigViewModel(FakeConfigGateway())
        advanceUntilIdle()

        viewModel.selectAliasEditorMode(AliasEditorMode.ADVANCED)
        viewModel.onAliasAdvancedTomlChange("parent =")
        viewModel.selectAliasEditorMode(AliasEditorMode.STRUCTURED)

        assertEquals(AliasEditorMode.ADVANCED, viewModel.uiState.aliasEditorMode)
        assertTrue(viewModel.uiState.aliasEditorErrorMessage.isNotBlank())
    }

    @Test
    fun save_rejects_duplicate_alias_key_across_other_alias_files() = runTest(dispatcher) {
        val gateway = FakeConfigGateway()
        val viewModel = ConfigViewModel(gateway)
        advanceUntilIdle()

        viewModel.addAliasEntry(parentGroupId = null, aliasKey = "zhihu", canonicalLeaf = "news")
        viewModel.saveCurrentFile()
        advanceUntilIdle()

        assertTrue(viewModel.uiState.statusText.contains("Duplicate alias key"))
        assertTrue(gateway.saveCalls.isEmpty())
    }

    @Test
    fun rules_files_keep_plain_toml_editor_state() = runTest(dispatcher) {
        val viewModel = ConfigViewModel(FakeConfigGateway())
        advanceUntilIdle()

        viewModel.selectConverterSubcategory(ConverterSubcategory.RULES)
        advanceUntilIdle()

        assertEquals("converter/alias_mapping.toml", viewModel.uiState.selectedFilePath)
        assertNull(viewModel.uiState.aliasDocumentDraft)
        assertEquals("", viewModel.uiState.aliasAdvancedTomlDraft)
        assertTrue(viewModel.uiState.editableContent.contains("includes"))
    }
}

private class FakeConfigGateway : ConfigGateway {
    private val entries = ConfigTomlListResult(
        ok = true,
        converterFiles = listOf(
            ConfigTomlFileEntry("converter/alias_mapping.toml", "alias_mapping.toml"),
            ConfigTomlFileEntry("converter/aliases/meal.toml", "aliases/meal.toml"),
            ConfigTomlFileEntry("converter/aliases/recreation.toml", "aliases/recreation.toml"),
            ConfigTomlFileEntry("converter/duration_rules.toml", "duration_rules.toml")
        ),
        chartFiles = emptyList(),
        metaFiles = emptyList(),
        reportFiles = emptyList(),
        message = "ok"
    )

    private val fileContents = linkedMapOf(
        "converter/alias_mapping.toml" to """
            includes = [
              "aliases/meal.toml",
              "aliases/recreation.toml",
            ]
        """.trimIndent(),
        "converter/aliases/meal.toml" to """
            parent = "meal"

            [aliases.breakfast]
            "早餐" = "breakfast"

            [aliases.dinner]
            "晚饭" = "dinner"
        """.trimIndent(),
        "converter/aliases/recreation.toml" to """
            parent = "recreation"

            [aliases.online-platforms]
            "zhihu" = "zhihu"

            [aliases.game]
            "minecraft" = "minecraft"
        """.trimIndent(),
        "converter/duration_rules.toml" to """
            [rules.default]
            min_minutes = 15
        """.trimIndent()
    )

    val saveCalls = mutableListOf<Pair<String, String>>()

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = entries

    override suspend fun readConfigTomlFile(relativePath: String): TxtFileContentResult {
        val content = fileContents[relativePath]
            ?: return TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = "missing fake file"
            )
        return TxtFileContentResult(
            ok = true,
            filePath = relativePath,
            content = content,
            message = "ok"
        )
    }

    override suspend fun saveConfigTomlFile(
        relativePath: String,
        content: String
    ): TxtFileContentResult {
        saveCalls += relativePath to content
        fileContents[relativePath] = content
        return TxtFileContentResult(
            ok = true,
            filePath = relativePath,
            content = content,
            message = "ok"
        )
    }

    override suspend fun listRecentDiagnostics(limit: Int): RuntimeDiagnosticsListResult =
        RuntimeDiagnosticsListResult(
            ok = true,
            entries = emptyList(),
            message = "ok",
            diagnosticsLogPath = ""
        )

    override suspend fun buildDiagnosticsPayload(maxEntries: Int): RuntimeDiagnosticsPayloadResult =
        RuntimeDiagnosticsPayloadResult(
            ok = true,
            payload = "",
            message = "ok",
            entryCount = 0,
            diagnosticsLogPath = ""
        )
}
