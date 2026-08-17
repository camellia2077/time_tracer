@file:Suppress("LongParameterList")

package com.example.tracer

/** Coordinates the persisted half of an already-confirmed activity hierarchy change. */
internal class ActivityHierarchyMigrationUseCase(
    private val gateway: ActivityHierarchyMigrationGateway,
    private val quickActivitiesPreferenceGateway: QuickActivitiesPreferenceGateway? = null
) {
    suspend fun applyCoreResult(
        configRelativePath: String,
        updatedTomlContent: String,
        replacementPlan: ActivityNameReplacementPlan = ActivityNameReplacementPlan(),
        updatedDocuments: List<ActivityHierarchyDocumentInput> = emptyList(),
        configFileRename: ActivityHierarchyDocumentRename? = null,
        allowMissingConfig: Boolean = false
    ): ActivityHierarchyMigrationOutcome {
        val result = gateway.applyActivityHierarchyMigration(
            ActivityHierarchyMigrationRequest(
                configRelativePath = configRelativePath,
                updatedTomlContent = updatedTomlContent,
                replacementPlan = replacementPlan,
                updatedDocuments = updatedDocuments,
                configFileRename = configFileRename,
                allowMissingConfig = allowMissingConfig
            )
        )
        if (!result.ok) {
            return ActivityHierarchyMigrationOutcome.Invalid(result.message)
        }

        val preferenceError = quickActivitiesPreferenceGateway
            ?.let { preferences ->
                val replacements = replacementPlan.aliases.associate { it.oldAlias to it.newAlias }
                if (replacements.isEmpty()) {
                    null
                } else {
                    runCatching {
                        preferences.setQuickActivities(
                            preferences.getQuickActivities().map { value ->
                                replacements[value] ?: value
                            }
                        )
                    }.exceptionOrNull()
                }
            }
        if (preferenceError != null) {
            return ActivityHierarchyMigrationOutcome.Invalid(
                preferenceError.message ?: "Quick Access alias migration failed."
            )
        }

        return ActivityHierarchyMigrationOutcome.Applied(
                    result.updatedTomlContent.ifBlank { updatedTomlContent },
                    result
                )
    }

}

internal sealed interface ActivityHierarchyMigrationOutcome {
    data class Applied(
        val renderedToml: String,
        val result: ActivityHierarchyMigrationResult
    ) : ActivityHierarchyMigrationOutcome

    data class Invalid(val message: String) : ActivityHierarchyMigrationOutcome
}
