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
import org.junit.Assert.assertFalse
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
    fun alias_category_is_selected_by_default() = runTest(dispatcher) {
        val gateway = FakeConfigRuntime()
        val quickActivitiesGateway = FakeQuickActivitiesPreferenceGateway()

        val viewModel = ConfigViewModel(gateway, gateway, quickActivitiesGateway)
        advanceUntilIdle()

        assertEquals(ConfigCategory.ALIAS, viewModel.uiState.selectedCategory)
        assertEquals("meal.toml", viewModel.uiState.selectedFileDisplayName)
        assertEquals("user/activity_hierarchy/meal.toml", viewModel.uiState.selectedFilePath)
        assertEquals(AliasEditorMode.STRUCTURED, viewModel.uiState.aliasEditorMode)
        assertNotNull(viewModel.uiState.aliasDocumentDraft)

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

        assertEquals("user/activity_hierarchy/meal.toml", viewModel.uiState.selectedFilePath)

        // Contract: choosing parent means switching to the corresponding alias file.
        viewModel.updateAliasParent("recreation")
        advanceUntilIdle()

        assertEquals("user/activity_hierarchy/recreation.toml", viewModel.uiState.selectedFilePath)
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
        advanceUntilIdle()

        assertEquals(AliasEditorMode.ADVANCED, viewModel.uiState.aliasEditorMode)
        assertTrue(viewModel.uiState.aliasEditorErrorMessage.isNotBlank())
    }

    @Test
    fun save_rejects_duplicate_alias_key_across_other_alias_files() = runTest(dispatcher) {
        val gateway = FakeConfigRuntime()
        val viewModel = ConfigViewModel(gateway, gateway, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.addAliasEntry(parentGroupId = null, canonicalLeaf = "news", aliases = listOf("zhihu"))
        advanceUntilIdle()

        assertTrue(viewModel.uiState.aliasEditorErrorMessage.contains("Duplicate alias key"))
        assertTrue(gateway.lastMigrationRequest == null)
    }

    @Test
    fun save_allows_multiple_aliases_for_one_canonical_leaf() = runTest(dispatcher) {
        val gateway = FakeConfigRuntime()
        val viewModel = ConfigViewModel(gateway, gateway, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.addAliasEntry(parentGroupId = null, canonicalLeaf = "dining", aliases = listOf("吃饭", "饭"))
        advanceUntilIdle()

        assertTrue(gateway.lastMigrationRequest != null)
    }

    @Test
    fun creating_alias_toml_uses_structured_editor_with_file_name_as_parent() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.createAliasTomlFile("study")
        advanceUntilIdle()

        assertTrue(runtime.hasConfigFile("user/activity_hierarchy/study.toml"))
        assertEquals(
            "parent = \"study\"\n\n[aliases]\n",
            runtime.configContent("user/activity_hierarchy/study.toml")
        )
        assertEquals("user/activity_hierarchy/study.toml", viewModel.uiState.selectedFilePath)
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
    fun deleting_alias_toml_removes_it_from_file_list() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.deleteCurrentAliasTomlFile()
        advanceUntilIdle()

        assertTrue(!runtime.hasConfigFile("user/activity_hierarchy/meal.toml"))
        assertTrue(viewModel.uiState.aliasFiles.none { it.relativePath == "user/activity_hierarchy/meal.toml" })
    }

    @Test
    fun switching_alias_files_restores_unsaved_advanced_draft_and_mode_when_returning() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.selectAliasEditorMode(AliasEditorMode.ADVANCED)
        viewModel.onAliasAdvancedTomlChange("parent = \"meal\"\n\n[aliases.breakfast]\n\"draft\" = [\"早餐\"]")

        viewModel.openFile("user/activity_hierarchy/recreation.toml")
        advanceUntilIdle()
        viewModel.openFile("user/activity_hierarchy/meal.toml")
        advanceUntilIdle()

        assertEquals("user/activity_hierarchy/meal.toml", viewModel.uiState.selectedFilePath)
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
            canonicalLeaf = "breakfast",
            aliases = listOf("早饭")
        )
        viewModel.saveCurrentFile()
        advanceUntilIdle()

        assertEquals("user/activity_hierarchy/meal.toml", runtime.lastMigrationRequest?.configRelativePath)
        assertEquals(
            AliasKeyReplacement("早餐", "早饭"),
            runtime.lastMigrationRequest?.replacementPlan?.aliases?.single()
        )
        assertEquals(listOf("早饭", "晚饭", "study/math"), quickActivitiesGateway.quickActivities)
        assertEquals(1L, viewModel.uiState.txtReloadRequestVersion)
    }

    @Test
    fun renaming_activity_category_sends_core_result_with_file_rename() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.renameAliasCategory("daily")
        advanceUntilIdle()

        val request = requireNotNull(runtime.lastMigrationRequest)
        assertEquals("user/activity_hierarchy/meal.toml", request.configRelativePath)
        assertEquals(
            ActivityHierarchyDocumentRename(
                oldSourceName = "user/activity_hierarchy/meal.toml",
                newSourceName = "user/activity_hierarchy/daily.toml"
            ),
            request.configFileRename
        )
        assertEquals("meal", request.replacementPlan.canonical.firstOrNull()?.oldCanonical?.substringBefore('_'))
        assertTrue(!runtime.hasConfigFile("user/activity_hierarchy/meal.toml"))
        assertTrue(runtime.hasConfigFile("user/activity_hierarchy/daily.toml"))
        assertEquals("user/activity_hierarchy/daily.toml", viewModel.uiState.selectedFilePath)
        assertEquals("daily", viewModel.uiState.aliasDocumentDraft?.parent)
    }

    @Test
    fun renaming_default_activity_category_updates_the_new_toml_name_and_content() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        assertEquals("user/activity_hierarchy/meal.toml", viewModel.uiState.selectedFilePath)
        assertEquals("meal", viewModel.uiState.aliasDocumentDraft?.parent)

        viewModel.renameAliasCategory("daily")
        advanceUntilIdle()

        assertFalse(runtime.hasConfigFile("user/activity_hierarchy/meal.toml"))
        assertTrue(runtime.hasConfigFile("user/activity_hierarchy/daily.toml"))
        assertTrue(runtime.configContent("user/activity_hierarchy/daily.toml").contains("parent = \"daily\""))
        assertTrue(runtime.configContent("user/activity_hierarchy/daily.toml").contains("\"breakfast\" = [\"早餐\"]"))
        assertEquals("daily.toml", viewModel.uiState.selectedFileDisplayName)
        assertEquals("user/activity_hierarchy/daily.toml", viewModel.uiState.selectedFilePath)
        assertEquals("daily", viewModel.uiState.aliasDocumentDraft?.parent)
    }

    @Test
    fun failed_activity_category_rename_keeps_the_original_editor_state() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime(
            migrationFailure = ActivityHierarchyMigrationResult(
                ok = false,
                message = "candidate database rebuild failed"
            )
        )
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        viewModel.renameAliasCategory("daily")
        advanceUntilIdle()

        assertEquals("user/activity_hierarchy/meal.toml", viewModel.uiState.selectedFilePath)
        assertEquals("meal", viewModel.uiState.aliasDocumentDraft?.parent)
        assertFalse(runtime.hasConfigFile("user/activity_hierarchy/daily.toml"))
        assertEquals(ConfigAutoSaveStatus.FAILED, viewModel.uiState.autoSaveStatus)
        assertEquals("candidate database rebuild failed", viewModel.uiState.aliasEditorErrorMessage)
    }

    @Test
    fun previewing_alias_move_keeps_toml_draft_and_storage_unchanged() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        val document = requireNotNull(viewModel.uiState.aliasDocumentDraft)
        val breakfast = document.nodes
            .filterIsInstance<AliasTomlGroup>()
            .first { it.name == "breakfast" }
        val dinner = document.nodes
            .filterIsInstance<AliasTomlGroup>()
            .first { it.name == "dinner" }
        val entry = breakfast.nodes.filterIsInstance<AliasTomlEntry>().single()
        val originalToml = viewModel.uiState.aliasAdvancedTomlDraft

        viewModel.previewAliasEntryMove(entry.id, dinner.id)
        advanceUntilIdle()

        val plan = requireNotNull(viewModel.uiState.aliasEntryMovePlan)
        assertEquals("meal_breakfast_breakfast", plan.oldCanonical)
        assertEquals("meal_dinner_breakfast", plan.newCanonical)
        assertEquals(originalToml, viewModel.uiState.aliasAdvancedTomlDraft)
        assertTrue(runtime.saveCalls.isEmpty())
        assertTrue(runtime.savedTxtWrites.isEmpty())
    }

    @Test
    fun confirming_alias_move_delegates_atomic_migration_and_updates_editor_state() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()

        val document = requireNotNull(viewModel.uiState.aliasDocumentDraft)
        val breakfast = document.nodes.filterIsInstance<AliasTomlGroup>()
            .first { it.name == "breakfast" }
            .nodes.filterIsInstance<AliasTomlEntry>().first()
        val dinnerGroup = document.nodes.filterIsInstance<AliasTomlGroup>()
            .first { it.name == "dinner" }

        viewModel.previewAliasEntryMove(breakfast.id, dinnerGroup.id)
        advanceUntilIdle()
        viewModel.confirmAliasEntryMovePlan()
        advanceUntilIdle()

        val request = requireNotNull(runtime.lastMigrationRequest)
        assertEquals("meal_breakfast_breakfast", request.replacementPlan.canonical.single().oldCanonical)
        assertEquals("meal_dinner_breakfast", request.replacementPlan.canonical.single().newCanonical)
        assertTrue(request.updatedTomlContent.contains("[aliases.dinner]"))
        assertNull(viewModel.uiState.aliasEntryMovePlan)
        assertEquals(ConfigAutoSaveStatus.SAVED, viewModel.uiState.autoSaveStatus)
        assertEquals(1L, viewModel.uiState.txtReloadRequestVersion)
    }

    @Test
    fun promoting_alias_to_category_preserves_its_record_name_and_canonical_leaf() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()
        val breakfast = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "breakfast" }
            .nodes.filterIsInstance<AliasTomlEntry>().single()

        viewModel.promoteAliasEntryToGroup(breakfast.id)
        advanceUntilIdle()

        val category = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "breakfast" }
            .nodes.filterIsInstance<AliasTomlGroup>().single()
        assertEquals(listOf("早餐"), category.groupAliases)
        assertTrue(category.nodes.isEmpty())
        assertTrue(runtime.lastMigrationRequest != null)
    }

    @Test
    fun renaming_category_record_name_uses_atomic_txt_and_database_migration() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()
        val entry = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "breakfast" }
            .nodes.filterIsInstance<AliasTomlEntry>().single()
        viewModel.promoteAliasEntryToGroup(entry.id)
        advanceUntilIdle()
        val category = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "breakfast" }
            .nodes.filterIsInstance<AliasTomlGroup>().single()

        viewModel.renameGroupAlias(category.id, "早餐", "早饭")
        advanceUntilIdle()

        val request = requireNotNull(runtime.lastMigrationRequest)
        assertTrue(request.replacementPlan.canonical.isEmpty())
        assertEquals("早餐", request.replacementPlan.aliases.single().oldAlias)
        assertEquals("早饭", request.replacementPlan.aliases.single().newAlias)
        assertTrue(request.updatedTomlContent.contains("group_aliases = [\"早饭\"]"))
    }

    @Test
    fun changing_group_alias_list_migrates_the_renamed_alias_token() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()
        val entry = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "dinner" }
            .nodes.filterIsInstance<AliasTomlEntry>().single()
        viewModel.promoteAliasEntryToGroup(entry.id)
        advanceUntilIdle()
        val category = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "dinner" }

        viewModel.updateGroupAliases(category.id, listOf("晚饭", "晚餐"))
        advanceUntilIdle()

        val request = requireNotNull(runtime.lastMigrationRequest)
        assertTrue(request.replacementPlan.canonical.isEmpty())
        assertTrue(request.updatedTomlContent.contains("group_aliases = [\"晚饭\", \"晚餐\"]"))
    }

    @Test
    fun adding_category_record_name_only_updates_the_toml_draft() = runTest(dispatcher) {
        val runtime = FakeConfigRuntime()
        val viewModel = ConfigViewModel(runtime, runtime, FakeQuickActivitiesPreferenceGateway())
        advanceUntilIdle()
        val category = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "breakfast" }

        viewModel.addGroupAlias(category.id, "早餐记录")
        advanceUntilIdle()

        val updated = requireNotNull(viewModel.uiState.aliasDocumentDraft).nodes
            .filterIsInstance<AliasTomlGroup>().first { it.name == "breakfast" }
        assertEquals(listOf("早餐记录"), updated.groupAliases)
        assertTrue(runtime.lastMigrationRequest != null)
    }
}

