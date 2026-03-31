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

val keystoreProperties = loadOptionalProperties(rootProject.file("keystore.properties"))
val androidVersionProperties =
    loadOptionalProperties(rootProject.file("meta/version.properties"))
val tracerCoreVersionHeader = rootProject.file("../../libs/tracer_core/src/shared/types/version.hpp")
val tracerCoreVersion = readTracerCoreVersion(tracerCoreVersionHeader)
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
        minSdk = 35
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
            "Verify packaged assets policy for $variant: bundled licenses present and no runtime TXT fixtures packaged."
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

                if (bundledTxtEntries.isNotEmpty()) {
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

val renameReleaseApk by tasks.registering(Copy::class) {
    from(releaseApkDir)
    include("*-release.apk")
    rename { "Tracer.apk" }
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
    androidTestImplementation(libs.androidx.junit)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation(libs.androidx.compose.ui.test.junit4)

    debugImplementation(libs.androidx.compose.ui.tooling)
    debugImplementation(libs.androidx.compose.ui.test.manifest)
}
