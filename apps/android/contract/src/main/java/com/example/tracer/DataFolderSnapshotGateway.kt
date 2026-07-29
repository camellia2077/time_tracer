package com.example.tracer

interface DataFolderSnapshotGateway {
    suspend fun replaceDataFolderSnapshot(stagedRootPath: String): DataFolderSnapshotResult
}
