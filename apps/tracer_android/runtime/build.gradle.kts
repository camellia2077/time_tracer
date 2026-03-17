import org.gradle.api.DefaultTask
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.RegularFileProperty
import org.gradle.api.provider.Property
import org.gradle.api.tasks.Input
import org.gradle.api.tasks.InputDirectory
import org.gradle.api.tasks.InputFile
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.PathSensitive
import org.gradle.api.tasks.PathSensitivity
import org.gradle.api.tasks.TaskAction
import org.gradle.kotlin.dsl.register
import org.gradle.process.ExecOperations
import javax.inject.Inject

plugins {
    alias(libs.plugins.android.library)
}

abstract class SyncDebugInputSeedAssetsTask @Inject constructor(
    private val execOperations: ExecOperations,
) : DefaultTask() {
    @get:Input
    abstract val pythonExecutable: Property<String>

    @get:InputFile
    @get:PathSensitive(PathSensitivity.NONE)
    abstract val syncScript: RegularFileProperty

    @get:InputDirectory
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val sourceRoot: DirectoryProperty

    @get:OutputDirectory
    abstract val outputDirectory: DirectoryProperty

    @TaskAction
    fun sync() {
        val assetRoot = outputDirectory.get().asFile
        val inputFullRoot = assetRoot.resolve("tracer_core/input/full")
        inputFullRoot.mkdirs()
        execOperations.exec {
            commandLine(
                pythonExecutable.get(),
                syncScript.get().asFile.absolutePath,
                "--source-root",
                sourceRoot.get().asFile.absolutePath,
                "--android-input-full-root",
                inputFullRoot.absolutePath,
                "--apply",
                "--prune-managed-years",
            )
        }.assertNormalExitValue()
    }
}

val repoRootDir =
    rootProject.projectDir.parentFile?.parentFile
        ?: throw GradleException(
            "Cannot resolve repository root from ${rootProject.projectDir.absolutePath}"
        )

val timeTracerSourceConfigRootProperty = providers.gradleProperty("timeTracerSourceConfigRoot").orNull
val timeTracerConfigRootProperty = providers.gradleProperty("timeTracerConfigRoot").orNull
val timeTracerPythonProperty = providers.gradleProperty("timeTracerPython").orNull?.trim().orEmpty()
val timeTracerDisableNativeOptimization =
    providers.gradleProperty("timeTracerDisableNativeOptimization")
        .orNull
        ?.trim()
        ?.equals("true", ignoreCase = true) == true
val timeTracerSourceTestDataRootProperty = providers.gradleProperty("timeTracerSourceTestDataRoot").orNull
val timeTracerAndroidDebugAssetsRootProperty = providers.gradleProperty("timeTracerAndroidDebugAssetsRoot").orNull
val legacyTimeTracerAndroidDebugInputFullRootProperty =
    providers.gradleProperty("timeTracerAndroidDebugInputFullRoot").orNull
        ?: providers.gradleProperty("timeTracerAndroidInputFullRoot").orNull

val timeTracerSourceConfigRoot =
    timeTracerSourceConfigRootProperty?.let { file(it) }
        ?: repoRootDir.resolve("assets/tracer_core/config")
val timeTracerSourceTestDataRoot =
    timeTracerSourceTestDataRootProperty?.let { file(it) }
        ?: repoRootDir.resolve("test/data")
val defaultDebugAssetsRoot = layout.buildDirectory.dir("generated/tracer/runtime/debug/assets").get().asFile
val timeTracerConfigRootFile =
    timeTracerConfigRootProperty?.let { file(it) }
        ?: projectDir.resolve("src/main/assets/tracer_core/config")
val timeTracerAndroidDebugAssetsRoot =
    timeTracerAndroidDebugAssetsRootProperty?.let { file(it) }
        ?: legacyTimeTracerAndroidDebugInputFullRootProperty?.let { file(it).parentFile?.parentFile?.parentFile }
        ?: defaultDebugAssetsRoot
val platformConfigRunner = repoRootDir.resolve("tools/platform_config/run.py")
val inputDataSyncRunner =
    repoRootDir.resolve("tools/scripts/devtools/android/sync_android_input_from_test_data.py")
