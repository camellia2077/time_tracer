@file:Suppress("LongParameterList")

package com.example.tracer

/**
 * Coordinates the two-phase activity hierarchy edit:
 * Core first produces the validated TOML and replacement plan, then the host
 * migration commits TOML, TXT and the rebuilt database as one runtime change.
 */
internal class ActivityHierarchyEditCoordinator(
    private val hierarchyGateway: ActivityHierarchyGateway,
    private val migrationUseCase: ActivityHierarchyMigrationUseCase
) {
    suspend fun apply(request: ActivityHierarchyEditRequest): ActivityHierarchyEditOutcome {
        val coreResult = hierarchyGateway.applyActivityHierarchyOperation(
            request.tomlContent,
            request.operation
        )
        val document = coreResult.hierarchy?.toActivityHierarchyDocument()
        if (!coreResult.ok || document == null) {
            return ActivityHierarchyEditOutcome.Failed(
                coreResult.message.ifBlank { "Activity hierarchy operation failed." }
            )
        }
        return persist(
            request = request.toMigrationRequest(
                updatedTomlContent = coreResult.updatedTomlContent,
                replacementPlan = coreResult.replacementPlan
            ),
            document = document
        )
    }

    suspend fun rewrite(request: ActivityHierarchyRewriteRequest): ActivityHierarchyEditOutcome {
        val coreResult = hierarchyGateway.rewriteActivityHierarchyDocument(
            originalTomlContent = request.originalTomlContent,
            updatedTomlContent = request.updatedTomlContent
        )
        val document = coreResult.hierarchy?.toActivityHierarchyDocument()
        if (!coreResult.ok || document == null) {
            return ActivityHierarchyEditOutcome.Failed(
                coreResult.message.ifBlank { "Activity hierarchy rewrite failed." }
            )
        }
        return persist(
            request = ActivityHierarchyMigrationRequest(
                configRelativePath = request.configRelativePath,
                updatedTomlContent = coreResult.updatedTomlContent,
                replacementPlan = coreResult.replacementPlan,
                allowMissingConfig = request.allowMissingConfig
            ),
            document = document
        )
    }

    suspend fun persistCoreResult(
        configRelativePath: String,
        updatedTomlContent: String,
        replacementPlan: ActivityNameReplacementPlan,
        document: ActivityHierarchyDocument,
        updatedDocuments: List<ActivityHierarchyDocumentInput> = emptyList(),
        configFileRename: ActivityHierarchyDocumentRename? = null,
        allowMissingConfig: Boolean = false
    ): ActivityHierarchyEditOutcome = persist(
        request = ActivityHierarchyMigrationRequest(
            configRelativePath = configRelativePath,
            updatedTomlContent = updatedTomlContent,
            replacementPlan = replacementPlan,
            updatedDocuments = updatedDocuments,
            configFileRename = configFileRename,
            allowMissingConfig = allowMissingConfig
        ),
        document = document
    )

    suspend fun persistMigration(
        request: ActivityHierarchyMigrationRequest
    ): ActivityHierarchyMigrationOutcome = migrationUseCase.applyCoreResult(
        configRelativePath = request.configRelativePath,
        updatedTomlContent = request.updatedTomlContent,
        replacementPlan = request.replacementPlan,
        updatedDocuments = request.updatedDocuments,
        configFileRename = request.configFileRename,
        allowMissingConfig = request.allowMissingConfig
    )

    private suspend fun persist(
        request: ActivityHierarchyMigrationRequest,
        document: ActivityHierarchyDocument
    ): ActivityHierarchyEditOutcome {
        return when (val migration = migrationUseCase.applyCoreResult(
            configRelativePath = request.configRelativePath,
            updatedTomlContent = request.updatedTomlContent,
            replacementPlan = request.replacementPlan,
            updatedDocuments = request.updatedDocuments,
            configFileRename = request.configFileRename,
            allowMissingConfig = request.allowMissingConfig
        )) {
            is ActivityHierarchyMigrationOutcome.Applied -> ActivityHierarchyEditOutcome.Applied(
                document = document,
                renderedToml = migration.renderedToml,
                migration = migration.result
            )
            is ActivityHierarchyMigrationOutcome.Invalid ->
                ActivityHierarchyEditOutcome.Failed(migration.message)
        }
    }
}

internal data class ActivityHierarchyEditRequest(
    val configRelativePath: String,
    val tomlContent: String,
    val operation: ActivityHierarchyOperation,
    val updatedDocuments: List<ActivityHierarchyDocumentInput> = emptyList(),
    val configFileRename: ActivityHierarchyDocumentRename? = null,
    val allowMissingConfig: Boolean = false
) {
    fun toMigrationRequest(
        updatedTomlContent: String,
        replacementPlan: ActivityNameReplacementPlan
    ): ActivityHierarchyMigrationRequest = ActivityHierarchyMigrationRequest(
        configRelativePath = configRelativePath,
        updatedTomlContent = updatedTomlContent,
        replacementPlan = replacementPlan,
        updatedDocuments = updatedDocuments,
        configFileRename = configFileRename,
        allowMissingConfig = allowMissingConfig
    )
}

internal data class ActivityHierarchyRewriteRequest(
    val configRelativePath: String,
    val originalTomlContent: String,
    val updatedTomlContent: String,
    val allowMissingConfig: Boolean = false
)

internal sealed interface ActivityHierarchyEditOutcome {
    data class Applied(
        val document: ActivityHierarchyDocument,
        val renderedToml: String,
        val migration: ActivityHierarchyMigrationResult
    ) : ActivityHierarchyEditOutcome

    data class Failed(val message: String) : ActivityHierarchyEditOutcome
}
