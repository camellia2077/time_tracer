package com.example.tracer

interface RuntimeGateway :
    RuntimeInitializer,
    RecordGateway,
    ReportGateway,
    QueryGateway,
    TxtStorageGateway,
    ConfigGateway,
    AliasHierarchyGateway,
    AliasMoveMigrationGateway,
    FileCryptoGateway,
    TracerExchangeGateway
