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
        val gateway = FakeConfigRuntime()
        val quickActivitiesGateway = FakeQuickActivitiesPreferenceGateway()

        val viewModel = ConfigViewModel(gateway, gateway, quickActivitiesGateway)
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
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
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
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
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
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        assertEquals(listOf("meal", "recreation"), viewModel.uiState.aliasParentOptions)
    }

    @Test
    fun invalid_advanced_toml_blocks_return_to_structured_mode() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.selectAliasEditorMode(AliasEditorMode.ADVANCED)
        viewModel.onAliasAdvancedTomlChange("parent =")
        viewModel.selectAliasEditorMode(AliasEditorMode.STRUCTURED)

        assertEquals(AliasEditorMode.ADVANCED, viewModel.uiState.aliasEditorMode)
        assertTrue(viewModel.uiState.aliasEditorErrorMessage.isNotBlank())
    }

    @Test
    fun save_rejects_duplicate_alias_key_across_other_alias_files() = runTest(dispatcher) {
        val gateway = FakeConfigRuntime()
        val viewModel = ConfigViewModel(gateway, gateway, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.addAliasEntry(parentGroupId = null, aliasKey = "zhihu", canonicalLeaf = "news")
        viewModel.saveCurrentFile()
        advanceUntilIdle()

        assertTrue(viewModel.uiState.statusText.contains("Duplicate alias key"))
        assertTrue(gateway.saveCalls.isEmpty())
    }

    @Test
    fun save_allows_multiple_aliases_for_one_canonical_leaf() = runTest(dispatcher) {
        val gateway = FakeConfigRuntime()
        val viewModel = ConfigViewModel(gateway, gateway, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.addAliasEntry(parentGroupId = null, aliasKey = "吃饭", canonicalLeaf = "dining")
        viewModel.addAliasEntry(parentGroupId = null, aliasKey = "饭", canonicalLeaf = "dining")
        viewModel.saveCurrentFile()
        advanceUntilIdle()

        assertTrue(gateway.saveCalls.any { it.first == "converter/aliases/meal.toml" })
    }

    @Test
    fun rules_files_keep_plain_toml_editor_state() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.selectConverterSubcategory(ConverterSubcategory.RULES)
        advanceUntilIdle()

        assertEquals("converter/alias_mapping.toml", viewModel.uiState.selectedFilePath)
        assertNull(viewModel.uiState.aliasDocumentDraft)
        assertEquals("", viewModel.uiState.aliasAdvancedTomlDraft)
        assertTrue(viewModel.uiState.editableContent.contains("includes"))
    }

    @Test
    fun creating_alias_toml_uses_structured_editor_with_file_name_as_parent() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.createAliasTomlFile("study")
        advanceUntilIdle()

        assertTrue(runtime.saveCalls.any { it.first == "converter/aliases/study.toml" })
        assertTrue(
            runtime.configContent("converter/alias_mapping.toml")
                .contains("\"aliases/study.toml\"")
        )
        assertEquals("converter/aliases/study.toml", viewModel.uiState.selectedFilePath)
        assertEquals(AliasEditorMode.STRUCTURED, viewModel.uiState.aliasEditorMode)
        assertEquals("study", viewModel.uiState.aliasDocumentDraft?.parent)
    }

    @Test
    fun creating_toml_rejects_path_like_file_name() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.createAliasTomlFile("../outside")
        advanceUntilIdle()

        assertTrue(runtime.saveCalls.isEmpty())
        assertTrue(viewModel.uiState.statusText.contains("single non-empty file name"))
    }

    @Test
    fun deleting_alias_toml_removes_it_from_mapping_index_and_file_list() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.deleteCurrentAliasTomlFile()
        advanceUntilIdle()

        assertTrue(!runtime.hasConfigFile("converter/aliases/meal.toml"))
        assertTrue(
            !runtime.configContent("converter/alias_mapping.toml")
                .contains("\"aliases/meal.toml\"")
        )
        assertTrue(viewModel.uiState.converterFiles.none { it.relativePath == "converter/aliases/meal.toml" })
    }

    @Test
    fun switching_plain_toml_files_restores_unsaved_draft_when_returning() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.selectConverterSubcategory(ConverterSubcategory.RULES)
        advanceUntilIdle()
        viewModel.onEditableContentChange("unsaved rules draft")

        viewModel.openFile("converter/aliases/meal.toml")
        advanceUntilIdle()
        viewModel.openFile("converter/alias_mapping.toml")
        advanceUntilIdle()

        assertEquals("converter/alias_mapping.toml", viewModel.uiState.selectedFilePath)
        assertEquals("unsaved rules draft", viewModel.uiState.editableContent)
    }

    @Test
    fun switching_alias_files_restores_unsaved_advanced_draft_and_mode_when_returning() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.selectAliasEditorMode(AliasEditorMode.ADVANCED)
        viewModel.onAliasAdvancedTomlChange("parent = \"meal\"\n\n[aliases.breakfast]\n\"早餐\" = \"draft\"")

        viewModel.openFile("converter/aliases/recreation.toml")
        advanceUntilIdle()
        viewModel.openFile("converter/aliases/meal.toml")
        advanceUntilIdle()

        assertEquals("converter/aliases/meal.toml", viewModel.uiState.selectedFilePath)
        assertEquals(AliasEditorMode.ADVANCED, viewModel.uiState.aliasEditorMode)
        assertTrue(viewModel.uiState.aliasAdvancedTomlDraft.contains("\"draft\""))
    }

    @Test
    fun renaming_alias_updates_toml_and_matching_txt_files_without_sync() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val quickActivitiesGateway = FakeQuickActivitiesPreferenceGateway(
            initialQuickActivities = listOf("早餐", "晚饭", "study/math")
        )
        val viewModel = ConfigViewModel(runtime, runtime, quickActivitiesGateway)
        advanceUntilIdle()

        val breakfastEntryId = requireNotNull(viewModel.uiState.aliasDocumentDraft)
            .nodes
            .filterIsInstance<AliasTomlGroup>()
            .first { it.name == "breakfast" }
            .nodes
            .filterIsInstance<AliasTomlEntry>()
            .first()
            .id

        viewModel.updateAliasEntry(
            entryId = breakfastEntryId,
            aliasKey = "早饭",
            canonicalLeaf = "breakfast"
        )
        viewModel.saveCurrentFile()
        advanceUntilIdle()

        assertTrue(runtime.saveCalls.any { it.first == "converter/aliases/meal.toml" })
        assertTrue(runtime.savedTxtWrites.containsKey("2026/2026-03.txt"))
        assertEquals(
            "0601\n0800早饭\n0900study/math\n0910-1010早饭 // keep focus remark 早餐\n",
            runtime.savedTxtWrites["2026/2026-03.txt"]
        )
        assertEquals(null, runtime.savedTxtWrites["2026/2026-04.txt"])
        assertEquals(listOf("早饭", "晚饭", "study/math"), quickActivitiesGateway.quickActivities)
        assertEquals(1L, viewModel.uiState.txtReloadRequestVersion)
        assertTrue(viewModel.uiState.statusText.contains("updated 1 TXT file"))
        assertTrue(viewModel.uiState.statusText.contains("updated Quick Access"))
    }
}

