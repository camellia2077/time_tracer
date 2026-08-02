package com.example.tracer

import java.io.File
import java.nio.file.Files
import kotlinx.coroutines.runBlocking
import org.json.JSONArray
import org.json.JSONObject
import org.junit.Assert.assertEquals
import org.junit.Assert.assertFalse
import org.junit.Assert.assertTrue
import org.junit.Test

class RuntimeActivityHierarchyMigrationServiceTest {
    @Test
    fun cross_toml_leaf_and_group_moves_rewrite_txt_and_rebuild_database() = runBlocking {
        val scenarios = listOf(
            MoveScenario(
                name = "leaf to other TOML root",
                sourceToml = "\"go\" = [\"围棋\"]",
                destinationToml = "\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "\"eat\" = [\"吃饭\"]\n\"go\" = [\"围棋\"]",
                oldCanonical = "exercise_go",
                newCanonical = "meal_go"
            ),
            MoveScenario(
                name = "leaf to other TOML group",
                sourceToml = "\"go\" = [\"围棋\"]",
                destinationToml = "[canonical.breakfast]\n\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "[canonical.breakfast]\n\"eat\" = [\"吃饭\"]\n\"go\" = [\"围棋\"]",
                oldCanonical = "exercise_go",
                newCanonical = "meal_breakfast_go"
            ),
            MoveScenario(
                name = "leaf to other TOML group subtree",
                sourceToml = "\"go\" = [\"围棋\"]",
                destinationToml = "[canonical.breakfast.morning]\n\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "[canonical.breakfast.morning]\n\"eat\" = [\"吃饭\"]\n\"go\" = [\"围棋\"]",
                oldCanonical = "exercise_go",
                newCanonical = "meal_breakfast_morning_go"
            ),
            MoveScenario(
                name = "group subtree to other TOML root",
                sourceToml = "[canonical.cardio]\n\"swimming\" = [\"游泳\"]\n[canonical.cardio.running]\n\"track-running\" = [\"操场跑\"]",
                destinationToml = "\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "\"eat\" = [\"吃饭\"]\n[canonical.cardio]\n\"swimming\" = [\"游泳\"]\n[canonical.cardio.running]\n\"track-running\" = [\"操场跑\"]",
                oldCanonical = "exercise_cardio",
                newCanonical = "meal_cardio",
                extraReplacements = listOf(
                    CanonicalActivityNameReplacement("exercise_cardio_swimming", "meal_cardio_swimming"),
                    CanonicalActivityNameReplacement("exercise_cardio_running_track-running", "meal_cardio_running_track-running")
                )
            ),
            MoveScenario(
                name = "group subtree to other TOML group subtree",
                sourceToml = "[canonical.cardio]\n\"swimming\" = [\"游泳\"]\n[canonical.cardio.running]\n\"track-running\" = [\"操场跑\"]",
                destinationToml = "[canonical.fitness.deep]\n\"stretch\" = [\"拉伸\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "[canonical.fitness.deep]\n\"stretch\" = [\"拉伸\"]\n[canonical.fitness.deep.cardio]\n\"swimming\" = [\"游泳\"]\n[canonical.fitness.deep.cardio.running]\n\"track-running\" = [\"操场跑\"]",
                oldCanonical = "exercise_cardio",
                newCanonical = "meal_fitness_deep_cardio",
                extraReplacements = listOf(
                    CanonicalActivityNameReplacement("exercise_cardio_swimming", "meal_fitness_deep_cardio_swimming"),
                    CanonicalActivityNameReplacement("exercise_cardio_running_track-running", "meal_fitness_deep_cardio_running_track-running")
                )
            )
        )

        scenarios.forEach { scenario ->
            val fixture = MigrationFixture(scenario)
            try {
                val result = fixture.service.apply(fixture.request)

                assertTrue(scenario.name, result.ok)
                assertEquals(scenario.updatedSourceToml, fixture.config.read("user/activity_hierarchy/exercise.toml"))
                assertEquals(scenario.updatedDestinationToml, fixture.config.read("user/activity_hierarchy/meal.toml"))
                assertEquals(
                    "2026-01-01|${scenario.newCanonical}|remark",
                    fixture.text.files["2026/2026-01.txt"]
                )
                assertEquals("candidate-db", fixture.db.readText())
            } finally {
                fixture.root.deleteRecursively()
            }
        }
    }

