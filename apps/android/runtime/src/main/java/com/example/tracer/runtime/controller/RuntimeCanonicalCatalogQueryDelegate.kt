package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeCanonicalCatalogQueryDelegate(
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage,
    private val searchActivityHierarchy: suspend (String, String) -> ActivityHierarchyDescribeResult
) {
    suspend fun listCanonicalCatalog(searchQuery: String): CanonicalCatalogResult =
        withContext(Dispatchers.IO) {
            try {
                val storage = ensureConfigTomlStorage()
                val listResult = storage.listTomlFiles()
                if (!listResult.ok) {
                    return@withContext CanonicalCatalogResult(
                        ok = false,
                        roots = emptyList(),
                        entries = emptyList(),
                        message = listResult.message
                    )
                }

                val aliasFiles = listResult.aliasFiles
                    .filter { entry ->
                        entry.relativePath.startsWith("user/activity_hierarchy/") &&
                            entry.relativePath.endsWith(".toml", ignoreCase = true)
                    }
                    .sortedBy { it.relativePath }
                if (aliasFiles.isEmpty()) {
                    return@withContext CanonicalCatalogResult(
                        ok = false,
                        roots = emptyList(),
                        entries = emptyList(),
                        message = "Canonical catalog query failed: no canonical files."
                    )
                }

                val documents = buildList<Pair<String, ActivityHierarchySnapshot>> {
                    for (entry in aliasFiles) {
                        val readResult = storage.readTomlFile(entry.relativePath)
                        if (!readResult.ok) {
                            return@withContext canonicalCatalogFailure(readResult.message)
                        }
                        val searchResult = searchActivityHierarchy(readResult.content, searchQuery)
                        if (!searchResult.ok) {
                            return@withContext canonicalCatalogFailure(
                                "${entry.displayName}: ${searchResult.message}"
                            )
                        }
                        val hierarchy = searchResult.hierarchy
                            ?: return@withContext canonicalCatalogFailure(
                                "${entry.displayName}: ${searchResult.message}"
                            )
                        add(readResult.filePath to hierarchy)
                    }
                }
                RuntimeCanonicalCatalogBuilder.buildFromSnapshots(documents)
            } catch (error: Exception) {
                CanonicalCatalogResult(
                    ok = false,
                    roots = emptyList(),
                    entries = emptyList(),
                    message = formatNativeFailure("list canonical catalog failed", error)
                )
            }
        }
}

private fun canonicalCatalogFailure(message: String): CanonicalCatalogResult =
    CanonicalCatalogResult(
        ok = false,
        roots = emptyList(),
        entries = emptyList(),
        message = "Canonical catalog query failed: $message"
    )