private class FakeConfigRuntime : ConfigGateway, TxtStorageGateway {

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
    )
    private val txtContents = linkedMapOf(
        "2026/2026-03.txt" to
            "0601\n0800早餐\n0900study/math\n0910-1010早餐 // keep focus remark 早餐\n",
        "2026/2026-04.txt" to "0602\n1000晚饭\n"
    )

    val saveCalls = mutableListOf<Pair<String, String>>()

    fun configContent(relativePath: String): String = fileContents.getValue(relativePath)

    fun hasConfigFile(relativePath: String): Boolean = fileContents.containsKey(relativePath)
    val savedTxtWrites = linkedMapOf<String, String>()

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = ConfigTomlListResult(
        ok = true,
        converterFiles = fileEntriesUnder("converter/"),
        chartFiles = fileEntriesUnder("charts/"),
        metaFiles = fileEntriesUnder("meta/"),
        reportFiles = fileEntriesUnder("reports/"),
        message = "ok"
    )

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

    override suspend fun deleteConfigTomlFile(relativePath: String): TxtFileContentResult {
        val removed = fileContents.remove(relativePath)
            ?: return TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = "missing fake file"
            )
        return TxtFileContentResult(
            ok = true,
            filePath = relativePath,
            content = removed,
            message = "ok"
        )
    }

    override suspend fun inspectTxtFiles(): TxtInspectionResult = TxtInspectionResult(
        ok = true,
        entries = txtContents.keys.map { path ->
            TxtInspectionEntry(
                relativePath = path,
                headerMonth = null,
                expectedCanonicalRelativePath = null,
                syncState = TxtSyncState.SYNCED,
                canOpen = true,
                message = "ok"
            )
        },
        message = "ok"
    )

    override suspend fun listTxtFiles(): TxtHistoryListResult =
        TxtHistoryListResult(ok = true, files = txtContents.keys.toList(), message = "ok")

    override suspend fun readTxtFile(relativePath: String): TxtFileContentResult {
        val content = txtContents[relativePath]
            ?: return TxtFileContentResult(
                ok = false,
                filePath = relativePath,
                content = "",
                message = "missing fake txt file"
            )
        return TxtFileContentResult(
            ok = true,
            filePath = relativePath,
            content = content,
            message = "ok"
        )
    }

    override suspend fun saveTxtFile(relativePath: String, content: String): TxtFileContentResult {
        savedTxtWrites[relativePath] = content
        txtContents[relativePath] = content
        return TxtFileContentResult(
            ok = true,
            filePath = relativePath,
            content = content,
            message = "ok"
        )
    }

    override suspend fun saveTxtFileAndSync(relativePath: String, content: String): RecordActionResult =
        RecordActionResult(ok = true, message = "ok")

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

    private fun fileEntriesUnder(directory: String): List<ConfigTomlFileEntry> = fileContents.keys
        .filter { path -> path.startsWith(directory) }
        .map { path ->
            ConfigTomlFileEntry(
                relativePath = path,
                displayName = when {
                    path.startsWith("converter/") -> path.removePrefix("converter/")
                    path.startsWith("charts/") -> path.removePrefix("charts/")
                    path.startsWith("reports/") -> path.removePrefix("reports/")
                    else -> path
                }
            )
        }
}