    @Test
    fun cross_toml_move_failure_restores_toml_txt_and_database() = runBlocking {
        val fixture = MigrationFixture(
            MoveScenario(
                name = "failed move",
                sourceToml = "\"go\" = [\"围棋\"]",
                destinationToml = "\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "\"eat\" = [\"吃饭\"]\n\"go\" = [\"围棋\"]",
                oldCanonical = "exercise_go",
                newCanonical = "meal_go"
            ),
            failCandidateIngest = true
        )
        try {
            val result = fixture.service.apply(fixture.request)

            assertFalse(result.ok)
            assertEquals(fixture.scenario.sourceToml, fixture.config.read("user/activity_hierarchy/exercise.toml"))
            assertEquals(fixture.scenario.destinationToml, fixture.config.read("user/activity_hierarchy/meal.toml"))
            assertEquals("2026-01-01|exercise_go|remark", fixture.text.files["2026/2026-01.txt"])
            assertEquals("active-db", fixture.db.readText())
        } finally {
            fixture.root.deleteRecursively()
        }
    }

    @Test
    fun category_rename_moves_toml_rewrites_txt_and_rebuilds_database() = runBlocking {
        val fixture = MigrationFixture(
            MoveScenario(
                name = "category rename",
                sourceToml = "parent = \"exercise\"\n\"go\" = [\"围棋\"]",
                destinationToml = "parent = \"meal\"\n\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "parent = \"fitness\"\n\"go\" = [\"围棋\"]",
                updatedDestinationToml = "parent = \"meal\"\n\"eat\" = [\"吃饭\"]",
                oldCanonical = "exercise_go",
                newCanonical = "fitness_go"
            ),
            configRename = ActivityHierarchyDocumentRename(
                oldSourceName = "user/activity_hierarchy/exercise.toml",
                newSourceName = "user/activity_hierarchy/fitness.toml"
            )
        )
        try {
            val result = fixture.service.apply(fixture.request)

            assertTrue(result.ok)
            assertEquals(
                fixture.scenario.updatedSourceToml,
                fixture.config.read("user/activity_hierarchy/fitness.toml")
            )
            assertFalse(File(fixture.config.root, "user/activity_hierarchy/exercise.toml").exists())
            assertEquals(
                "2026-01-01|${fixture.scenario.newCanonical}|remark",
                fixture.text.files["2026/2026-01.txt"]
            )
            assertEquals("candidate-db", fixture.db.readText())
        } finally {
            fixture.root.deleteRecursively()
        }
    }

    @Test
    fun canonical_and_alias_replacements_are_both_applied_to_txt() = runBlocking {
        val fixture = MigrationFixture(
            MoveScenario(
                name = "combined replacement plan",
                sourceToml = "parent = \"exercise\"\n\"go\" = [\"旧名称\"]",
                destinationToml = "parent = \"meal\"\n\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "parent = \"meal\"\n\"go\" = [\"新名称\"]",
                updatedDestinationToml = "parent = \"meal\"\n\"eat\" = [\"吃饭\"]",
                oldCanonical = "exercise_go",
                newCanonical = "meal_go"
            )
        )
        try {
            fixture.text.files["2026/2026-01.txt"] = "2026-01-01|exercise_go|旧名称 remark"
            val result = fixture.service.apply(
                fixture.request.copy(
                    replacementPlan = fixture.request.replacementPlan.copy(
                        aliases = listOf(AliasKeyReplacement("旧名称", "新名称"))
                    )
                )
            )

            assertTrue(result.ok)
            assertEquals("2026-01-01|meal_go|新名称 remark", fixture.text.files["2026/2026-01.txt"])
        } finally {
            fixture.root.deleteRecursively()
        }
    }

    @Test
    fun category_rename_failure_restores_old_toml_and_removes_new_toml() = runBlocking {
        val fixture = MigrationFixture(
            MoveScenario(
                name = "failed category rename",
                sourceToml = "parent = \"exercise\"\n\"go\" = [\"围棋\"]",
                destinationToml = "parent = \"meal\"\n\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "parent = \"fitness\"\n\"go\" = [\"围棋\"]",
                updatedDestinationToml = "parent = \"meal\"\n\"eat\" = [\"吃饭\"]",
                oldCanonical = "exercise_go",
                newCanonical = "fitness_go"
            ),
            failCandidateIngest = true,
            configRename = ActivityHierarchyDocumentRename(
                oldSourceName = "user/activity_hierarchy/exercise.toml",
                newSourceName = "user/activity_hierarchy/fitness.toml"
            )
        )
        try {
            val result = fixture.service.apply(fixture.request)

            assertFalse(result.ok)
            assertEquals(fixture.scenario.sourceToml, fixture.config.read("user/activity_hierarchy/exercise.toml"))
            assertFalse(File(fixture.config.root, "user/activity_hierarchy/fitness.toml").exists())
            assertEquals("2026-01-01|exercise_go|remark", fixture.text.files["2026/2026-01.txt"])
            assertEquals("active-db", fixture.db.readText())
        } finally {
            fixture.root.deleteRecursively()
        }
    }
}

private data class MoveScenario(
    val name: String,
    val sourceToml: String,
    val destinationToml: String,
    val updatedSourceToml: String,
    val updatedDestinationToml: String,
    val oldCanonical: String,
    val newCanonical: String,
    val extraReplacements: List<CanonicalActivityNameReplacement> = emptyList()
)