private class FakeConfigRuntime(
    private val migrationFailure: ActivityHierarchyMigrationResult? = null
) :
    ConfigGateway,
    ActivityHierarchyGateway,
    TxtStorageGateway,
    ActivityHierarchyMigrationGateway {

    private val fileContents = linkedMapOf(
        "user/activity_hierarchy/meal.toml" to """
            parent = "meal"

            [aliases.breakfast]
            "breakfast" = ["早餐"]

            [aliases.dinner]
            "dinner" = ["晚饭"]
        """.trimIndent(),
        "user/activity_hierarchy/recreation.toml" to """
            parent = "recreation"

            [aliases.online-platforms]
            "zhihu" = ["zhihu"]

            [aliases.game]
            "minecraft" = ["minecraft"]
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
    var lastMigrationRequest: ActivityHierarchyMigrationRequest? = null

    override suspend fun applyActivityHierarchyMigration(
        request: ActivityHierarchyMigrationRequest
    ): ActivityHierarchyMigrationResult {
        lastMigrationRequest = request
        migrationFailure?.let { return it }
        val rename = request.configFileRename
        if (rename == null) {
            fileContents[request.configRelativePath] = request.updatedTomlContent
        } else {
            fileContents.remove(rename.oldSourceName)
            fileContents[rename.newSourceName] = request.updatedTomlContent
        }
        return ActivityHierarchyMigrationResult(
            ok = true,
            message = "ok",
            updatedTxtFileCount = 2,
            updatedTomlContent = request.updatedTomlContent,
            updatedConfigRelativePath = rename?.newSourceName.orEmpty()
        )
    }

    override suspend fun describeActivityHierarchy(
        tomlContent: String
    ): ActivityHierarchyDescribeResult {
        if (tomlContent.trimEnd().endsWith("parent =")) {
            return ActivityHierarchyDescribeResult(ok = false, message = "invalid alias TOML")
        }
        return ActivityHierarchyDescribeResult(ok = true, hierarchy = snapshotFor(tomlContent), message = "ok")
    }

    override suspend fun validateActivityHierarchyDocuments(
        documents: List<ActivityHierarchyDocumentInput>
    ): ActivityHierarchyValidationResult = ActivityHierarchyValidationResult(ok = true, message = "ok")

    override suspend fun applyActivityHierarchyOperation(
        tomlContent: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyOperationResult {
        if (operation.kind == ActivityHierarchyOperationKind.ADD_LEAF && operation.aliases.contains("zhihu")) {
            return ActivityHierarchyOperationResult(
                ok = false,
                updatedTomlContent = tomlContent,
                message = "Duplicate alias key: zhihu"
            )
        }
        val updatedContent = when (operation.kind) {
            ActivityHierarchyOperationKind.RENAME_PARENT -> tomlContent.replace(
                Regex("parent\\s*=\\s*\"[^\"]*\""),
                "parent = \"${operation.newName}\""
            )
            ActivityHierarchyOperationKind.APPEND_LEAF_ALIAS -> tomlContent + "\n\"${operation.canonicalKey}\" = [\"${operation.aliases.firstOrNull().orEmpty()}\"]\n"
            ActivityHierarchyOperationKind.SET_LEAF_ALIASES -> tomlContent +
                "\n\"${operation.targetPath.substringAfterLast('.') }\" = [\"${operation.aliases.joinToString("\", \"")}\"]\n"
            ActivityHierarchyOperationKind.SET_GROUP_ALIASES,
            ActivityHierarchyOperationKind.RENAME_GROUP_ALIAS,
            ActivityHierarchyOperationKind.APPEND_GROUP_ALIAS -> tomlContent +
                "\ngroup_aliases = [\"${operation.aliases.joinToString("\", \"").ifBlank { operation.newName }}\"]\n"
            else -> tomlContent
        }
        val hierarchy = when (operation.kind) {
            ActivityHierarchyOperationKind.PROMOTE_LEAF -> snapshotForPromotedBreakfast()
            ActivityHierarchyOperationKind.ADD_LEAF -> snapshotForAddedLeaf(operation.canonicalKey, operation.aliases)
            ActivityHierarchyOperationKind.SET_GROUP_ALIASES,
            ActivityHierarchyOperationKind.RENAME_GROUP_ALIAS,
            ActivityHierarchyOperationKind.APPEND_GROUP_ALIAS -> snapshotForGroupAliasUpdate(
                operation.aliases.firstOrNull() ?: operation.newName
            )
            ActivityHierarchyOperationKind.RENAME_PARENT -> snapshotFor(updatedContent)
            else -> snapshotFor(tomlContent)
        }
        return ActivityHierarchyOperationResult(
            ok = true,
            updatedTomlContent = updatedContent,
            replacementPlan = ActivityNameReplacementPlan(
                canonical = if (operation.kind == ActivityHierarchyOperationKind.RENAME_PARENT) {
                    listOf(CanonicalActivityNameReplacement("meal_breakfast", "${operation.newName}_breakfast"))
                } else if (operation.kind == ActivityHierarchyOperationKind.MOVE_LEAF) {
                    listOf(CanonicalActivityNameReplacement(
                        "meal_breakfast_breakfast",
                        "meal_dinner_breakfast"
                    ))
                } else {
                    emptyList()
                },
                aliases = when (operation.kind) {
                ActivityHierarchyOperationKind.SET_LEAF_ALIASES -> listOf(
                    AliasKeyReplacement("早餐", operation.aliases.firstOrNull().orEmpty())
                ).filter { it.newAlias.isNotBlank() && it.oldAlias != it.newAlias }
                ActivityHierarchyOperationKind.RENAME_GROUP_ALIAS -> listOf(
                    AliasKeyReplacement(operation.oldAlias, operation.newName)
                )
                else -> emptyList()
                }
            ),
            hierarchy = hierarchy,
            message = "ok"
        )
    }

    override suspend fun moveActivityHierarchyNodeBetweenDocuments(
        documents: List<ActivityHierarchyDocumentInput>,
        sourceName: String,
        destinationName: String,
        operation: ActivityHierarchyOperation
    ): ActivityHierarchyCrossDocumentOperationResult = ActivityHierarchyCrossDocumentOperationResult(
        ok = false,
        message = "cross-document move is not used by this fixture"
    )

    override suspend fun rewriteActivityHierarchyDocument(
        originalTomlContent: String,
        updatedTomlContent: String
    ): ActivityHierarchyOperationResult = ActivityHierarchyOperationResult(
        ok = true,
        updatedTomlContent = updatedTomlContent,
        hierarchy = snapshotFor(updatedTomlContent),
        message = "ok"
    )

    private fun snapshotFor(tomlContent: String): ActivityHierarchySnapshot {
        val parent = Regex("parent\\s*=\\s*\"([^\"]*)\"")
            .find(tomlContent)?.groupValues?.get(1).orEmpty()
        return when (parent) {
            "recreation" -> ActivityHierarchySnapshot(
                parent = parent,
                nodes = listOf(
                    ActivityHierarchyNode(
                        canonicalKey = "online-platforms",
                        path = "online-platforms",
                        kind = ActivityHierarchyNodeKind.GROUP,
                        aliases = emptyList(),
                        children = listOf(
                            ActivityHierarchyNode(
                                canonicalKey = "zhihu",
                                path = "online-platforms.zhihu",
                                kind = ActivityHierarchyNodeKind.LEAF,
                                aliases = listOf("zhihu"),
                                children = emptyList()
                            )
                        )
                    ),
                    ActivityHierarchyNode(
                        canonicalKey = "game",
                        path = "game",
                        kind = ActivityHierarchyNodeKind.GROUP,
                        aliases = emptyList(),
                        children = listOf(
                            ActivityHierarchyNode(
                                canonicalKey = "minecraft",
                                path = "game.minecraft",
                                kind = ActivityHierarchyNodeKind.LEAF,
                                aliases = listOf("minecraft"),
                                children = emptyList()
                            )
                        )
                    )
                )
            )
            "meal" -> snapshotForMeal()
            else -> ActivityHierarchySnapshot(parent = parent, nodes = emptyList())
        }
    }

    private fun snapshotForMeal(): ActivityHierarchySnapshot = ActivityHierarchySnapshot(
        parent = "meal",
        nodes = listOf(
            mealGroup("breakfast", "早餐"),
            mealGroup("dinner", "晚饭")
        )
    )

    private fun mealGroup(name: String, alias: String): ActivityHierarchyNode = ActivityHierarchyNode(
        canonicalKey = name,
        path = name,
        kind = ActivityHierarchyNodeKind.GROUP,
        aliases = emptyList(),
        children = listOf(
            ActivityHierarchyNode(
                canonicalKey = name,
                path = "$name.$name",
                kind = ActivityHierarchyNodeKind.LEAF,
                aliases = listOf(alias),
                children = emptyList()
            )
        )
    )

    private fun snapshotForPromotedBreakfast(): ActivityHierarchySnapshot =
        snapshotForMeal().copy(
            nodes = listOf(
                snapshotForMeal().nodes.first().copy(
                    children = listOf(
                        ActivityHierarchyNode(
                            canonicalKey = "breakfast",
                            path = "breakfast.breakfast",
                            kind = ActivityHierarchyNodeKind.GROUP,
                            aliases = listOf("早餐"),
                            children = emptyList()
                        )
                    )
                ),
                snapshotForMeal().nodes[1]
            )
        )

    private fun snapshotForAddedLeaf(
        canonicalKey: String,
        aliases: List<String>
    ): ActivityHierarchySnapshot = snapshotForMeal().copy(
        nodes = snapshotForMeal().nodes + ActivityHierarchyNode(
            canonicalKey = canonicalKey,
            path = canonicalKey,
            kind = ActivityHierarchyNodeKind.LEAF,
            aliases = aliases,
            children = emptyList()
        )
    )

    private fun snapshotForGroupAliasUpdate(alias: String): ActivityHierarchySnapshot =
        snapshotForMeal().copy(
            nodes = listOf(
                snapshotForMeal().nodes.first().copy(aliases = listOf(alias)),
                snapshotForMeal().nodes[1]
            )
        )

    override suspend fun listConfigTomlFiles(): ConfigTomlListResult = ConfigTomlListResult(
        ok = true,
        aliasFiles = fileEntriesUnder("user/activity_hierarchy/"),
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
                    path.startsWith("user/activity_hierarchy/") -> path.removePrefix("user/activity_hierarchy/")
                    path.startsWith("charts/") -> path.removePrefix("charts/")
                    path.startsWith("reports/") -> path.removePrefix("reports/")
                    else -> path
                }
            )
        }
}
