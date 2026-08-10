package com.example.tracer

interface RuntimeGateway :
    RuntimeInitializer,
    RecordGateway,
    InsightsGateway,
    QueryGateway,
    TxtStorageGateway,
    ConfigGateway,
    QuickAccessGateway,
    DataFolderSnapshotGateway,
    ActivityHierarchyGateway,
    ActivityHierarchyMigrationGateway,
    TracerExchangeGateway
