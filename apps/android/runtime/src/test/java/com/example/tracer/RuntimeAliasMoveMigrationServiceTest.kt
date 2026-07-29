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

class RuntimeAliasMoveMigrationServiceTest {
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
                destinationToml = "[aliases.breakfast]\n\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "[aliases.breakfast]\n\"eat\" = [\"吃饭\"]\n\"go\" = [\"围棋\"]",
                oldCanonical = "exercise_go",
                newCanonical = "meal_breakfast_go"
            ),
            MoveScenario(
                name = "leaf to other TOML group subtree",
                sourceToml = "\"go\" = [\"围棋\"]",
                destinationToml = "[aliases.breakfast.morning]\n\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "[aliases.breakfast.morning]\n\"eat\" = [\"吃饭\"]\n\"go\" = [\"围棋\"]",
                oldCanonical = "exercise_go",
                newCanonical = "meal_breakfast_morning_go"
            ),
            MoveScenario(
                name = "group subtree to other TOML root",
                sourceToml = "[aliases.cardio]\n\"swimming\" = [\"游泳\"]\n[aliases.cardio.running]\n\"track-running\" = [\"操场跑\"]",
                destinationToml = "\"eat\" = [\"吃饭\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "\"eat\" = [\"吃饭\"]\n[aliases.cardio]\n\"swimming\" = [\"游泳\"]\n[aliases.cardio.running]\n\"track-running\" = [\"操场跑\"]",
                oldCanonical = "exercise_cardio",
                newCanonical = "meal_cardio",
                extraReplacements = listOf(
                    CanonicalActivityNameReplacement("exercise_cardio_swimming", "meal_cardio_swimming"),
                    CanonicalActivityNameReplacement("exercise_cardio_running_track-running", "meal_cardio_running_track-running")
                )
            ),
            MoveScenario(
                name = "group subtree to other TOML group subtree",
                sourceToml = "[aliases.cardio]\n\"swimming\" = [\"游泳\"]\n[aliases.cardio.running]\n\"track-running\" = [\"操场跑\"]",
                destinationToml = "[aliases.fitness.deep]\n\"stretch\" = [\"拉伸\"]",
                updatedSourceToml = "",
                updatedDestinationToml = "[aliases.fitness.deep]\n\"stretch\" = [\"拉伸\"]\n[aliases.fitness.deep.cardio]\n\"swimming\" = [\"游泳\"]\n[aliases.fitness.deep.cardio.running]\n\"track-running\" = [\"操场跑\"]",
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
                assertEquals(scenario.updatedSourceToml, fixture.config.read("activity_hierarchy/exercise.toml"))
                assertEquals(scenario.updatedDestinationToml, fixture.config.read("activity_hierarchy/meal.toml"))
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
            assertEquals(fixture.scenario.sourceToml, fixture.config.read("activity_hierarchy/exercise.toml"))
            assertEquals(fixture.scenario.destinationToml, fixture.config.read("activity_hierarchy/meal.toml"))
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
    private val failCandidateIngest: Boolean = false
) {
    val root = Files.createTempDirectory("alias-move-migration-").toFile()
    val config = FixtureConfigStorage(File(root, "config")).also {
        it.write("activity_hierarchy/exercise.toml", scenario.sourceToml)
        it.write("activity_hierarchy/meal.toml", scenario.destinationToml)
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

    val request = AliasEntryMoveMigrationRequest(
        configRelativePath = "activity_hierarchy/exercise.toml",
        updatedTomlContent = scenario.updatedSourceToml,
        replacements = listOf(
            CanonicalActivityNameReplacement(scenario.oldCanonical, scenario.newCanonical)
        ) + scenario.extraReplacements,
        updatedDocuments = listOf(
            ActivityHierarchyDocumentInput("activity_hierarchy/exercise.toml", scenario.updatedSourceToml),
            ActivityHierarchyDocumentInput("activity_hierarchy/meal.toml", scenario.updatedDestinationToml)
        )
    )

    val service = RuntimeAliasMoveMigrationService(
        ensureRuntimePaths = { paths },
        ensureTextStorage = { text },
        ensureConfigTomlStorage = { ConfigTomlStorage(config.root.absolutePath) },
        nativeInit = { candidatePaths ->
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
