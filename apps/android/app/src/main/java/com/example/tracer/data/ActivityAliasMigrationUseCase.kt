package com.example.tracer

/** Coordinates the persisted half of an already-confirmed alias hierarchy change. */
internal class ActivityAliasMigrationUseCase(
    private val gateway: AliasMoveMigrationGateway
) {
    suspend fun applyCoreResult(
        configRelativePath: String,
        updatedTomlContent: String,
        replacements: List<CanonicalActivityNameReplacement>,
        aliasReplacements: List<AliasKeyReplacement> = emptyList(),
        allowMissingConfig: Boolean = false
    ): ActivityAliasMigrationOutcome {
        val result = gateway.applyAliasEntryMoveMigration(
            AliasEntryMoveMigrationRequest(
                configRelativePath = configRelativePath,
                updatedTomlContent = updatedTomlContent,
                replacements = replacements,
                aliasReplacements = aliasReplacements,
                allowMissingConfig = allowMissingConfig
            )
        )
        return if (result.ok) {
            ActivityAliasMigrationOutcome.Applied(
                result.updatedTomlContent.ifBlank { updatedTomlContent },
                result
            )
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
