package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

internal class ActivityHierarchyEditorViewModel(
    private val configGateway: ConfigGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    private val activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway
) : ViewModel() {
    private val configFileEditor = ActivityHierarchyFileEditor(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway
    )
    private val aliasEditor = ActivityHierarchyEditor(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway,
        activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        configFileEditor = configFileEditor,
        scope = viewModelScope,
        readState = { uiState },
        writeState = { uiState = it }
    )
    private val aliasMoveEditor = ActivityHierarchyMoveEditor(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway,
        activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        configFileEditor = configFileEditor,
        scope = viewModelScope,
        readState = { uiState },
        writeState = { uiState = it }
    )
    private val saveCoordinator = ActivityHierarchySaveCoordinator(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway,
        activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        configFileEditor = configFileEditor,
        scope = viewModelScope,
        readState = { uiState },
        writeState = { uiState = it },
        refreshActivityCategories = ::openActivityCategories
    )

    var uiState by mutableStateOf(ActivityHierarchyEditorState())
        private set

    fun openActivityCategories() {
        viewModelScope.launch {
            uiState = configFileEditor.openActivityCategories(uiState)
        }
    }

    fun openFile(path: String) {
        val trimmedPath = path.trim()
        if (trimmedPath.isEmpty()) {
            return
        }
        viewModelScope.launch {
            uiState = configFileEditor.open(uiState, trimmedPath)
        }
    }

    fun onAliasAdvancedTomlChange(value: String) = aliasEditor.onAliasAdvancedTomlChange(value)

    fun selectAliasEditorMode(mode: AliasEditorMode) = aliasEditor.selectAliasEditorMode(mode)

    fun updateAliasParent(value: String) = aliasEditor.updateAliasParent(value)

    fun renameAliasCategory(newName: String) = aliasEditor.renameAliasCategory(newName)

    fun setAliasParentColor(color: String) = aliasEditor.setAliasParentColor(color)

    fun addAliasGroup(parentGroupId: String?, name: String) =
        aliasEditor.addAliasGroup(parentGroupId, name)

    fun deleteAliasGroup(groupId: String) = aliasEditor.deleteAliasGroup(groupId)

    fun addAliasEntry(parentGroupId: String?, canonicalLeaf: String, aliases: List<String>) =
        aliasEditor.addAliasEntry(parentGroupId, canonicalLeaf, aliases)

    fun updateAliasEntry(entryId: String, canonicalLeaf: String, aliases: List<String>) =
        aliasEditor.updateAliasEntry(entryId, canonicalLeaf, aliases)

    fun deleteAliasEntry(entryId: String) = aliasEditor.deleteAliasEntry(entryId)

    fun renameAliasGroup(groupId: String, name: String) = aliasEditor.renameAliasGroup(groupId, name)

    fun addAliasToEntry(entryId: String, alias: String) =
        aliasEditor.addAliasToEntry(entryId, alias)

    fun renameAliasOnEntry(entryId: String, oldAlias: String, newAlias: String) =
        aliasEditor.renameAliasOnEntry(entryId, oldAlias, newAlias)

    fun deleteAliasFromEntry(entryId: String, alias: String) =
        aliasEditor.deleteAliasFromEntry(entryId, alias)

    fun promoteAliasEntryToGroup(entryId: String) =
        aliasEditor.promoteAliasEntryToGroup(entryId)

    fun mergeAliasEntry(sourceEntryId: String, destinationEntryId: String) =
        aliasEditor.mergeAliasEntry(sourceEntryId, destinationEntryId)

    fun renameGroupAlias(groupId: String, oldAlias: String, newAlias: String) =
        aliasEditor.renameGroupAlias(groupId, oldAlias, newAlias)

    fun addGroupAlias(groupId: String, alias: String) =
        aliasEditor.addGroupAlias(groupId, alias)

    fun updateGroupAliases(groupId: String, aliases: List<String>) =
        aliasEditor.updateGroupAliases(groupId, aliases)

    fun previewAliasEntryMove(entryId: String, targetGroupId: String) =
        aliasMoveEditor.previewAliasEntryMove(entryId, targetGroupId)

    fun prepareAliasEntryMove(entryId: String) = aliasMoveEditor.prepareAliasEntryMove(entryId)

    fun prepareAliasGroupMove(groupId: String) = aliasMoveEditor.prepareAliasGroupMove(groupId)

    fun previewAliasEntryMove(entryId: String, target: AliasEntryMoveTarget) =
        aliasMoveEditor.previewAliasEntryMove(entryId, target)

    fun previewAliasGroupMove(groupId: String, target: AliasEntryMoveTarget) =
        aliasMoveEditor.previewAliasGroupMove(groupId, target)

    fun discardAliasEntryMovePlan() = aliasMoveEditor.discardAliasEntryMovePlan()

    fun confirmAliasEntryMovePlan() = aliasMoveEditor.confirmAliasEntryMovePlan()

    fun setStatusText(message: String) {
        uiState = uiState.copy(statusText = message)
    }

    fun createAliasTomlFile(fileName: String) = saveCoordinator.createAliasTomlFile(fileName)

    fun deleteCurrentAliasTomlFile() = saveCoordinator.deleteCurrentAliasTomlFile()

    fun saveCurrentFile() = saveCoordinator.saveCurrentFile()

    fun discardUnsavedDraft() = saveCoordinator.discardUnsavedDraft()

    suspend fun applyImportedAliasToml(
        relativePath: String,
        updatedTomlContent: String
    ): String? = saveCoordinator.applyImportedAliasToml(relativePath, updatedTomlContent)
}
