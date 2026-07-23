package com.example.tracer

import org.tomlj.Toml
import org.tomlj.TomlTable

internal object ActivityAliasTomlParser {
    const val GROUP_ALIASES_KEY = "group_aliases"

    fun parse(rawToml: String): AliasTomlParseResult {
        val parsed = Toml.parse(rawToml)
        if (parsed.hasErrors()) {
            return AliasTomlParseResult(
                errorMessage = parsed.errors().joinToString("; ") { error -> error.toString() }
            )
        }
        val parent = parsed.getString("parent")
            ?: return AliasTomlParseResult(errorMessage = "Alias child file must contain a non-empty `parent` string.")
        if (parent.isBlank()) {
            return AliasTomlParseResult(errorMessage = "Alias child file must contain a non-empty `parent` string.")
        }
        val aliasesTable = parsed.getTable("aliases")
            ?: return AliasTomlParseResult(errorMessage = "Alias child file must contain an `aliases` table.")
        return runCatching {
            val document = AliasTomlDocument(parent = parent, nodes = parseNodes(aliasesTable, false))
            ActivityAliasHierarchyValidator.validateParsed(document)
                ?.let { AliasTomlParseResult(errorMessage = it) }
                ?: AliasTomlParseResult(document = document)
        }.getOrElse { error ->
            AliasTomlParseResult(errorMessage = error.message ?: "unknown alias parse error")
        }
    }

    private fun parseNodes(table: TomlTable, isCategoryTable: Boolean): List<AliasTomlNode> = buildList {
        for ((key, node) in table.entrySet()) {
            if (isCategoryTable && key == GROUP_ALIASES_KEY) continue
            when (node) {
                is String -> add(AliasTomlEntry(aliasKey = key, canonicalLeaf = node))
                is TomlTable -> add(
                    AliasTomlGroup(
                        name = key,
                        groupAliases = parseGroupAliases(node, key),
                        nodes = parseNodes(node, true)
                    )
                )
                else -> throw IllegalArgumentException("Alias field `$key` must be a string or nested table.")
            }
        }
    }

    private fun parseGroupAliases(table: TomlTable, groupName: String): List<String> {
        val values = table.getArray(GROUP_ALIASES_KEY) ?: return emptyList()
        return buildList {
            for (index in 0 until values.size()) {
                add(values.getString(index) ?: throw IllegalArgumentException(
                    "`$GROUP_ALIASES_KEY` in group `$groupName` must contain only strings."
                ))
            }
        }
    }
}
