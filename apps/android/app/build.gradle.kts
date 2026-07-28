import org.gradle.api.DefaultTask
import org.gradle.api.file.DirectoryProperty
import org.gradle.api.file.RegularFileProperty
import org.gradle.api.tasks.InputFile
import org.gradle.api.tasks.OutputDirectory
import org.gradle.api.tasks.PathSensitive
import org.gradle.api.tasks.PathSensitivity
import org.gradle.api.tasks.TaskAction
import org.gradle.kotlin.dsl.register
import java.io.FileInputStream
import java.util.Properties
import java.util.zip.ZipFile

abstract class GenerateAboutLibrariesAssetTask : DefaultTask() {
    @get:InputFile
    @get:PathSensitive(PathSensitivity.NONE)
    abstract val sourceJson: RegularFileProperty

    @get:OutputDirectory
    abstract val outputDirectory: DirectoryProperty

    @TaskAction
    fun generate() {
        val sourceFile = sourceJson.get().asFile
        if (!sourceFile.exists()) {
            throw GradleException(
                "Missing generated AboutLibraries metadata: ${sourceFile.absolutePath}",
            )
        }
        if (sourceFile.length() <= 0L) {
            throw GradleException(
                "Generated AboutLibraries metadata is empty: ${sourceFile.absolutePath}",
            )
        }

        val outputRoot = outputDirectory.get().asFile
        val targetDir = outputRoot.resolve("aboutlibraries")
        val targetFile = targetDir.resolve("aboutlibraries.json")
        outputRoot.deleteRecursively()
        targetDir.mkdirs()
        sourceFile.copyTo(targetFile, overwrite = true)
    }
}

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.compose)
    alias(libs.plugins.aboutlibraries.android)
}

fun loadOptionalProperties(path: java.io.File): Properties {
    val properties = Properties()
    if (path.exists()) {
        FileInputStream(path).use { input ->
            properties.load(input)
        }
    }
    return properties
}

