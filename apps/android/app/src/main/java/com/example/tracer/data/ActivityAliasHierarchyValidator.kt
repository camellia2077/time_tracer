package com.example.tracer

internal object ActivityAliasHierarchyValidator {
    fun validateForSave(document: AliasTomlDocument): String? {
        if (document.parent.isBlank()) return "`parent` must not be empty."
        if (document.nodes.isEmpty()) return "Alias document must contain at least one alias group or alias entry."
        return validateNodes(document.nodes, listOf("aliases"))
    }

    fun validateParsed(document: AliasTomlDocument): String? =
        if (document.parent.isBlank()) "Alias child file must contain a non-empty `parent` string."
        else validateNodes(document.nodes, listOf("aliases"))

    fun collectAliasKeys(document: AliasTomlDocument): List<String> = buildList {
        collectAliasKeys(document.nodes, this)
    }

    private fun validateNodes(nodes: List<AliasTomlNode>, path: List<String>): String? {
        val groupNames = linkedSetOf<String>()
        val aliasKeys = linkedSetOf<String>()
        for (node in nodes) when (node) {
            is AliasTomlGroup -> {
                if (node.name.isBlank()) return "Alias group `${path.joinToString(".")}` must not contain an empty group name."
                if (!groupNames.add(node.name)) return "Alias group `${path.joinToString(".")}` contains duplicate child group `${node.name}`."
                if (aliasKeys.contains(node.name)) return "Alias group `${path.joinToString(".")}` cannot reuse `${node.name}` as both alias key and group name."
                if (node.name == ActivityAliasTomlParser.GROUP_ALIASES_KEY) return "Alias group name `${ActivityAliasTomlParser.GROUP_ALIASES_KEY}` is reserved."
                for (groupAlias in node.groupAliases) {
                    if (groupAlias.isBlank()) return "Recordable aliases in group `${node.name}` must not be empty."
                    if (!aliasKeys.add(groupAlias)) return "Alias group `${path.joinToString(".")}` contains duplicate alias key `$groupAlias`."
                }
                validateNodes(node.nodes, path + node.name)?.let { return it }
            }
            is AliasTomlEntry -> {
                if (node.aliasKey.isBlank()) return "Alias group `${path.joinToString(".")}` must not contain an empty alias key."
                if (node.canonicalLeaf.isBlank()) return "Alias key `${node.aliasKey}` must map to a non-empty canonical leaf."
                if (!aliasKeys.add(node.aliasKey)) return "Alias group `${path.joinToString(".")}` contains duplicate alias key `${node.aliasKey}`."
                if (groupNames.contains(node.aliasKey)) return "Alias group `${path.joinToString(".")}` cannot reuse `${node.aliasKey}` as both alias key and group name."
            }
        }
        return null
    }

    private fun collectAliasKeys(nodes: List<AliasTomlNode>, sink: MutableList<String>) {
        nodes.forEach { node -> when (node) {
            is AliasTomlEntry -> sink += node.aliasKey
            is AliasTomlGroup -> { sink += node.groupAliases; collectAliasKeys(node.nodes, sink) }
        } }
    }
}
