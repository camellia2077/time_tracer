package com.example.tracer

import android.util.Log
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.lifecycle.ViewModel
import androidx.lifecycle.ViewModelProvider
import androidx.lifecycle.viewModelScope
import kotlinx.coroutines.launch

internal enum class ConfigCategory {
    // Alias files live under `aliases/*.toml`.
    ALIAS,
    // Charts = `charts/*.toml`
    CHARTS,
    // Meta = `config.toml` plus `meta/*.toml`
    META,
    // Reports = `reports/**/*.toml`
    REPORTS
}

internal enum class ConfigAutoSaveStatus {
    IDLE,
    SAVING,
    SAVED,
    FAILED
}

internal data class ConfigUiState(
    val selectedCategory: ConfigCategory = ConfigCategory.ALIAS,
    val aliasFiles: List<ConfigTomlFileEntry> = emptyList(),
    val chartFiles: List<ConfigTomlFileEntry> = emptyList(),
    val metaFiles: List<ConfigTomlFileEntry> = emptyList(),
    val reportFiles: List<ConfigTomlFileEntry> = emptyList(),
    val selectedFilePath: String = "",
    val selectedFileDisplayName: String = "",
    val selectedFileContent: String = "",
    val editableContent: String = "",
    // Keep unsaved drafts in memory per file so switching tabs/files behaves like a normal text
    // editor: the user sees their in-session edits again without silently writing them to disk.
    val plainTomlDraftsByFile: Map<String, String> = emptyMap(),
    val aliasEditorMode: AliasEditorMode = AliasEditorMode.STRUCTURED,
    val aliasDocumentDraft: AliasTomlDocument? = null,
    val aliasBaselineDocument: AliasTomlDocument? = null,
    val aliasParentOptions: List<String> = emptyList(),
    val aliasAdvancedTomlDraft: String = "",
    val aliasStructuredDraftsByFile: Map<String, AliasTomlDocument> = emptyMap(),
    val aliasAdvancedDraftsByFile: Map<String, String> = emptyMap(),
    val aliasEditorModeByFile: Map<String, AliasEditorMode> = emptyMap(),
    // A relocation plan is intentionally preview-only in this iteration. It
    // must not alter the TOML draft before TXT and database migration exist.
    val aliasEntryMovePlan: AliasEntryMovePlan? = null,
    val aliasEditorErrorMessage: String = "",
    val txtReloadRequestVersion: Long = 0L,
    val autoSaveStatus: ConfigAutoSaveStatus = ConfigAutoSaveStatus.IDLE,
    val statusText: String = "Preparing config..."
)

