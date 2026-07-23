package com.example.tracer

// A single host-side transaction: TOML hierarchy, canonical TXT references,
// and the derived database must change together.
interface AliasMoveMigrationGateway {
    suspend fun applyAliasEntryMoveMigration(
        request: AliasEntryMoveMigrationRequest
    ): AliasEntryMoveMigrationResult
}
