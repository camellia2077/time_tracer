package com.example.tracer

interface RuntimeGateway :
    RuntimeInitializer,
    RecordGateway,
    InsightsGateway,
    QueryGateway,
    TxtStorageGateway,
    ConfigGateway,
    QuickAccessGateway,
    ActivityHierarchyGateway,
    ActivityHierarchyMigrationGateway,
    TracerExchangeGateway
