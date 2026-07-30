package com.example.tracer

// A single host-side transaction: TOML hierarchy, the Core-produced activity
// name replacement plan, and the derived database must change together.
interface ActivityHierarchyMigrationGateway {
    suspend fun applyActivityHierarchyMigration(
        request: ActivityHierarchyMigrationRequest
    ): ActivityHierarchyMigrationResult
}
