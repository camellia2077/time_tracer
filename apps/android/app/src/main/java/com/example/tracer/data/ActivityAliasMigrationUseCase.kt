package com.example.tracer

/** Coordinates the persisted half of an already-confirmed alias hierarchy change. */
internal class ActivityAliasMigrationUseCase(
    private val gateway: AliasMoveMigrationGateway
) {
    suspend fun apply(
        configRelativePath: String,
        updatedDocument: ActivityAliasDocument,
        replacements: List<CanonicalActivityNameReplacement>
    ): ActivityAliasMigrationOutcome {
        val validationMessage = ActivityAliasHierarchyValidator.validateForSave(updatedDocument)
        if (validationMessage != null) return ActivityAliasMigrationOutcome.Invalid(validationMessage)

        val renderedToml = ActivityAliasTomlSerializer.serialize(updatedDocument)
        val result = gateway.applyAliasEntryMoveMigration(
            AliasEntryMoveMigrationRequest(
                configRelativePath = configRelativePath,
                updatedTomlContent = renderedToml,
                replacements = replacements
            )
        )
        return if (result.ok) {
            ActivityAliasMigrationOutcome.Applied(renderedToml, result)
        } else {
            ActivityAliasMigrationOutcome.Invalid(result.message)
        }
    }
}

internal sealed interface ActivityAliasMigrationOutcome {
    data class Applied(
        val renderedToml: String,
        val result: AliasEntryMoveMigrationResult
    ) : ActivityAliasMigrationOutcome

    data class Invalid(val message: String) : ActivityAliasMigrationOutcome
}
