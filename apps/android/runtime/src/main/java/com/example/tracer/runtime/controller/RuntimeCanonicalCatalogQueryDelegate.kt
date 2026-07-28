package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext

internal class RuntimeCanonicalCatalogQueryDelegate(
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage
) {
    private companion object {
        const val SYSTEM_ALIAS_CONFIG_PATH = "aliases/_system.toml"
    }

    suspend fun listCanonicalCatalog(): CanonicalCatalogResult =
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
                        entry.relativePath.startsWith("aliases/") &&
                            entry.relativePath != SYSTEM_ALIAS_CONFIG_PATH
                    }
                    .sortedBy { it.relativePath }
                if (aliasFiles.isEmpty()) {
                    return@withContext CanonicalCatalogResult(
                        ok = false,
                        roots = emptyList(),
                        entries = emptyList(),
                        message = "Canonical catalog query failed: no alias files."
                    )
                }

                val documents = buildList {
                    for (entry in aliasFiles) {
                        val readResult = storage.readTomlFile(entry.relativePath)
                        if (!readResult.ok) {
                            return@withContext CanonicalCatalogResult(
                                ok = false,
                                roots = emptyList(),
                                entries = emptyList(),
                                message = "Canonical catalog query failed: ${readResult.message}"
                            )
                        }

                        val parseResult = RuntimeCanonicalCatalogParser.parse(readResult.content)
                        val document = parseResult.document
                            ?: return@withContext CanonicalCatalogResult(
                                ok = false,
                                roots = emptyList(),
                                entries = emptyList(),
                                message = "Canonical catalog query failed for ${entry.displayName}: ${parseResult.errorMessage}"
                            )
                        add(readResult.filePath to document)
                    }
                }

                RuntimeCanonicalCatalogBuilder.build(documents)
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
