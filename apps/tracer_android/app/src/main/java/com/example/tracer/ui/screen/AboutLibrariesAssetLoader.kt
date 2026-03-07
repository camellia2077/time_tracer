package com.example.tracer

import android.content.Context
import com.mikepenz.aboutlibraries.Libs
import java.io.IOException

private const val ABOUT_LIBRARIES_ASSET_PATH = "aboutlibraries/aboutlibraries.json"

internal object AboutLibrariesAssetLoader {
    fun load(context: Context): Libs {
        val json =
            context.assets.open(ABOUT_LIBRARIES_ASSET_PATH).bufferedReader().use { reader ->
                reader.readText()
            }
        if (json.isBlank()) {
            throw IOException("Bundled AboutLibraries asset is empty: $ABOUT_LIBRARIES_ASSET_PATH")
        }

        val libs = Libs.Builder().withJson(json).build()
        if (libs.libraries.isEmpty() && libs.licenses.isEmpty()) {
            throw IOException(
                "Bundled AboutLibraries asset parsed successfully but produced no libraries: " +
                    ABOUT_LIBRARIES_ASSET_PATH
            )
        }
        return libs
    }
}