internal class ConfigViewModel(
    private val configGateway: ConfigGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway,
    private val aliasMoveMigrationGateway: AliasMoveMigrationGateway? =
        configGateway as? AliasMoveMigrationGateway
) : ViewModel() {
    private val aliasMigrationUseCase = aliasMoveMigrationGateway?.let(::ActivityAliasMigrationUseCase)
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
            uiState = uiState.copy(statusText = if (showStatus) {
                "refreshing config toml..."
            } else {
                ""
            })
            val listResult = configGateway.listConfigTomlFiles()
            if (!listResult.ok) {
                uiState = uiState.copy(statusText = listResult.message)
                return@launch
            }

            val updated = uiState.copy(
                aliasFiles = listResult.aliasFiles,
                chartFiles = listResult.chartFiles,
                metaFiles = listResult.metaFiles,
                reportFiles = listResult.reportFiles
            )
            val files = configFilesForCategory(updated, updated.selectedCategory)
            val targetFile = preferredConfigFilePath(updated, files)
            if (targetFile.isEmpty()) {
                uiState = clearSelectedConfigFile(
                    updated,
                    statusText = if (showStatus) listResult.message else ""
                )
                return@launch
            }

            uiState = readConfigFileIntoState(
                baseState = updated,
                path = targetFile,
                statusText = if (showStatus) listResult.message else ""
            )
        }
    }

    fun selectCategory(category: ConfigCategory) {
        viewModelScope.launch {
            val changed = uiState.copy(selectedCategory = category)
            val files = configFilesForCategory(changed, category)
            if (files.isEmpty()) {
                uiState = clearSelectedConfigFile(
                    changed,
                    statusText = "No TOML files in ${category.name.lowercase()}."
                )
                return@launch
            }

            uiState = readConfigFileIntoState(
                baseState = changed,
                path = preferredConfigFilePath(changed, files),
                statusText = "open toml -> ${preferredConfigFilePath(changed, files)}"
            )
        }
    }

    fun openFile(path: String) {
        val trimmedPath = path.trim()
        if (trimmedPath.isEmpty()) {
            return
        }
        viewModelScope.launch {
            uiState = readConfigFileIntoState(
                baseState = uiState,
                path = trimmedPath,
                statusText = "open toml -> $trimmedPath"
            )
        }
    }

    fun onEditableContentChange(value: String) {
        val selectedFile = uiState.selectedFilePath
        val nextDrafts = uiState.plainTomlDraftsByFile.toMutableMap()
        if (selectedFile.isNotBlank()) {
            if (value == uiState.selectedFileContent) {
                nextDrafts.remove(selectedFile)
            } else {
                nextDrafts[selectedFile] = value
            }
        }
        uiState = uiState.copy(
            editableContent = value,
            plainTomlDraftsByFile = nextDrafts
        )
    }

    fun onAliasAdvancedTomlChange(value: String) {
        val selectedFile = uiState.selectedFilePath
        val nextAdvancedDrafts = uiState.aliasAdvancedDraftsByFile.toMutableMap()
        val nextModeByFile = uiState.aliasEditorModeByFile.toMutableMap()
        if (selectedFile.isNotBlank()) {
            if (value == uiState.selectedFileContent) {
                nextAdvancedDrafts.remove(selectedFile)
            } else {
                nextAdvancedDrafts[selectedFile] = value
            }
            nextModeByFile[selectedFile] = AliasEditorMode.ADVANCED
        }
        uiState = uiState.copy(
            aliasAdvancedTomlDraft = value,
            aliasAdvancedDraftsByFile = nextAdvancedDrafts,
            aliasEditorModeByFile = nextModeByFile
        )
    }

    fun selectAliasEditorMode(mode: AliasEditorMode) {
        if (!isAliasConfigFilePath(uiState.selectedFilePath) || uiState.aliasEditorMode == mode) {
            return
        }
        if (mode == AliasEditorMode.ADVANCED) {
            uiState = cacheAliasAdvancedMode(switchAliasEditorToAdvanced(uiState))
            return
        }
        val gateway = configGateway as? AliasHierarchyGateway ?: run {
            uiState = uiState.copy(aliasEditorErrorMessage = "Alias hierarchy runtime is unavailable.")
            return
        }
        val rawToml = uiState.aliasAdvancedTomlDraft
        viewModelScope.launch {
            val result = gateway.describeAliasHierarchy(rawToml)
            val document = result.hierarchy?.toActivityAliasDocument()
            if (!result.ok || document == null) {
                val message = result.message.ifBlank { "Alias hierarchy validation failed." }
                uiState = uiState.copy(aliasEditorErrorMessage = message, statusText = message)
                return@launch
            }
            uiState = cacheAliasStructuredMode(uiState.copy(
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = document,
                aliasParentOptions = normalizeAliasParentOptions(
                    uiState.aliasParentOptions + document.parent
                ),
                aliasEditorErrorMessage = ""
            ))
        }
    }

    fun updateAliasParent(value: String) {
        val normalizedValue = value.trim()
        if (normalizedValue.isEmpty() || !isAliasConfigFilePath(uiState.selectedFilePath)) {
            return
        }
        viewModelScope.launch {
            val currentFilePath = uiState.selectedFilePath
            val targetFilePath = resolveAliasFilePathForParent(
                configGateway = configGateway,
                aliasFiles = uiState.aliasFiles,
                currentFilePath = currentFilePath,
                currentAliasDocument = uiState.aliasDocumentDraft,
                parent = normalizedValue
            )
            if (targetFilePath != null && targetFilePath != currentFilePath) {
                uiState = readConfigFileIntoState(
                    baseState = uiState,
                    path = targetFilePath,
                    statusText = "open toml -> $targetFilePath"
                )
                return@launch
            }

            applyCoreAliasHierarchyOperation(
                AliasHierarchyOperation(kind = "rename_parent", newName = normalizedValue)
            )
        }
    }

    fun addAliasGroup(parentGroupId: String?, name: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedName = name.trim()
        if (normalizedName.isEmpty()) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Alias group name must not be empty.")
            return
        }
        applyCoreAliasHierarchyOperation(
            AliasHierarchyOperation(
                kind = "add_group",
                targetPath = parentGroupId?.let(document::canonicalTargetPathForGroup) ?: "root",
                canonicalKey = normalizedName
            )
        )
    }

    fun deleteAliasGroup(groupId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation("delete_group", targetPath = path))
    }

    fun addAliasEntry(parentGroupId: String?, canonicalLeaf: String, aliases: List<String>) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedCanonicalLeaf = canonicalLeaf.trim()
        val normalizedAliases = aliases.map(String::trim).filter(String::isNotEmpty)
        if (normalizedCanonicalLeaf.isEmpty() || normalizedAliases.isEmpty()) {
            uiState = uiState.copy(
                aliasEditorErrorMessage = "Canonical and at least one alias must not be empty."
            )
            return
        }
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "add_leaf",
            targetPath = parentGroupId?.let(document::canonicalTargetPathForGroup) ?: "root",
            canonicalKey = normalizedCanonicalLeaf,
            aliases = normalizedAliases
        ))
    }

    fun updateAliasEntry(entryId: String, canonicalLeaf: String, aliases: List<String>) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedCanonicalLeaf = canonicalLeaf.trim()
        val normalizedAliases = aliases.map(String::trim).filter(String::isNotEmpty)
        if (normalizedCanonicalLeaf.isEmpty() || normalizedAliases.isEmpty()) {
            uiState = uiState.copy(
                aliasEditorErrorMessage = "Canonical and at least one alias must not be empty."
            )
            return
        }
        val canonicalTargetPath = document.canonicalTargetPathForEntry(entryId)
        val oldLeaf = document.findAliasEntry(entryId)?.canonicalLeaf
        if (canonicalTargetPath == null || oldLeaf == null) return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = if (oldLeaf == normalizedCanonicalLeaf) "set_leaf_aliases" else "rename_leaf_canonical",
            targetPath = canonicalTargetPath,
            newName = normalizedCanonicalLeaf,
            aliases = normalizedAliases
        ))
    }

    fun deleteAliasEntry(entryId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation("delete_leaf", targetPath = path))
    }

    private fun applyCoreAliasHierarchyOperation(operation: AliasHierarchyOperation) {
        val gateway = configGateway as? AliasHierarchyGateway ?: run {
            uiState = uiState.copy(aliasEditorErrorMessage = "Alias hierarchy runtime is unavailable.")
            return
        }
        val content = uiState.aliasAdvancedTomlDraft.ifBlank { uiState.selectedFileContent }
        val selectedFile = uiState.selectedFilePath
        viewModelScope.launch {
            val result = gateway.applyAliasHierarchyOperation(content, operation)
            val document = result.hierarchy?.toActivityAliasDocument()
            if (!result.ok || document == null) {
                uiState = uiState.copy(aliasEditorErrorMessage = result.message.ifBlank { "Alias hierarchy operation failed." })
                return@launch
            }
            // Every Core-produced alias TOML is persisted transactionally,
            // including operations whose TXT replacement list is empty.
            val requiresMigration = true
            val migratedToml = if (requiresMigration) {
                uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
                val migration = aliasMigrationUseCase?.applyCoreResult(
                    configRelativePath = selectedFile,
                    updatedTomlContent = result.updatedTomlContent,
                    replacements = result.replacements,
                    aliasReplacements = result.aliasReplacements
                ) ?: ActivityAliasMigrationOutcome.Invalid("Alias migration runtime is unavailable.")
                if (migration is ActivityAliasMigrationOutcome.Invalid) {
                    uiState = uiState.copy(
                        aliasEditorErrorMessage = migration.message,
                        autoSaveStatus = ConfigAutoSaveStatus.FAILED
                    )
                    return@launch
                }
                if (result.aliasReplacements.isNotEmpty()) {
                    val replacements = result.aliasReplacements.associate {
                        it.oldAlias to it.newAlias
                    }
                    val quickAccessUpdate = runCatching {
                        quickActivitiesPreferenceGateway.setQuickActivities(
                            quickActivitiesPreferenceGateway.getQuickActivities().map { value ->
                                replacements[value] ?: value
                            }
                        )
                    }.exceptionOrNull()
                    if (quickAccessUpdate != null) {
                        uiState = uiState.copy(
                            aliasEditorErrorMessage = quickAccessUpdate.message
                                ?: "Quick Access alias migration failed.",
                            autoSaveStatus = ConfigAutoSaveStatus.FAILED
                        )
                        return@launch
                    }
                }
                (migration as ActivityAliasMigrationOutcome.Applied).renderedToml
            } else {
                result.updatedTomlContent
            }
            val parentOptions = if (requiresMigration) {
                resolveAliasParentOptions(
                    configGateway = configGateway,
                    aliasFiles = uiState.aliasFiles,
                    selectedFilePath = selectedFile,
                    selectedFileContent = migratedToml
                )
            } else {
                uiState.aliasParentOptions
            }
            uiState = uiState.copy(
                aliasDocumentDraft = document,
                aliasBaselineDocument = if (requiresMigration) {
                    document
                } else {
                    uiState.aliasBaselineDocument
                },
                selectedFileContent = if (requiresMigration) migratedToml else uiState.selectedFileContent,
                aliasAdvancedTomlDraft = migratedToml,
                aliasStructuredDraftsByFile = cacheStructuredDraft(
                    selectedFile,
                    document,
                    migratedToml
                ),
                aliasEditorModeByFile = cacheAliasMode(selectedFile, AliasEditorMode.STRUCTURED),
                aliasEntryMovePlan = null,
                aliasParentOptions = parentOptions,
                aliasEditorErrorMessage = "",
                txtReloadRequestVersion = if (requiresMigration) {
                    uiState.txtReloadRequestVersion + 1
                } else {
                    uiState.txtReloadRequestVersion
                },
                autoSaveStatus = if (requiresMigration) {
                    ConfigAutoSaveStatus.SAVED
                } else {
                    uiState.autoSaveStatus
                }
            )
        }
    }

    fun renameAliasGroup(groupId: String, name: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedName = name.trim()
        if (normalizedName.isEmpty()) return
        val targetPath = document.canonicalTargetPathForGroup(groupId)
        if (targetPath == null) return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "rename_group_canonical", targetPath = targetPath, newName = normalizedName
        ))
    }

    fun addAliasToEntry(entryId: String, alias: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAlias = alias.trim()
        if (normalizedAlias.isEmpty()) return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        val parentPath = path.substringBeforeLast('.', missingDelimiterValue = "root")
        val key = path.substringAfterLast('.')
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "append_leaf_alias", targetPath = parentPath,
            canonicalKey = key, aliases = listOf(normalizedAlias)
        ))
    }

    fun renameAliasOnEntry(entryId: String, oldAlias: String, newAlias: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAlias = newAlias.trim()
        if (normalizedAlias.isEmpty() || normalizedAlias == oldAlias) return
        val entry = document.findAliasEntry(entryId) ?: return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "set_leaf_aliases", targetPath = path,
            aliases = entry.aliases.map { if (it == oldAlias) normalizedAlias else it }
        ))
    }

    fun deleteAliasFromEntry(entryId: String, alias: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val entry = document.findAliasEntry(entryId) ?: return
        if (entry.aliases.size <= 1) {
            uiState = uiState.copy(aliasEditorErrorMessage = "A canonical must keep at least one alias.")
            return
        }
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "set_leaf_aliases", targetPath = path,
            aliases = entry.aliases - alias
        ))
    }

    fun promoteAliasEntryToGroup(entryId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val path = document.canonicalTargetPathForEntry(entryId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation("promote_leaf", targetPath = path))
    }

    fun renameGroupAlias(groupId: String, oldAlias: String, newAlias: String) {
        val trimmedAlias = newAlias.trim()
        val document = uiState.aliasDocumentDraft ?: return
        if (trimmedAlias.isEmpty() || trimmedAlias == oldAlias) return
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "rename_group_alias", targetPath = path,
            oldAlias = oldAlias, newName = trimmedAlias
        ))
    }

    fun addGroupAlias(groupId: String, alias: String) {
        val trimmedAlias = alias.trim()
        val document = uiState.aliasDocumentDraft ?: return
        if (trimmedAlias.isEmpty()) return
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "append_group_alias", targetPath = path, aliases = listOf(trimmedAlias)
        ))
    }

    fun updateGroupAliases(groupId: String, aliases: List<String>) {
        val document = uiState.aliasDocumentDraft ?: return
        val normalizedAliases = aliases.map(String::trim)
        if (normalizedAliases.any(String::isEmpty) || normalizedAliases.distinct().size != normalizedAliases.size) {
            uiState = uiState.copy(aliasEditorErrorMessage = "Group aliases must be non-empty and unique.")
            return
        }
        val path = document.canonicalTargetPathForGroup(groupId) ?: return
        applyCoreAliasHierarchyOperation(AliasHierarchyOperation(
            kind = "set_group_aliases", targetPath = path, aliases = normalizedAliases
        ))
    }

    fun previewAliasEntryMove(entryId: String, targetGroupId: String) {
        val document = uiState.aliasDocumentDraft ?: return
        val gateway = configGateway as? AliasHierarchyGateway ?: run {
            uiState = uiState.copy(aliasEditorErrorMessage = "Alias hierarchy runtime is unavailable.")
            return
        }
        val entry = document.findAliasEntry(entryId) ?: return
        val sourcePath = document.canonicalTargetPathForEntry(entryId) ?: return
        val destinationPath = document.canonicalTargetPathForGroup(targetGroupId) ?: return
        val content = uiState.aliasAdvancedTomlDraft.ifBlank { uiState.selectedFileContent }
        viewModelScope.launch {
            val result = gateway.applyAliasHierarchyOperation(
                content,
                AliasHierarchyOperation(
                    kind = "move_leaf",
                    targetPath = sourcePath,
                    destinationPath = destinationPath
                )
            )
            val replacement = result.replacements.singleOrNull()
            if (!result.ok || replacement == null) {
                val message = result.message.ifBlank { "Alias move preview failed." }
                uiState = uiState.copy(
                    aliasEntryMovePlan = null,
                    aliasEditorErrorMessage = message,
                    statusText = message
                )
                return@launch
            }
            val sourceGroupPath = sourcePath.substringBeforeLast('.', "").split('.').filter(String::isNotEmpty)
            val targetGroupPath = destinationPath.removePrefix("root.").split('.').filter(String::isNotEmpty)
            val plan = AliasEntryMovePlan(
                entryId = entryId,
                aliasKey = entry.aliasKey,
                canonicalLeaf = entry.canonicalLeaf,
                sourceParentGroupId = sourcePath.substringBeforeLast('.', "").ifBlank { null },
                sourceGroupPath = sourceGroupPath,
                targetGroupId = targetGroupId,
                targetGroupPath = targetGroupPath,
                oldCanonical = replacement.oldCanonical,
                newCanonical = replacement.newCanonical
            )
            uiState = uiState.copy(
                aliasEntryMovePlan = plan,
                aliasEditorErrorMessage = "",
                statusText = "move plan -> ${plan.oldCanonical} to ${plan.newCanonical} (not saved)"
            )
        }
    }

    fun discardAliasEntryMovePlan() {
        if (uiState.aliasEntryMovePlan == null) {
            return
        }
        uiState = uiState.copy(
            aliasEntryMovePlan = null,
            aliasEditorErrorMessage = "",
            statusText = "move plan discarded"
        )
    }

    fun confirmAliasEntryMovePlan() {
        val plan = uiState.aliasEntryMovePlan ?: return
        val document = uiState.aliasDocumentDraft ?: return
        val sourcePath = document.canonicalTargetPathForEntry(plan.entryId) ?: return
        val destinationPath = document.canonicalTargetPathForGroup(plan.targetGroupId) ?: return
        applyCoreAliasHierarchyOperation(
            AliasHierarchyOperation(
                kind = "move_leaf",
                targetPath = sourcePath,
                destinationPath = destinationPath
            )
        )
    }

    fun setStatusText(message: String) {
        uiState = uiState.copy(statusText = message)
    }

    fun createAliasTomlFile(fileName: String) {
        val targetFilePath = newAliasTomlPath(fileName)
        if (targetFilePath == null) {
            uiState = uiState.copy(
                statusText = "Alias file name must be a single non-empty file name."
            )
            return
        }
        viewModelScope.launch {
            val listResult = configGateway.listConfigTomlFiles()
            if (!listResult.ok) {
                uiState = uiState.copy(statusText = listResult.message)
                return@launch
            }
            val existingPaths = sequenceOf(
                listResult.aliasFiles,
                listResult.chartFiles,
                listResult.metaFiles,
                listResult.reportFiles
            ).flatten().map { entry -> entry.relativePath }
            if (existingPaths.any { path -> path.equals(targetFilePath, ignoreCase = true) }) {
                uiState = uiState.copy(statusText = "TOML file already exists: $targetFilePath")
                return@launch
            }

            val parent = targetFilePath.substringAfterLast('/').removeSuffix(".toml")
            val initialContent = (configGateway as? AliasHierarchyGateway)
                ?.createAliasHierarchyDocument(parent)
                ?.takeIf { it.ok }
                ?.tomlContent
                ?: run {
                    uiState = uiState.copy(statusText = "Alias hierarchy runtime is unavailable.")
                    return@launch
                }
            val migration = aliasMigrationUseCase?.applyCoreResult(
                configRelativePath = targetFilePath,
                updatedTomlContent = initialContent,
                replacements = emptyList(),
                allowMissingConfig = true
            ) ?: ActivityAliasMigrationOutcome.Invalid("Alias migration runtime is unavailable.")
            if (migration is ActivityAliasMigrationOutcome.Invalid) {
                uiState = uiState.copy(statusText = migration.message)
                return@launch
            }

            val refreshedListResult = configGateway.listConfigTomlFiles()
            if (!refreshedListResult.ok) {
                uiState = uiState.copy(statusText = refreshedListResult.message)
                return@launch
            }
            val updated = uiState.copy(
                aliasFiles = refreshedListResult.aliasFiles,
                chartFiles = refreshedListResult.chartFiles,
                metaFiles = refreshedListResult.metaFiles,
                reportFiles = refreshedListResult.reportFiles
            )
            uiState = readConfigFileIntoState(
                baseState = updated,
                path = targetFilePath,
                statusText = "created alias toml through core migration -> $targetFilePath"
            )
        }
    }

    fun deleteCurrentAliasTomlFile() {
        val targetFilePath = uiState.selectedFilePath
        if (!isAliasConfigFilePath(targetFilePath)) {
            uiState = uiState.copy(statusText = "Select an alias TOML file to delete.")
            return
        }
        viewModelScope.launch {
            val deleteResult = configGateway.deleteConfigTomlFile(targetFilePath)
            if (!deleteResult.ok) {
                uiState = uiState.copy(statusText = deleteResult.message)
                return@launch
            }
            reloadRuntimeAfterAliasConfigChange()?.let { message ->
                uiState = uiState.copy(statusText = message)
                return@launch
            }
            refreshConfigFiles(showStatus = false)
            uiState = uiState.copy(statusText = "deleted alias toml -> $targetFilePath")
        }
    }

    private suspend fun reloadRuntimeAfterAliasConfigChange(): String? {
        val reloadResult = (configGateway as? RuntimeInitializer)?.initializeRuntime() ?: return null
        return if (reloadResult.initialized) {
            null
        } else {
            "Alias TOML was saved but runtime reload failed."
        }
    }

    fun saveCurrentFile() {
        val selectedFile = uiState.selectedFilePath
        if (selectedFile.isEmpty()) {
            uiState = uiState.copy(
                statusText = "No TOML file selected.",
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
            )
            return
        }
        uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
        viewModelScope.launch {
            if (isAliasConfigFilePath(selectedFile)) {
                saveAliasFile(selectedFile)
            } else {
                savePlainTomlFile(selectedFile)
            }
        }
    }

    fun discardUnsavedDraft() {
        if (isAliasConfigFilePath(uiState.selectedFilePath)) {
            val selectedFile = uiState.selectedFilePath
            val baseline = uiState.aliasBaselineDocument
            uiState = uiState.copy(
                aliasEditorMode = AliasEditorMode.STRUCTURED,
                aliasDocumentDraft = baseline,
                aliasAdvancedTomlDraft = uiState.selectedFileContent,
                aliasStructuredDraftsByFile = uiState.aliasStructuredDraftsByFile - selectedFile,
                aliasAdvancedDraftsByFile = uiState.aliasAdvancedDraftsByFile - selectedFile,
                aliasEditorModeByFile = uiState.aliasEditorModeByFile - selectedFile,
                aliasEditorErrorMessage = if (baseline == null) {
                    "Alias hierarchy is unavailable for this file."
                } else {
                    ""
                }
            )
            return
        }
        val selectedFile = uiState.selectedFilePath
        if (selectedFile.isBlank() || uiState.editableContent == uiState.selectedFileContent) {
            return
        }
        uiState = uiState.copy(
            editableContent = uiState.selectedFileContent,
            plainTomlDraftsByFile = uiState.plainTomlDraftsByFile - selectedFile
        )
    }

    private suspend fun readConfigFileIntoState(
        baseState: ConfigUiState,
        path: String,
        statusText: String
    ): ConfigUiState {
        val readResult = configGateway.readConfigTomlFile(path)
        if (!readResult.ok) {
            return baseState.copy(statusText = readResult.message)
        }
        val aliasParentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            aliasFiles = baseState.aliasFiles,
            selectedFilePath = readResult.filePath,
            selectedFileContent = readResult.content
        )
        val hierarchyResult = if (isAliasConfigFilePath(readResult.filePath)) {
            val gateway = configGateway as? AliasHierarchyGateway
            if (gateway == null) {
                AliasHierarchyDescribeResult(
                    ok = false,
                    message = "Alias hierarchy runtime is unavailable."
                )
            } else {
                gateway.describeAliasHierarchy(readResult.content)
            }
        } else {
            null
        }
        return applyLoadedConfigFile(
            state = baseState,
            filePath = readResult.filePath,
            content = readResult.content,
            aliasParentOptions = aliasParentOptions,
            statusText = statusText,
            coreDocument = hierarchyResult?.hierarchy?.toActivityAliasDocument(),
            coreErrorMessage = hierarchyResult?.takeIf { !it.ok }?.message.orEmpty()
        )
    }

    private suspend fun savePlainTomlFile(selectedFile: String) {
        val saveResult = configGateway.saveConfigTomlFile(
            relativePath = selectedFile,
            content = uiState.editableContent
        )
        uiState = if (saveResult.ok) {
            uiState.copy(
                selectedFileContent = uiState.editableContent,
                plainTomlDraftsByFile = uiState.plainTomlDraftsByFile - selectedFile,
                autoSaveStatus = ConfigAutoSaveStatus.SAVED,
                statusText = "save toml -> ${saveResult.filePath}"
            )
        } else {
            uiState.copy(
                statusText = saveResult.message,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
            )
        }
    }

    private suspend fun saveAliasFile(selectedFile: String) {
        uiState = when (uiState.aliasEditorMode) {
            AliasEditorMode.STRUCTURED -> saveStructuredAliasFile(selectedFile)
            AliasEditorMode.ADVANCED -> saveAdvancedAliasFile(selectedFile)
        }
    }

    private suspend fun saveStructuredAliasFile(selectedFile: String): ConfigUiState {
        val document = uiState.aliasDocumentDraft
            ?: return uiState.copy(
                statusText = "Alias editor is unavailable for this file.",
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
            )
        // Structured operations are already persisted by
        // applyCoreAliasHierarchyOperation through the migration service.
        // This button only clears presentation drafts; it must never write
        // alias TOML directly from Android.
        return uiState.copy(
            aliasEditorMode = AliasEditorMode.STRUCTURED,
            aliasStructuredDraftsByFile = uiState.aliasStructuredDraftsByFile - selectedFile,
            aliasAdvancedDraftsByFile = uiState.aliasAdvancedDraftsByFile - selectedFile,
            aliasEditorModeByFile = uiState.aliasEditorModeByFile + (selectedFile to AliasEditorMode.STRUCTURED),
            aliasEditorErrorMessage = "",
            autoSaveStatus = ConfigAutoSaveStatus.SAVED,
            statusText = "alias TOML already persisted by core"
        )
    }

    /** Import path for alias TOML; persistence still goes through Core + migration. */
    suspend fun applyImportedAliasToml(relativePath: String, updatedTomlContent: String): String? {
        if (!isAliasConfigFilePath(relativePath)) return "Not an alias TOML path: $relativePath"
        val gateway = configGateway as? AliasHierarchyGateway
            ?: return "Alias hierarchy runtime is unavailable."
        val original = configGateway.readConfigTomlFile(relativePath)
        val result = gateway.rewriteAliasHierarchyDocument(
            originalTomlContent = if (original.ok) original.content else updatedTomlContent,
            updatedTomlContent = updatedTomlContent
        )
        if (!result.ok) return result.message
        val duplicateMessage = validateAliasKeyUniqueness(
            configGateway = configGateway,
            aliasFiles = uiState.aliasFiles,
            currentFilePath = relativePath,
            currentTomlContent = result.updatedTomlContent
        )
        if (duplicateMessage != null) return duplicateMessage
        val migration = aliasMigrationUseCase?.applyCoreResult(
            configRelativePath = relativePath,
            updatedTomlContent = result.updatedTomlContent,
            replacements = result.replacements,
            aliasReplacements = result.aliasReplacements,
            allowMissingConfig = !original.ok
        ) ?: return "Alias migration runtime is unavailable."
        return (migration as? ActivityAliasMigrationOutcome.Invalid)?.message
    }

    private suspend fun saveAdvancedAliasFile(selectedFile: String): ConfigUiState {
        val gateway = configGateway as? AliasHierarchyGateway
            ?: return uiState.copy(
                aliasEditorErrorMessage = "Alias hierarchy runtime is unavailable.",
                autoSaveStatus = ConfigAutoSaveStatus.FAILED,
                statusText = "Alias hierarchy runtime is unavailable."
            )
        val result = gateway.rewriteAliasHierarchyDocument(
            originalTomlContent = uiState.selectedFileContent,
            updatedTomlContent = uiState.aliasAdvancedTomlDraft
        )
        val document = result.hierarchy?.toActivityAliasDocument()
            ?: return uiState.copy(
                aliasEditorErrorMessage = result.message,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED,
                statusText = result.message
            )
        if (!result.ok) {
            return uiState.copy(
                aliasEditorErrorMessage = result.message,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED,
                statusText = result.message
            )
        }
        val duplicateMessage = validateAliasKeyUniqueness(
            configGateway = configGateway,
            aliasFiles = uiState.aliasFiles,
            currentFilePath = selectedFile,
            currentTomlContent = result.updatedTomlContent
        )
        if (duplicateMessage != null) {
            return uiState.copy(
                aliasEditorErrorMessage = duplicateMessage,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED,
                statusText = duplicateMessage
            )
        }
        uiState = uiState.copy(autoSaveStatus = ConfigAutoSaveStatus.SAVING)
        val migration = aliasMigrationUseCase?.applyCoreResult(
            configRelativePath = selectedFile,
            updatedTomlContent = result.updatedTomlContent,
            replacements = result.replacements,
            aliasReplacements = result.aliasReplacements
        ) ?: ActivityAliasMigrationOutcome.Invalid("Alias migration runtime is unavailable.")
        if (migration is ActivityAliasMigrationOutcome.Invalid) {
            return uiState.copy(
                statusText = migration.message,
                aliasEditorErrorMessage = migration.message,
                autoSaveStatus = ConfigAutoSaveStatus.FAILED
            )
        }
        val migratedToml = (migration as ActivityAliasMigrationOutcome.Applied).renderedToml
        if (result.aliasReplacements.isNotEmpty()) {
            val replacements = result.aliasReplacements.associate { it.oldAlias to it.newAlias }
            quickActivitiesPreferenceGateway.setQuickActivities(
                quickActivitiesPreferenceGateway.getQuickActivities().map { value ->
                    replacements[value] ?: value
                }
            )
        }
        val aliasParentOptions = resolveAliasParentOptions(
            configGateway = configGateway,
            aliasFiles = uiState.aliasFiles,
            selectedFilePath = selectedFile,
            selectedFileContent = migratedToml
        )
        return uiState.copy(
            selectedFileContent = migratedToml,
            aliasDocumentDraft = document,
            aliasParentOptions = aliasParentOptions,
            aliasAdvancedTomlDraft = migratedToml,
            aliasStructuredDraftsByFile = uiState.aliasStructuredDraftsByFile - selectedFile,
            aliasAdvancedDraftsByFile = uiState.aliasAdvancedDraftsByFile - selectedFile,
            aliasEditorModeByFile = uiState.aliasEditorModeByFile + (selectedFile to AliasEditorMode.ADVANCED),
            aliasEditorErrorMessage = "",
            autoSaveStatus = ConfigAutoSaveStatus.SAVED,
            txtReloadRequestVersion = uiState.txtReloadRequestVersion + 1,
            statusText = "save alias TOML through core migration"
        )
    }

    private fun cacheStructuredDraft(
        filePath: String,
        document: AliasTomlDocument,
        tomlContent: String
    ): Map<String, AliasTomlDocument> {
        if (filePath.isBlank()) {
            return uiState.aliasStructuredDraftsByFile
        }
        val nextDrafts = uiState.aliasStructuredDraftsByFile.toMutableMap()
        if (tomlContent == uiState.selectedFileContent) {
            nextDrafts.remove(filePath)
        } else {
            nextDrafts[filePath] = document
        }
        return nextDrafts
    }

    private fun cacheAliasMode(filePath: String, mode: AliasEditorMode): Map<String, AliasEditorMode> {
        if (filePath.isBlank()) {
            return uiState.aliasEditorModeByFile
        }
        return uiState.aliasEditorModeByFile + (filePath to mode)
    }

    private fun cacheAliasAdvancedMode(state: ConfigUiState): ConfigUiState {
        val selectedFile = state.selectedFilePath
        if (selectedFile.isBlank()) {
            return state
        }
        val nextAdvancedDrafts = state.aliasAdvancedDraftsByFile.toMutableMap()
        if (state.aliasAdvancedTomlDraft == state.selectedFileContent) {
            nextAdvancedDrafts.remove(selectedFile)
        } else {
            nextAdvancedDrafts[selectedFile] = state.aliasAdvancedTomlDraft
        }
        return state.copy(
            aliasAdvancedDraftsByFile = nextAdvancedDrafts,
            aliasEditorModeByFile = state.aliasEditorModeByFile + (selectedFile to AliasEditorMode.ADVANCED)
        )
    }

    private fun cacheAliasStructuredMode(state: ConfigUiState): ConfigUiState {
        val selectedFile = state.selectedFilePath
        val document = state.aliasDocumentDraft
        if (selectedFile.isBlank() || document == null) {
            return state
        }
        val nextStructuredDrafts = state.aliasStructuredDraftsByFile.toMutableMap()
        if (state.aliasAdvancedTomlDraft == state.selectedFileContent) {
            nextStructuredDrafts.remove(selectedFile)
        } else {
            nextStructuredDrafts[selectedFile] = document
        }
        return state.copy(
            aliasStructuredDraftsByFile = nextStructuredDrafts,
            aliasEditorModeByFile = state.aliasEditorModeByFile + (selectedFile to AliasEditorMode.STRUCTURED)
        )
    }
}

private fun logInfo(tag: String, message: String) {
    runCatching { Log.i(tag, message) }
}

internal class ConfigViewModelFactory(
    private val configGateway: ConfigGateway,
    private val txtStorageGateway: TxtStorageGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway
) : ViewModelProvider.Factory {
    override fun <T : ViewModel> create(modelClass: Class<T>): T {
        if (modelClass.isAssignableFrom(ConfigViewModel::class.java)) {
            @Suppress("UNCHECKED_CAST")
            return ConfigViewModel(
                configGateway = configGateway,
                txtStorageGateway = txtStorageGateway,
                quickActivitiesPreferenceGateway = quickActivitiesPreferenceGateway
            ) as T
        }
        throw IllegalArgumentException("Unknown ViewModel class: ${modelClass.name}")
    }
}