private class MigrationFixture(
    val scenario: MoveScenario,
    private val failCandidateIngest: Boolean = false,
    private val configRename: ActivityHierarchyDocumentRename? = null
) {
    val root = Files.createTempDirectory("alias-move-migration-").toFile()
    val config = FixtureConfigStorage(File(root, "config")).also {
        it.write("user/activity_hierarchy/exercise.toml", scenario.sourceToml)
        it.write("user/activity_hierarchy/meal.toml", scenario.destinationToml)
    }
    val text = FixtureTextStorage(
        linkedMapOf("2026/2026-01.txt" to "2026-01-01|${scenario.oldCanonical}|remark")
    )
    val db = File(root, "runtime/time_data.sqlite3").also {
        it.parentFile?.mkdirs()
        it.writeText("active-db")
    }
    private val paths = RuntimePaths(
        dbPath = db.absolutePath,
        outputRoot = File(root, "runtime/output").absolutePath,
        configRootPath = config.root.absolutePath,
        configTomlPath = File(config.root, "config.toml").absolutePath,
        inputRootPath = File(root, "input").absolutePath,
        cacheRootPath = File(root, "runtime/cache").absolutePath
    )
    private var initializedPaths = paths

    val request = ActivityHierarchyMigrationRequest(
        configRelativePath = "user/activity_hierarchy/exercise.toml",
        updatedTomlContent = scenario.updatedSourceToml,
        replacementPlan = ActivityNameReplacementPlan(
            canonical = listOf(
                CanonicalActivityNameReplacement(scenario.oldCanonical, scenario.newCanonical)
            ) + scenario.extraReplacements
        ),
        updatedDocuments = listOf(
            ActivityHierarchyDocumentInput("user/activity_hierarchy/exercise.toml", scenario.updatedSourceToml),
            ActivityHierarchyDocumentInput("user/activity_hierarchy/meal.toml", scenario.updatedDestinationToml)
        ),
        configFileRename = configRename
    )

    val service = RuntimeActivityHierarchyMigrationService(
        ensureRuntimePaths = { paths },
        ensureTextStorage = { text },
        ensureConfigTomlStorage = { ConfigTomlStorage(config.root.absolutePath) },
        nativeInit = { candidatePaths ->
            initializedPaths = candidatePaths
            File(candidatePaths.outputRoot).mkdirs()
            File(candidatePaths.cacheRootPath).mkdirs()
            """{"ok":true}"""
        },
        nativeInitPipeline = { candidatePaths ->
            initializedPaths = candidatePaths
            File(candidatePaths.outputRoot).mkdirs()
            File(candidatePaths.cacheRootPath).mkdirs()
            """{"ok":true}"""
        },
        nativeShutdown = { """{"ok":true}""" },
        nativeIngest = { _, _, _ ->
            if (failCandidateIngest) {
                """{"ok":false,"error_message":"candidate ingest failed"}"""
            } else {
                File(initializedPaths.dbPath).parentFile?.mkdirs()
                File(initializedPaths.dbPath).writeText("candidate-db")
                """{"ok":true}"""
            }
        },
        nativeTxt = ::replaceTxt,
        responseCodec = NativeResponseCodec()
    )

    private fun replaceTxt(payload: String): String {
        val requestJson = JSONObject(payload)
        var content = requestJson.optString("content")
        val replacements = requestJson.optJSONArray("replacements") ?: JSONArray()
        for (index in 0 until replacements.length()) {
            val replacement = replacements.getJSONObject(index)
            val oldName = replacement.optString("old_canonical").ifBlank {
                replacement.optString("old_alias")
            }
            val newName = replacement.optString("new_canonical").ifBlank {
                replacement.optString("new_alias")
            }
            content = content.replace(oldName, newName)
        }
        return JSONObject().put("ok", true).put("updated_content", content).toString()
    }
}

private class FixtureConfigStorage(val root: File) {
    fun write(relativePath: String, content: String) {
        val file = File(root, relativePath)
        file.parentFile?.mkdirs()
        file.writeText(content)
    }

    fun read(relativePath: String): String = File(root, relativePath).readText()
}

private class FixtureTextStorage(
    val files: LinkedHashMap<String, String>
) : TextStorage {
    override fun listTxtFiles(): TxtHistoryListResult = TxtHistoryListResult(
        ok = true,
        files = files.keys.toList(),
        message = "ok"
    )

    override fun readTxtFile(relativePath: String): TxtFileContentResult {
        val content = files[relativePath]
            ?: return TxtFileContentResult(false, relativePath, "", "missing TXT")
        return TxtFileContentResult(true, relativePath, content, "ok")
    }

    override fun writeTxtFile(relativePath: String, content: String): TxtFileContentResult {
        files[relativePath] = content
        return TxtFileContentResult(true, relativePath, content, "ok")
    }
}
