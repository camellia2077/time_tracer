package com.example.tracer

import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import org.json.JSONObject

// Converts only the text supplied by the caller. File selection, month scope,
// draft ownership, and persistence remain in the feature module.
internal class RuntimeTxtActivityNameService(
    private val initializeRuntimeInternal: () -> NativeCallResult,
    private val nativeTxt: (String) -> String,
    private val codec: NativeTxtRuntimeCodec
) {
    suspend fun convertTxtActivityNames(
        content: String,
        direction: TxtActivityNameMappingDirection
    ): TxtActivityNameConversionResult = withContext(Dispatchers.IO) {
        if (!initializeRuntimeInternal().initialized) {
            return@withContext TxtActivityNameConversionResult(
                ok = false,
                convertedContent = content,
                message = "native init failed."
            )
        }

        try {
            codec.parseActivityNameConversion(
                nativeTxt(
                    JSONObject()
                        .put("action", "convert_activity_names")
                        .put("content", content)
                        .put("direction", direction.wireValue)
                        .toString()
                ),
                fallbackContent = content
            )
        } catch (error: Exception) {
            TxtActivityNameConversionResult(
                ok = false,
                convertedContent = content,
                message = formatNativeFailure(
                    "convert txt activity names failed",
                    error
                )
            )
        }
    }

    suspend fun replaceTxtCanonicalActivityNames(
        content: String,
        replacements: List<CanonicalActivityNameReplacement>
    ): TxtCanonicalActivityReplacementResult = withContext(Dispatchers.IO) {
        if (!initializeRuntimeInternal().initialized) {
            return@withContext TxtCanonicalActivityReplacementResult(
                ok = false,
                updatedContent = content,
                message = "native init failed."
            )
        }
        try {
            codec.parseCanonicalActivityReplacement(
                nativeTxt(
                    JSONObject()
                        .put("action", "replace_canonical_activity_names")
                        .put("content", content)
                        .put("replacements", org.json.JSONArray().apply {
                            replacements.forEach { replacement ->
                                put(
                                    JSONObject()
                                        .put("old_canonical", replacement.oldCanonical)
                                        .put("new_canonical", replacement.newCanonical)
                                )
                            }
                        })
                        .toString()
                ),
                fallbackContent = content
            )
        } catch (error: Exception) {
            TxtCanonicalActivityReplacementResult(
                ok = false,
                updatedContent = content,
                message = formatNativeFailure(
                    "replace txt canonical activity names failed",
                    error
                )
            )
        }
    }
}
