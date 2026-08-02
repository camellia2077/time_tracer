package com.example.tracer

import android.content.Context
import android.util.Log

private const val ACTIVITY_HIERARCHY_RECORD_TAG = "TimeTracerRecord"

/** Registers an activity token before the record reaches Core's ingest path. */
internal class RuntimeActivityHierarchyAutoRegistrar(
    context: Context,
    private val ensureConfigTomlStorage: () -> ConfigTomlStorage,
    private val catalogQuery: RuntimeCanonicalCatalogQueryDelegate,
    private val loadWakeKeywords: suspend () -> ActivityMappingNamesResult,
    private val hierarchyService: RuntimeActivityHierarchyService
) {
    private val language = context.resources.configuration.locales
        .get(0)?.language.orEmpty()

    suspend fun ensureRegistered(rawActivityName: String): ActivityHierarchyAutoRegistrationResult {
        val activityName = rawActivityName.trim()
        Log.i(
            ACTIVITY_HIERARCHY_RECORD_TAG,
            "hierarchy.register.start rawLength=${rawActivityName.length} activity=$activityName"
        )
        if (activityName.isEmpty()) {
            Log.i(ACTIVITY_HIERARCHY_RECORD_TAG, "hierarchy.register.skip reason=empty_activity")
            return ActivityHierarchyAutoRegistrationResult(ok = true)
        }

        // Wake keywords are valid event tokens, but intentionally do not belong
        // to the activity hierarchy. Keep them out of the auto-created default
        // hierarchy document.
        val wakeKeywords = loadWakeKeywords()
        Log.i(
            ACTIVITY_HIERARCHY_RECORD_TAG,
            "hierarchy.register.wake_query ok=${wakeKeywords.ok} count=${wakeKeywords.names.size} " +
                "message=${wakeKeywords.message}"
        )
        if (wakeKeywords.ok && isWakeKeywordActivity(activityName, wakeKeywords.names)) {
            Log.i(
                ACTIVITY_HIERARCHY_RECORD_TAG,
                "hierarchy.register.skip reason=wake_keyword activity=$activityName"
            )
            return ActivityHierarchyAutoRegistrationResult(ok = true)
        }

        val storage = ensureConfigTomlStorage()
        val files = storage.listTomlFiles()
        if (!files.ok) {
            Log.i(
                ACTIVITY_HIERARCHY_RECORD_TAG,
                "hierarchy.register.fail stage=list_files message=${files.message}"
            )
            return ActivityHierarchyAutoRegistrationResult(false, files.message)
        }

        val catalog = catalogQuery.listCanonicalCatalog()
        if (catalog.ok && catalog.entries.any { entry ->
                entry.canonicalLeaf == activityName ||
                    entry.canonicalPath == activityName ||
                    entry.aliases.any { it == activityName }
            }) {
            return ActivityHierarchyAutoRegistrationResult(ok = true)
        }
        if (!catalog.ok && files.aliasFiles.isNotEmpty()) {
            return ActivityHierarchyAutoRegistrationResult(false, catalog.message)
        }

        val relativePath = "user/activity_hierarchy/${defaultFileName()}.toml"
        Log.i(
            ACTIVITY_HIERARCHY_RECORD_TAG,
            "hierarchy.register.default_path path=$relativePath catalogOk=${catalog.ok} " +
                "catalogEntries=${catalog.entries.size}"
        )
        val existing = storage.readTomlFile(relativePath)
        val originalContent = if (existing.ok) {
            existing.content
        } else {
            if (existing.message != "TOML file not found.") {
                return ActivityHierarchyAutoRegistrationResult(false, existing.message)
            }
            "parent = \"${defaultFileName()}\"\n\n[canonical]\n"
        }

        val result = hierarchyService.apply(
            tomlContent = originalContent,
            operation = ActivityHierarchyOperation(
                kind = ActivityHierarchyOperationKind.ADD_LEAF,
                targetPath = "root",
                canonicalKey = activityName,
                aliases = listOf(activityName)
            )
        )
        if (!result.ok) {
            return ActivityHierarchyAutoRegistrationResult(false, result.message)
        }

        val saved = storage.writeTomlFile(relativePath, result.updatedTomlContent)
        Log.i(
            ACTIVITY_HIERARCHY_RECORD_TAG,
            "hierarchy.register.write path=$relativePath ok=${saved.ok} message=${saved.message}"
        )
        return if (saved.ok) {
            ActivityHierarchyAutoRegistrationResult(true)
        } else {
            ActivityHierarchyAutoRegistrationResult(false, saved.message)
        }
    }

    private fun defaultFileName(): String = activityHierarchyTomlFileName(language)
}

internal fun isWakeKeywordActivity(activityName: String, wakeKeywords: Collection<String>): Boolean {
    val normalizedActivityName = activityName.trim()
    return normalizedActivityName.isNotEmpty() && wakeKeywords.any {
        it.trim() == normalizedActivityName
    }
}

internal fun activityHierarchyTomlFileName(language: String): String = when (language.lowercase()) {
    "zh", "zh-cn", "zh-tw", "zh-hk" -> "默认"
    "ja" -> "デフォルト"
    else -> "default"
}

internal data class ActivityHierarchyAutoRegistrationResult(
    val ok: Boolean,
    val message: String = ""
)
