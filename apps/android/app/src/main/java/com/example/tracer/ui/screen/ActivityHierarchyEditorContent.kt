package com.example.tracer

import androidx.compose.runtime.Composable
import androidx.compose.runtime.LaunchedEffect

/** The sole host for canonical activity and alias maintenance. */
@Composable
internal fun ActivityHierarchyEditorContent(
    state: ActivityHierarchyEditorState,
    viewModel: ActivityHierarchyEditorViewModel
) {
    LaunchedEffect(Unit) {
        viewModel.openActivityCategories()
    }
    LaunchedEffect(state.selectedFileContent, state.aliasSearchQuery) {
        viewModel.refreshAliasSearch()
    }
    ActivityHierarchyEditorCard(
        aliasFiles = state.aliasFiles.filter { it.relativePath.startsWith("user/activity_hierarchy/") },
        selectedFileDisplayName = state.selectedFileDisplayName.removePrefix("user/activity_hierarchy/"),
        document = state.aliasSearchDocument ?: state.aliasDocumentDraft,
        searchQuery = state.aliasSearchQuery,
        movePlan = state.aliasEntryMovePlan,
        moveDestinations = state.aliasEntryMoveDestinations,
        moveDestinationsLoading = state.aliasEntryMoveDestinationsLoading,
        errorMessage = state.aliasEditorErrorMessage,
        onCreateAliasTomlFile = viewModel::createAliasTomlFile,
        onSelectAliasFile = viewModel::openFile,
        onDeleteAliasTomlFile = viewModel::deleteCurrentAliasTomlFile,
        onRenameCategory = viewModel::renameAliasCategory,
        onSetParentColor = viewModel::setAliasParentColor,
        onAddGroup = viewModel::addAliasGroup,
        onDeleteGroup = viewModel::deleteAliasGroup,
        onRenameGroup = viewModel::renameAliasGroup,
        onAddEntry = viewModel::addAliasEntry,
        onUpdateEntry = viewModel::updateAliasEntry,
        onMergeEntry = viewModel::mergeAliasEntry,
        onPromoteEntry = viewModel::promoteAliasEntryToGroup,
        onRenameGroupAlias = viewModel::renameGroupAlias,
        onAddGroupAlias = viewModel::addGroupAlias,
        onUpdateGroupAliases = viewModel::updateGroupAliases,
        onDeleteEntry = viewModel::deleteAliasEntry,
        onPrepareEntryMove = viewModel::prepareAliasEntryMove,
        onPrepareGroupMove = viewModel::prepareAliasGroupMove,
        onPreviewEntryMove = viewModel::previewAliasEntryMove,
        onPreviewGroupMove = viewModel::previewAliasGroupMove,
        onConfirmMovePlan = viewModel::confirmAliasEntryMovePlan,
        onDiscardMovePlan = viewModel::discardAliasEntryMovePlan,
        onSearchQueryChange = viewModel::updateAliasSearchQuery
    )
}
