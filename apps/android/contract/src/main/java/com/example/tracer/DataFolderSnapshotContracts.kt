package com.example.tracer

data class DataFolderSnapshotResult(
    val ok: Boolean,
    val message: String,
    val txtFileCount: Int = 0,
    val tomlFileCount: Int = 0
)
