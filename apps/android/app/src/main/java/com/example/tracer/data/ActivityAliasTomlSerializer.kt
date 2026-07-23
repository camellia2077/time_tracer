package com.example.tracer

internal object ActivityAliasTomlSerializer {
    private val bareTomlKeyPattern = Regex("^[A-Za-z0-9_-]+$")

    fun serialize(document: AliasTomlDocument): String = buildString {
        append("parent = ").append(formatTomlString(document.parent)).append("\n\n")
        append(buildSection(listOf("aliases"), document.nodes, emptyList())).append("\n")
    }

    private fun buildSection(path: List<String>, nodes: List<AliasTomlNode>, groupAliases: List<String>): String {
        val bodyLines = buildList {
            if (groupAliases.isNotEmpty()) {
                add("${ActivityAliasTomlParser.GROUP_ALIASES_KEY} = [${groupAliases.joinToString(", ") { formatTomlString(it) }}]")
            }
            nodes.filterIsInstance<AliasTomlEntry>().forEach { entry ->
                add("${formatTomlString(entry.aliasKey)} = ${formatTomlString(entry.canonicalLeaf)}")
            }
        }
        val header = "[${path.joinToString(".") { formatTableSegment(it) }}]"
        return buildList {
            add(if (bodyLines.isEmpty()) header else "$header\n${bodyLines.joinToString("\n")}")
            nodes.filterIsInstance<AliasTomlGroup>().forEach { group ->
                add(buildSection(path + group.name, group.nodes, group.groupAliases))
            }
        }.joinToString("\n\n")
    }

    private fun formatTableSegment(segment: String): String =
        if (bareTomlKeyPattern.matches(segment)) segment else formatTomlString(segment)

    private fun formatTomlString(value: String): String = buildString {
        append('"')
        value.forEach { character ->
            append(
                when (character) {
                    '\\' -> "\\\\"
                    '"' -> "\\\""
                    '\n' -> "\\n"
                    '\r' -> "\\r"
                    '\t' -> "\\t"
                    else -> character.toString()
                }
            )
        }
        append('"')
    }
}
