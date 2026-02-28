plugins {
    alias(libs.plugins.android.library)
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
val timeTracerAndroidInputFullRootProperty = providers.gradleProperty("timeTracerAndroidInputFullRoot").orNull

val timeTracerSourceConfigRoot =
    timeTracerSourceConfigRootProperty?.let { file(it) }
        ?: repoRootDir.resolve("apps/tracer_core/config")
val timeTracerSourceTestDataRoot =
    timeTracerSourceTestDataRootProperty?.let { file(it) }
        ?: repoRootDir.resolve("test/data")
val timeTracerConfigRootFile =
    timeTracerConfigRootProperty?.let { file(it) }
        ?: projectDir.resolve("src/main/assets/tracer_core/config")
val timeTracerAndroidInputFullRoot =
    timeTracerAndroidInputFullRootProperty?.let { file(it) }
        ?: projectDir.resolve("src/main/assets/tracer_core/input/full")
val platformConfigRunner = repoRootDir.resolve("scripts/platform_config/run.py")
val inputDataSyncRunner = repoRootDir.resolve("scripts/tools/sync_android_input_from_test_data.py")
val pythonExecutable =
    if (timeTracerPythonProperty.isNotEmpty()) {
        timeTracerPythonProperty
    } else if (System.getProperty("os.name").lowercase().contains("windows")) {
        "python"
    } else {
        "python3"
    }

val syncTracerCoreConfig by tasks.registering(Exec::class) {
    group = "tracer_core"
    description = "Generate Android tracer_core config bundle before build."
    workingDir = repoRootDir
    doFirst {
        if (!platformConfigRunner.exists()) {
            throw GradleException(
                "Missing platform config generator: ${platformConfigRunner.absolutePath}"
            )
        }
        if (!timeTracerSourceConfigRoot.exists()) {
            throw GradleException(
                "Missing source config root: ${timeTracerSourceConfigRoot.absolutePath}"
            )
        }
        commandLine(
            pythonExecutable,
            platformConfigRunner.absolutePath,
            "--target",
            "android",
            "--source-root",
            timeTracerSourceConfigRoot.absolutePath,
            "--android-output-root",
            timeTracerConfigRootFile.absolutePath,
            "--apply",
        )
        logger.lifecycle(
            "sync Android config via scripts/platform_config/run.py " +
                "(source=${timeTracerSourceConfigRoot.absolutePath}, " +
                "output=${timeTracerConfigRootFile.absolutePath})"
        )
    }
}

val syncTracerCoreInputData by tasks.registering(Exec::class) {
    group = "tracer_core"
    description = "Sync Android input/full from canonical test/data before build."
    workingDir = repoRootDir
    doFirst {
        if (!inputDataSyncRunner.exists()) {
            throw GradleException(
                "Missing input data sync script: ${inputDataSyncRunner.absolutePath}"
            )
        }
        if (!timeTracerSourceTestDataRoot.exists()) {
            throw GradleException(
                "Missing source test data root: ${timeTracerSourceTestDataRoot.absolutePath}"
            )
        }
        commandLine(
            pythonExecutable,
            inputDataSyncRunner.absolutePath,
            "--source-root",
            timeTracerSourceTestDataRoot.absolutePath,
            "--android-input-full-root",
            timeTracerAndroidInputFullRoot.absolutePath,
            "--apply",
        )
        logger.lifecycle(
            "sync Android input/full via scripts/tools/sync_android_input_from_test_data.py " +
                "(source=${timeTracerSourceTestDataRoot.absolutePath}, " +
                "output=${timeTracerAndroidInputFullRoot.absolutePath})"
        )
    }
}

tasks.matching { it.name == "preBuild" }.configureEach {
    dependsOn(syncTracerCoreConfig)
    dependsOn(syncTracerCoreInputData)
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
                arguments += listOf("-DANDROID_STL=c++_static")
                cppFlags += listOf("-std=c++23")
                targets += listOf("time_tracker_android_bridge")
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

dependencies {
    coreLibraryDesugaring(libs.desugar.jdk.libs)
    implementation(project(":contract"))
    implementation(libs.kotlinx.coroutines.android)
    testImplementation(libs.junit)
    testImplementation(libs.org.json)
}
