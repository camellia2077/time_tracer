import java.io.FileInputStream
import java.util.Properties

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.compose)
}

val keystoreProperties = Properties()
val keystorePropertiesFile = rootProject.file("local.properties")
if (keystorePropertiesFile.exists()) {
    keystoreProperties.load(FileInputStream(keystorePropertiesFile))
}

android {
    namespace = "com.example.tracer"
    compileSdk = 36
    ndkVersion = "29.0.14206865"

    androidResources {
        localeFilters += listOf("zh", "en", "ja")
    }

    signingConfigs {
        create("release") {
            keyAlias = keystoreProperties["KEY_ALIAS"] as String? ?: "my-alias"
            keyPassword = keystoreProperties["KEY_PASSWORD"] as String? ?: "password"
            storeFile = file(keystoreProperties["STORE_FILE"] as String? ?: "my-release-key.jks")
            storePassword = keystoreProperties["STORE_PASSWORD"] as String? ?: "password"
        }
    }

    defaultConfig {
        applicationId = "com.example.tracer"
        minSdk = 35
        targetSdk = 36
        versionCode = 1
        versionName = "0.2.1"

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
    }

    lint {
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

val renameReleaseApk by tasks.registering {
    doLast {
        val apkDir =
            layout.buildDirectory
                .dir("outputs/apk/release")
                .get()
                .asFile
        val apks =
            apkDir
                .listFiles()
                ?.filter { it.isFile && it.extension == "apk" }
                ?: emptyList()
        val source =
            apks.firstOrNull { it.name.endsWith("-release.apk") }
                ?: apks.firstOrNull()
                ?: throw GradleException("No APK found in ${apkDir.absolutePath}")
        val target = apkDir.resolve("Tracer.apk")

        if (source.absolutePath != target.absolutePath) {
            source.copyTo(target, overwrite = true)
        }
        apks
            .filter { it.absolutePath != target.absolutePath }
            .forEach { it.delete() }
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
