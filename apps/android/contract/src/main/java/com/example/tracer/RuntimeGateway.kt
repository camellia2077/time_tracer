package com.example.tracer

interface RuntimeGateway :
    RuntimeInitializer,
    RecordGateway,
    ReportGateway,
    QueryGateway,
    TxtStorageGateway,
    ConfigGateway,
    AliasMoveMigrationGateway,
    FileCryptoGateway,
    TracerExchangeGateway
