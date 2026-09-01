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
import org.gradle.api.tasks.Sync
import org.gradle.api.tasks.TaskAction
import org.gradle.kotlin.dsl.register
import org.gradle.process.ExecOperations
import javax.inject.Inject

plugins {
    alias(libs.plugins.android.library)
}

abstract class SyncPlatformConfigSnapshotTask
    @Inject
    constructor(
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

            execOperations
                .exec {
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

val repoRootDir =
    rootProject.projectDir.parentFile?.parentFile
        ?: throw GradleException(
            "Cannot resolve repository root from ${rootProject.projectDir.absolutePath}",
        )

val timeTracerDisableNativeOptimization =
    providers
        .gradleProperty("timeTracerDisableNativeOptimization")
        .orNull
        ?.trim()
        ?.equals("true", ignoreCase = true) == true
val timeTracerSourceConfigRoot = repoRootDir.resolve("config/program")
val timeTracerGeneratedAssetsRoot =
    layout.buildDirectory.dir("generated/tracer/assets")
val timeTracerGeneratedConfigRoot =
    layout.buildDirectory.dir("generated/tracer/config")
val platformConfigRunner = repoRootDir.resolve("tools/platform_config/run.py")
val pythonExecutableCommand =
    if (System.getProperty("os.name").lowercase().contains("windows")) {
        "python"
    } else {
        "python3"
    }

val timeTracerSourceConfigRootPath = timeTracerSourceConfigRoot.absolutePath
val platformConfigRunnerPath = platformConfigRunner.absolutePath

val generateTracerCoreConfigAssets by tasks.register<SyncPlatformConfigSnapshotTask>(
    "generateTracerCoreConfigAssets",
) {
    group = "tracer_core"
    description = "Generate Android config/program assets from the canonical source config."
    pythonExecutable.set(pythonExecutableCommand)
    target.set("android")
    syncScript.set(file(platformConfigRunnerPath))
    sourceRoot.set(file(timeTracerSourceConfigRootPath))
    outputDirectory.set(timeTracerGeneratedConfigRoot)
}

val stageTracerCoreConfigAssets by tasks.register<Sync>("stageTracerCoreConfigAssets") {
    group = "tracer_core"
    description = "Stage generated config/program assets for the Android APK."
    dependsOn(generateTracerCoreConfigAssets)
    from(timeTracerGeneratedConfigRoot.map { it.dir("program") })
    into(timeTracerGeneratedAssetsRoot.map { it.dir("config/program") })
}

tasks.matching { it.name == "preBuild" }.configureEach {
    dependsOn(stageTracerCoreConfigAssets)
}

android {
    namespace = "com.example.tracer.runtime"

    sourceSets {
        getByName("main").assets.srcDir(timeTracerGeneratedAssetsRoot.get().asFile)
    }

    compileSdk = 37
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
                cppFlags +=
                    listOf(
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
                    arguments +=
                        listOf(
                            "-DCMAKE_BUILD_TYPE=Release",
                            if (timeTracerDisableNativeOptimization) {
                                "-DDISABLE_OPTIMIZATION=ON"
                            } else {
                                "-DDISABLE_OPTIMIZATION=OFF"
                            },
                            "-DENABLE_LTO=OFF",
                        )
                    cFlags += listOf("-g0")
                    cppFlags += listOf("-g0")
                }
            }
        }
    }
}

dependencies {
    coreLibraryDesugaring(libs.desugar.jdk.libs)
    implementation(project(":contract"))
    implementation(libs.kotlinx.coroutines.android)
    testImplementation(libs.junit)
    testImplementation(libs.org.json)
}