val pythonExecutable =
    if (timeTracerPythonProperty.isNotEmpty()) {
        timeTracerPythonProperty
    } else if (System.getProperty("os.name").lowercase().contains("windows")) {
        "python"
    } else {
        "python3"
    }

val repoRootPath = repoRootDir.absolutePath
val timeTracerSourceConfigRootPath = timeTracerSourceConfigRoot.absolutePath
val timeTracerSourceTestDataRootPath = timeTracerSourceTestDataRoot.absolutePath
val timeTracerConfigRootPath = timeTracerConfigRootFile.absolutePath
val timeTracerAndroidDebugAssetsRootPath = timeTracerAndroidDebugAssetsRoot.absolutePath
val platformConfigRunnerPath = platformConfigRunner.absolutePath
val inputDataSyncRunnerPath = inputDataSyncRunner.absolutePath

if (!platformConfigRunner.exists()) {
    throw GradleException(
        "Missing platform config generator: $platformConfigRunnerPath"
    )
}
if (!timeTracerSourceConfigRoot.exists()) {
    throw GradleException(
        "Missing source config root: $timeTracerSourceConfigRootPath"
    )
}

val syncTracerCoreConfig by tasks.registering(Exec::class) {
    group = "tracer_core"
    description = "Generate Android tracer_core config bundle before build."
    workingDir = File(repoRootPath)
    inputs.file(platformConfigRunnerPath)
    inputs.dir(timeTracerSourceConfigRootPath)
    outputs.dir(timeTracerConfigRootPath)
    commandLine(
        pythonExecutable,
        platformConfigRunnerPath,
        "--target",
        "android",
        "--source-root",
        timeTracerSourceConfigRootPath,
        "--android-output-root",
        timeTracerConfigRootPath,
        "--apply",
    )
}

val androidPythonExecutable = pythonExecutable

val syncTracerCoreDebugInputData = tasks.register<SyncDebugInputSeedAssetsTask>("syncTracerCoreDebugInputData") {
    group = "tracer_core"
    description = "Sync Android debug input/full seed TXT data from canonical test/data before build."
    pythonExecutable.set(androidPythonExecutable)
    syncScript.set(file(inputDataSyncRunnerPath))
    sourceRoot.set(file(timeTracerSourceTestDataRootPath))
    outputDirectory.set(file(timeTracerAndroidDebugAssetsRootPath))
}

tasks.matching { it.name == "preBuild" }.configureEach {
    dependsOn(syncTracerCoreConfig)
}

android {
    namespace = "com.example.tracer.runtime"
    compileSdk = 36
    ndkVersion = "29.0.14206865"

    defaultConfig {
        minSdk = 24

        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }

        externalNativeBuild {
            cmake {
                arguments.add("-DANDROID_STL=c++_static")
                // Work around NDK 29 Clang mis-handling std::as_const in libc++.
                cppFlags += listOf(
                    "-std=c++23",
                    "-Xclang",
                    "-fno-builtin-std-as_const",
                )
                targets += listOf("tt_android_bridge")
            }
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
        isCoreLibraryDesugaringEnabled = true
    }

    externalNativeBuild {
        cmake {
            path = file("src/main/cpp/CMakeLists.txt")
            version = "4.1.2"
        }
    }

    buildTypes {
        release {
            externalNativeBuild {
                cmake {
                    arguments += listOf(
                        "-DCMAKE_BUILD_TYPE=Release",
                        if (timeTracerDisableNativeOptimization) {
                            "-DDISABLE_OPTIMIZATION=ON"
                        } else {
                            "-DDISABLE_OPTIMIZATION=OFF"
                        },
                        "-DENABLE_LTO=OFF"
                    )
                    cFlags += listOf("-g0")
                    cppFlags += listOf("-g0")
                }
            }
        }
    }

}

androidComponents {
    onVariants(selector().withBuildType("debug")) { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(
            syncTracerCoreDebugInputData,
            SyncDebugInputSeedAssetsTask::outputDirectory,
        )
    }
}

dependencies {
    coreLibraryDesugaring(libs.desugar.jdk.libs)
    implementation(project(":contract"))
    implementation(libs.kotlinx.coroutines.android)
    testImplementation(libs.junit)
    testImplementation(libs.org.json)
}