fun readTracerCoreVersion(versionHeader: java.io.File): String {
    if (!versionHeader.exists()) {
        throw GradleException("Tracer core version header not found: ${versionHeader.path}")
    }
    val versionRegex = Regex("""kVersion\s*=\s*"([^"]+)"""")
    val match =
        versionRegex.find(versionHeader.readText())
            ?: throw GradleException(
                "Failed to parse tracer core version from: ${versionHeader.path}",
            )
    return match.groupValues[1]
}

fun resolveAdbExecutable(localProperties: Properties): String {
    val adbName =
        if (System.getProperty("os.name").lowercase().contains("windows")) {
            "adb.exe"
        } else {
            "adb"
        }
    val sdkRoots =
        listOf(
            providers.environmentVariable("ANDROID_SDK_ROOT").orNull,
            providers.environmentVariable("ANDROID_HOME").orNull,
            localProperties.getProperty("sdk.dir"),
        ).filterNotNull()
            .map { it.trim() }
            .filter { it.isNotBlank() }
    for (sdkRoot in sdkRoots) {
        val adbCandidate = project.file("$sdkRoot/platform-tools/$adbName")
        if (adbCandidate.exists()) {
            return adbCandidate.absolutePath
        }
    }
    return adbName
}

fun runAdbCommand(
    adbExecutable: String,
    logFile: java.io.File,
    ignoreExitValue: Boolean = false,
    args: List<String>,
): String {
    val process =
        ProcessBuilder(listOf(adbExecutable) + args)
            .redirectErrorStream(true)
            .start()
    val text = process.inputStream.bufferedReader(Charsets.UTF_8).use { it.readText() }
    val exitCode = process.waitFor()
    logFile.parentFile.mkdirs()
    logFile.writeText(text)
    if (!ignoreExitValue && exitCode != 0) {
        throw GradleException(
            "adb ${args.joinToString(" ")} failed with exit code $exitCode. " +
                "See ${logFile.absolutePath}.",
        )
    }
    return text
}

val keystoreProperties = loadOptionalProperties(rootProject.file("keystore.properties"))
val localProperties = loadOptionalProperties(rootProject.file("local.properties"))
val androidVersionProperties =
    loadOptionalProperties(rootProject.file("meta/version.properties"))
val tracerCoreVersionHeader = rootProject.file("../../libs/tracer_core/src/shared/types/version.hpp")
val tracerCoreVersion = readTracerCoreVersion(tracerCoreVersionHeader)
val tracerConfigProfile =
    providers.gradleProperty("tracerConfigProfile")
        .orElse("test")
        .map { it.trim().lowercase() }
        .get()
        .also { profile ->
            require(profile == "distribution" || profile == "test") {
                "Unsupported tracerConfigProfile `$profile`; expected `distribution` or `test`."
            }
        }
val androidAppVersionCode =
    androidVersionProperties.getProperty("VERSION_CODE")?.toIntOrNull()
        ?: throw GradleException("Invalid or missing VERSION_CODE in meta/version.properties")
val androidAppVersionName =
    androidVersionProperties.getProperty("VERSION_NAME")?.takeIf { it.isNotBlank() }
        ?: throw GradleException("Invalid or missing VERSION_NAME in meta/version.properties")

fun resolveReleaseSigningValue(key: String): String? =
    providers.environmentVariable("TT_ANDROID_$key").orNull?.takeIf { it.isNotBlank() }
        ?: keystoreProperties.getProperty(key)?.takeIf { it.isNotBlank() }

data class ReleaseSigningConfig(
    val storeFile: String,
    val storePassword: String,
    val keyAlias: String,
    val keyPassword: String,
)

val releaseSigningConfig =
    ReleaseSigningConfig(
        storeFile = resolveReleaseSigningValue("STORE_FILE").orEmpty(),
        storePassword = resolveReleaseSigningValue("STORE_PASSWORD").orEmpty(),
        keyAlias = resolveReleaseSigningValue("KEY_ALIAS").orEmpty(),
        keyPassword = resolveReleaseSigningValue("KEY_PASSWORD").orEmpty(),
    )

val hasReleaseSigningConfig =
    releaseSigningConfig.storeFile.isNotBlank() &&
        releaseSigningConfig.storePassword.isNotBlank() &&
        releaseSigningConfig.keyAlias.isNotBlank() &&
        releaseSigningConfig.keyPassword.isNotBlank()

val isReleaseTaskRequested =
    gradle.startParameter.taskNames.any { taskName ->
        taskName.contains("release", ignoreCase = true)
    }

if (isReleaseTaskRequested && !hasReleaseSigningConfig) {
    throw GradleException(
        """
        Android release signing is not configured.

        Configure one of the following before running a release build:
        1. Create `apps/android/keystore.properties`
        2. Or provide environment variables:
           - `TT_ANDROID_STORE_FILE`
           - `TT_ANDROID_STORE_PASSWORD`
           - `TT_ANDROID_KEY_ALIAS`
           - `TT_ANDROID_KEY_PASSWORD`

        See:
        - `apps/android/keystore.properties.example`
        - `apps/android/README.md`
        """.trimIndent(),
    )
}

val resolvedAdbExecutable = resolveAdbExecutable(localProperties)

fun variantTaskSuffix(variant: String): String =
    variant.replaceFirstChar { first ->
        if (first.isLowerCase()) {
            first.titlecase()
        } else {
            first.toString()
        }
    }

fun generatedAboutLibrariesRawFile(variant: String) =
    layout.buildDirectory.file("generated/aboutLibraries/$variant/res/raw/aboutlibraries.json")

fun generatedAboutLibrariesAssetsRoot(variant: String) = layout.buildDirectory.dir("generated/tracer/aboutlibraries/$variant/assets")

val debugAboutLibrariesAssetsRoot = generatedAboutLibrariesAssetsRoot("debug")
val releaseAboutLibrariesAssetsRoot = generatedAboutLibrariesAssetsRoot("release")

android {
    namespace = "com.example.tracer"
    compileSdk = 36
    ndkVersion = "29.0.14206865"

    androidResources {
        localeFilters += listOf("zh", "en", "ja")
    }

    signingConfigs {
        create("release") {
            keyAlias = releaseSigningConfig.keyAlias.ifBlank { null }
            keyPassword = releaseSigningConfig.keyPassword.ifBlank { null }
            storeFile =
                releaseSigningConfig.storeFile
                    .takeIf { it.isNotBlank() }
                    ?.let(::file)
            storePassword = releaseSigningConfig.storePassword.ifBlank { null }
        }
    }

    defaultConfig {
        applicationId = "com.example.tracer"
        minSdk = 29
        targetSdk = 36
        versionCode = androidAppVersionCode
        versionName = androidAppVersionName
        buildConfigField("String", "TRACER_CORE_VERSION", "\"$tracerCoreVersion\"")

        ndk {
            abiFilters.addAll(listOf("arm64-v8a", "x86_64"))
        }

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            signingConfig = signingConfigs.getByName("release")
            isMinifyEnabled = true
            isShrinkResources = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro",
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
        isCoreLibraryDesugaringEnabled = true
    }

    buildFeatures {
        compose = true
        buildConfig = true
    }

    lint {
        lintConfig = file("lint.xml")
        abortOnError = true
        checkDependencies = true
        checkAllWarnings = true
        showAll = true
        explainIssues = true
        textReport = true
        htmlReport = true
        xmlReport = false
    }

    testOptions {
        unitTests.isIncludeAndroidResources = true
    }
}

fun registerAboutLibrariesAssetSyncTask(variant: String): TaskProvider<GenerateAboutLibrariesAssetTask> {
    val taskSuffix = variantTaskSuffix(variant)
    val generatedJsonFile = generatedAboutLibrariesRawFile(variant)
    val assetOutputRoot = generatedAboutLibrariesAssetsRoot(variant)
    return tasks.register<GenerateAboutLibrariesAssetTask>("sync${taskSuffix}AboutLibrariesAsset") {
        group = "aboutlibraries"
        description = "Copy generated AboutLibraries metadata into packaged assets for the $variant variant."
        dependsOn("prepareLibraryDefinitions$taskSuffix")
        sourceJson.set(generatedJsonFile)
        outputDirectory.set(assetOutputRoot)
    }
}

fun registerPackagedAssetsPolicyTask(variant: String): TaskProvider<Task> {
    val taskSuffix = variantTaskSuffix(variant)
    val apkDirs =
        listOf(
            layout.buildDirectory.dir("outputs/apk/$variant"),
            layout.buildDirectory.dir("intermediates/apk/$variant"),
        )
    return tasks.register("verify${taskSuffix}PackagedAssetsPolicy") {
        group = "verification"
        description =
            "Verify packaged assets policy for $variant and config profile $tracerConfigProfile."
        dependsOn("assemble$taskSuffix")
        doLast {
            val candidateApks =
                apkDirs
                    .map { it.get().asFile }
                    .flatMap { apkDirectory ->
                        apkDirectory
                            .walkTopDown()
                            .filter { it.isFile && it.extension.equals("apk", ignoreCase = true) }
                            .toList()
                    }.sortedBy { it.name }
            val targetApk =
                candidateApks.firstOrNull { it.name.contains(variant, ignoreCase = true) }
                    ?: candidateApks.firstOrNull()
                    ?: throw GradleException(
                        "No $variant APK found in " +
                            apkDirs.joinToString { it.get().asFile.absolutePath },
                    )

            ZipFile(targetApk).use { zipFile ->
                val licenseEntry =
                    zipFile.getEntry("assets/aboutlibraries/aboutlibraries.json")
                        ?: throw GradleException(
                            "APK ${targetApk.name} is missing assets/aboutlibraries/aboutlibraries.json",
                        )
                if (licenseEntry.size <= 0L) {
                    throw GradleException(
                        "APK ${targetApk.name} contains an empty aboutlibraries asset.",
                    )
                }

                val bundledTxtEntries = mutableListOf<String>()
                val entries = zipFile.entries()
                while (entries.hasMoreElements()) {
                    val entry = entries.nextElement()
                    if (
                        !entry.isDirectory &&
                        entry.name.startsWith("assets/tracer_core/input/") &&
                        entry.name.endsWith(".txt", ignoreCase = true)
                    ) {
                        bundledTxtEntries += entry.name
                    }
                }

                if (tracerConfigProfile == "test" && bundledTxtEntries.isEmpty()) {
                    throw GradleException(
                        "APK ${targetApk.name} is missing test runtime TXT fixtures for config profile test.",
                    )
                }
                if (tracerConfigProfile == "distribution" && bundledTxtEntries.isNotEmpty()) {
                    throw GradleException(
                        "APK ${targetApk.name} unexpectedly packaged runtime TXT fixtures: " +
                            bundledTxtEntries.take(5).joinToString(),
                    )
                }
            }
        }
    }
}

val syncDebugAboutLibrariesAsset = registerAboutLibrariesAssetSyncTask("debug")
val syncReleaseAboutLibrariesAsset = registerAboutLibrariesAssetSyncTask("release")
val verifyDebugPackagedAssetsPolicy =
    registerPackagedAssetsPolicyTask(
        variant = "debug",
    )
val verifyReleasePackagedAssetsPolicy =
    registerPackagedAssetsPolicyTask(
        variant = "release",
    )

androidComponents {
    onVariants(selector().withBuildType("debug")) { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(
            syncDebugAboutLibrariesAsset,
            GenerateAboutLibrariesAssetTask::outputDirectory,
        )
    }
    onVariants(selector().withBuildType("release")) { variant ->
        variant.sources.assets?.addGeneratedSourceDirectory(
            syncReleaseAboutLibrariesAsset,
            GenerateAboutLibrariesAssetTask::outputDirectory,
        )
    }
}

val releaseApkDir = layout.buildDirectory.dir("outputs/apk/release")
val renamedReleaseApkDir = layout.buildDirectory.dir("outputs/final-apk/release")
val renamedReleaseApkName = "Tracer-release.apk"

val renameReleaseApk by tasks.registering(Copy::class) {
    from(releaseApkDir)
    include("*-release.apk")
    rename { renamedReleaseApkName }
    into(renamedReleaseApkDir)
}

tasks.named("check").configure {
    setDependsOn(emptyList<Any>())
    dependsOn(
        "ktlintCheck",
        "lintDebug",
        "testDebugUnitTest",
        verifyDebugPackagedAssetsPolicy,
    )
}

val qaRelease by tasks.registering {
    group = "verification"
    description = "Run release APK QA: build release, lint release, and verify release packaged assets policy."
    dependsOn(
        "assembleRelease",
        "lintRelease",
        verifyReleasePackagedAssetsPolicy,
    )
}

val qaReleaseDeviceSmoke by tasks.registering {
    group = "verification"
    description =
        "Install the signed release APK on a connected device and verify that MainActivity launches without an immediate crash."
    dependsOn(
        qaRelease,
        renameReleaseApk,
    )
    val releaseApk = renamedReleaseApkDir.map { it.file(renamedReleaseApkName) }
    val reportDir = layout.buildDirectory.dir("reports/release-startup-smoke")
    inputs.file(releaseApk)
    outputs.dir(reportDir)
    doLast {
        val apk = releaseApk.get().asFile
        require(apk.exists()) {
            "Release APK not found for startup smoke: ${apk.absolutePath}"
        }

        val outputRoot = reportDir.get().asFile
        outputRoot.mkdirs()
        val applicationId = "com.example.tracer"
        val launchActivity = ".MainActivity"
        val devicesOutput =
            runAdbCommand(
                adbExecutable = resolvedAdbExecutable,
                logFile = outputRoot.resolve("adb_devices.txt"),
                args = listOf("devices"),
            )
        val attachedDevices =
            devicesOutput
                .lineSequence()
                .map { it.trim() }
                .filter { it.contains("\tdevice") }
                .toList()
        if (attachedDevices.isEmpty()) {
            throw GradleException(
                "Release startup smoke requires a connected device or emulator. " +
                    "See ${outputRoot.resolve("adb_devices.txt").absolutePath}.",
            )
        }

        runAdbCommand(
            adbExecutable = resolvedAdbExecutable,
            logFile = outputRoot.resolve("adb_start_server.txt"),
            args = listOf("start-server"),
        )
        runAdbCommand(
            adbExecutable = resolvedAdbExecutable,
            logFile = outputRoot.resolve("logcat_clear.txt"),
            args = listOf("logcat", "-c"),
        )
        runAdbCommand(
            adbExecutable = resolvedAdbExecutable,
            logFile = outputRoot.resolve("install_output.txt"),
            args = listOf("install", "-r", apk.absolutePath),
        )
        runAdbCommand(
            adbExecutable = resolvedAdbExecutable,
            logFile = outputRoot.resolve("force_stop.txt"),
            args = listOf("shell", "am", "force-stop", applicationId),
        )

        val componentName = "$applicationId/$launchActivity"
        val startOutput =
            runAdbCommand(
                adbExecutable = resolvedAdbExecutable,
                logFile = outputRoot.resolve("start_output.txt"),
                args = listOf("shell", "am", "start", "-W", "-n", componentName),
            )
        if (!startOutput.contains("Status: ok")) {
            throw GradleException(
                "Release startup smoke failed to launch $componentName. " +
                    "See ${outputRoot.resolve("start_output.txt").absolutePath}.",
            )
        }

        Thread.sleep(3000)

        val pidOutput =
            runAdbCommand(
                adbExecutable = resolvedAdbExecutable,
                logFile = outputRoot.resolve("pid_output.txt"),
                ignoreExitValue = true,
                args = listOf("shell", "pidof", applicationId),
            )
        val logcatOutput =
            runAdbCommand(
                adbExecutable = resolvedAdbExecutable,
                logFile = outputRoot.resolve("logcat_output.txt"),
                ignoreExitValue = true,
                args = listOf("logcat", "-d", "-v", "brief"),
            )

        if (pidOutput.isBlank()) {
            throw GradleException(
                "Release startup smoke could not find a live $applicationId process " +
                    "after launch. See ${outputRoot.resolve("pid_output.txt").absolutePath} " +
                    "and ${outputRoot.resolve("logcat_output.txt").absolutePath}.",
            )
        }

        val packageName = Regex.escape(applicationId)
        val fatalForApp =
            Regex("FATAL EXCEPTION(?s:.*)Process:\\s+$packageName", RegexOption.MULTILINE)
        val androidRuntimeForApp =
            Regex("AndroidRuntime(?s:.*)Process:\\s+$packageName", RegexOption.MULTILINE)
        if (
            fatalForApp.containsMatchIn(logcatOutput) ||
            androidRuntimeForApp.containsMatchIn(logcatOutput) ||
            logcatOutput.contains("UnsatisfiedLinkError") ||
            logcatOutput.contains("JNI_ERR returned from JNI_OnLoad")
        ) {
            throw GradleException(
                "Release startup smoke detected a startup crash for $applicationId. " +
                    "See ${outputRoot.resolve("logcat_output.txt").absolutePath}.",
            )
        }
    }
}

tasks.matching { it.name == "assembleRelease" }.configureEach {
    finalizedBy(renameReleaseApk)
}

dependencies {
    coreLibraryDesugaring(libs.desugar.jdk.libs)
    implementation(project(":contract"))
    implementation(project(":runtime"))
    implementation(project(":feature-data"))
    implementation(project(":feature-report"))
    implementation(project(":feature-record"))
    implementation(project(":feature-ui-common"))

    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.lifecycle.viewmodel.ktx)
    implementation(libs.androidx.lifecycle.viewmodel.compose)
    implementation(libs.androidx.activity.compose)

    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.compose.ui)
    implementation(libs.androidx.compose.ui.graphics)
    implementation(libs.androidx.compose.ui.tooling.preview)
    implementation(libs.androidx.compose.material3)
    implementation(libs.androidx.compose.material.icons.extended)
    implementation(libs.mikepenz.markdown.m3)
    implementation(libs.aboutlibraries.compose.m3)
    implementation(libs.tomlj)
    implementation(libs.androidx.datastore.preferences)
    implementation(libs.google.material)

    testImplementation(libs.junit)
    testImplementation(libs.kotlinx.coroutines.test)
    testImplementation(libs.androidx.test.core)
    testImplementation(platform(libs.androidx.compose.bom))
    testImplementation(libs.androidx.compose.ui.test.junit4)
    testImplementation(libs.robolectric)
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation(libs.androidx.compose.ui.test.junit4)

    debugImplementation(libs.androidx.compose.ui.tooling)
    debugImplementation(libs.androidx.compose.ui.test.manifest)
}
