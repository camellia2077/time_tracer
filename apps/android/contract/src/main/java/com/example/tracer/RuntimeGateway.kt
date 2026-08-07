package com.example.tracer

interface RuntimeGateway :
    RuntimeInitializer,
    RecordGateway,
    ReportGateway,
    QueryGateway,
    TxtStorageGateway,
    ConfigGateway,
    QuickAccessGateway,
    DataFolderSnapshotGateway,
    ActivityHierarchyGateway,
    ActivityHierarchyMigrationGateway,
    TracerExchangeGateway
