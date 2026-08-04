package com.example.tracer

import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

internal class ConfigViewModel(
    private val configGateway: ConfigGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val activityHierarchyGateway: ActivityHierarchyGateway,
    private val activityHierarchyMigrationGateway: ActivityHierarchyMigrationGateway
) : ViewModel() {
    private val configFileEditor = ConfigFileEditor(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway
    )
    private val aliasEditor = ConfigAliasEditor(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway,
        activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        configFileEditor = configFileEditor,
        scope = viewModelScope,
        readState = { uiState },
        writeState = { uiState = it }
    )
    private val aliasMoveEditor = ConfigAliasMoveEditor(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway,
        activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        configFileEditor = configFileEditor,
        scope = viewModelScope,
        readState = { uiState },
        writeState = { uiState = it }
    )
    private val saveCoordinator = ConfigSaveCoordinator(
        configGateway = configGateway,
        activityHierarchyGateway = activityHierarchyGateway,
        activityHierarchyMigrationGateway = activityHierarchyMigrationGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        configFileEditor = configFileEditor,
        scope = viewModelScope,
        readState = { uiState },
        writeState = { uiState = it },
        refreshConfigFiles = ::refreshConfigFiles
    )

    /** Compatibility constructor for local tests that use one runtime fake. */
    constructor(
        configGateway: ConfigGateway,
        txtStorageGateway: TxtStorageGateway,
        quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway
    ) : this(
        configGateway = configGateway,
        txtStorageGateway = txtStorageGateway,
        quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway,
        activityHierarchyGateway = configGateway as? ActivityHierarchyGateway
            ?: UnavailableActivityHierarchyGateway,
        activityHierarchyMigrationGateway = configGateway as? ActivityHierarchyMigrationGateway
            ?: UnavailableActivityHierarchyMigrationGateway
    )
    companion object {
        private const val ALIAS_RENAME_LOG_TAG = "AliasRename"
    }

    var uiState by mutableStateOf(ConfigUiState())
        private set

    init {
        refreshConfigFiles()
    }

    fun refreshConfigFiles(showStatus: Boolean = true) {
        viewModelScope.launch {
            uiState = configFileEditor.refresh(uiState, showStatus)
        }
    }

    fun selectCategory(category: ConfigCategory) {
        viewModelScope.launch {
            uiState = configFileEditor.selectCategory(uiState, category)
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

    fun onEditableContentChange(value: String) {
        uiState = configFileEditor.updateEditableContent(uiState, value)
    }

    fun onAliasAdvancedTomlChange(value: String) = aliasEditor.onAliasAdvancedTomlChange(value)

    fun selectAliasEditorMode(mode: AliasEditorMode) = aliasEditor.selectAliasEditorMode(mode)

    fun updateAliasParent(value: String) = aliasEditor.updateAliasParent(value)

    fun renameAliasCategory(newName: String) = aliasEditor.renameAliasCategory(newName)

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
