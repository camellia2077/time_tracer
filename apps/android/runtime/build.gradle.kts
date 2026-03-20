import org.gradle.api.DefaultTask
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.FileSystemOperations
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
    private val fileSystemOperations: FileSystemOperations,
) : DefaultTask() {
    @get:InputDirectory
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val sourceRoot: DirectoryProperty

    @get:OutputDirectory
    abstract val outputDirectory: DirectoryProperty

    @TaskAction
    fun sync() {
        val assetRoot = outputDirectory.get().asFile
        val inputFullRoot = assetRoot.resolve("tracer_core/input/full")
        val sourceDir = sourceRoot.get().asFile
        require(sourceDir.exists()) {
            "Missing debug seed TXT source root: ${sourceDir.absolutePath}"
        }

        fileSystemOperations.sync {
            from(sourceRoot)
            include("**/*.txt")
            into(inputFullRoot)
        }
    }
}

abstract class SyncPlatformConfigSnapshotTask @Inject constructor(
    private val execOperations: ExecOperations,
) : DefaultTask() {
    @get:Input
    abstract val pythonExecutable: Property<String>

    @get:Input
    abstract val target: Property<String>

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
        val scriptFile = syncScript.get().asFile
        val sourceDir = sourceRoot.get().asFile
        val outputDir = outputDirectory.get().asFile
        require(scriptFile.exists()) {
            "Missing platform config generator: ${scriptFile.absolutePath}"
        }
        require(sourceDir.exists()) {
            "Missing source config root: ${sourceDir.absolutePath}"
        }

        execOperations.exec {
            commandLine(
                pythonExecutable.get(),
                scriptFile.absolutePath,
                "--target",
                target.get(),
                "--source-root",
                sourceDir.absolutePath,
                "--android-output-root",
                outputDir.absolutePath,
                "--apply",
            )
        }.assertNormalExitValue()
    }
}

abstract class VerifyPlatformConfigSnapshotTask @Inject constructor(
    private val execOperations: ExecOperations,
) : DefaultTask() {
    @get:Input
    abstract val pythonExecutable: Property<String>

    @get:Input
    abstract val target: Property<String>

    @get:InputFile
    @get:PathSensitive(PathSensitivity.NONE)
    abstract val syncScript: RegularFileProperty

    @get:InputDirectory
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val sourceRoot: DirectoryProperty

    @get:InputDirectory
    @get:PathSensitive(PathSensitivity.RELATIVE)
    abstract val snapshotRoot: DirectoryProperty

    @TaskAction
    fun verify() {
        val scriptFile = syncScript.get().asFile
        val sourceDir = sourceRoot.get().asFile
        val outputDir = snapshotRoot.get().asFile
        require(scriptFile.exists()) {
            "Missing platform config generator: ${scriptFile.absolutePath}"
        }
        require(sourceDir.exists()) {
            "Missing source config root: ${sourceDir.absolutePath}"
        }
        require(outputDir.exists()) {
            "Missing Android config snapshot root: ${outputDir.absolutePath}"
        }

        execOperations.exec {
            commandLine(
                pythonExecutable.get(),
                scriptFile.absolutePath,
                "--target",
                target.get(),
                "--source-root",
                sourceDir.absolutePath,
                "--android-output-root",
                outputDir.absolutePath,
                "--check",
            )
        }.assertNormalExitValue()
    }
}

val repoRootDir =
    rootProject.projectDir.parentFile?.parentFile
        ?: throw GradleException(
            "Cannot resolve repository root from ${rootProject.projectDir.absolutePath}"
        )

val timeTracerDisableNativeOptimization =
    providers.gradleProperty("timeTracerDisableNativeOptimization")
        .orNull
        ?.trim()
        ?.equals("true", ignoreCase = true) == true
val timeTracerSourceConfigRoot = repoRootDir.resolve("assets/tracer_core/config")
val timeTracerSourceTestDataRoot = repoRootDir.resolve("test/data")
val defaultDebugAssetsRoot = layout.buildDirectory.dir("generated/tracer/runtime/debug/assets").get().asFile
val timeTracerConfigRootFile = projectDir.resolve("src/main/assets/tracer_core/config")
val timeTracerAndroidDebugAssetsRoot = defaultDebugAssetsRoot
val platformConfigRunner = repoRootDir.resolve("tools/platform_config/run.py")
val pythonExecutableCommand =
    if (System.getProperty("os.name").lowercase().contains("windows")) {
        "python"
    } else {
        "python3"
    }

val timeTracerSourceConfigRootPath = timeTracerSourceConfigRoot.absolutePath
val timeTracerSourceTestDataRootPath = timeTracerSourceTestDataRoot.absolutePath
val timeTracerConfigRootPath = timeTracerConfigRootFile.absolutePath
val timeTracerAndroidDebugAssetsRootPath = timeTracerAndroidDebugAssetsRoot.absolutePath
val platformConfigRunnerPath = platformConfigRunner.absolutePath

val syncTracerCoreConfigSnapshot by tasks.register<SyncPlatformConfigSnapshotTask>("syncTracerCoreConfigSnapshot") {
    group = "tracer_core"
    description = "Refresh the checked-in Android tracer_core config snapshot from canonical source config."
    pythonExecutable.set(pythonExecutableCommand)
    target.set("android")
    syncScript.set(file(platformConfigRunnerPath))
    sourceRoot.set(file(timeTracerSourceConfigRootPath))
    outputDirectory.set(file(timeTracerConfigRootPath))
}

val verifyTracerCoreConfigSnapshot by tasks.register<VerifyPlatformConfigSnapshotTask>("verifyTracerCoreConfigSnapshot") {
    group = "verification"
    description = "Fail when the checked-in Android tracer_core config snapshot drifts from canonical source config."
    pythonExecutable.set(pythonExecutableCommand)
    target.set("android")
    syncScript.set(file(platformConfigRunnerPath))
    sourceRoot.set(file(timeTracerSourceConfigRootPath))
    snapshotRoot.set(file(timeTracerConfigRootPath))
}

val syncTracerCoreDebugInputData = tasks.register<SyncDebugInputSeedAssetsTask>("syncTracerCoreDebugInputData") {
    group = "tracer_core"
    description = "Sync Android debug input/full seed TXT assets from canonical test/data using pure Gradle Sync semantics."
    sourceRoot.set(file(timeTracerSourceTestDataRootPath))
    outputDirectory.set(file(timeTracerAndroidDebugAssetsRootPath))
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

