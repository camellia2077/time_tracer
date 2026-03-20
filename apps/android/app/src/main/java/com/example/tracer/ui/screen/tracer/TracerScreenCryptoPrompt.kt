package com.example.tracer

import android.app.AlertDialog
import android.content.Context
import android.text.InputType
import android.text.method.PasswordTransformationMethod
import android.widget.EditText
import android.widget.LinearLayout
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import kotlinx.coroutines.suspendCancellableCoroutine
import kotlin.coroutines.resume

internal suspend fun promptPassphrase(
    context: Context,
    title: String,
    firstHint: String,
    secondHint: String? = null,
    requiredMessage: String,
    mismatchMessage: String
): String? = withContext(Dispatchers.Main) {
    suspendCancellableCoroutine { continuation ->
        val firstInput = buildPasswordInput(context, firstHint)
        val secondInput = if (secondHint != null) {
            buildPasswordInput(context, secondHint)
        } else {
            null
        }

        val container = LinearLayout(context).apply {
            orientation = LinearLayout.VERTICAL
            val horizontalPadding = 20.dp(context)
            val verticalPadding = 8.dp(context)
            setPadding(horizontalPadding, verticalPadding, horizontalPadding, verticalPadding)
            addView(firstInput)
            if (secondInput != null) {
                addView(secondInput)
            }
        }

        val dialog = AlertDialog.Builder(context)
            .setTitle(title)
            .setView(container)
            .setCancelable(true)
            .setNegativeButton(android.R.string.cancel) { _, _ ->
                if (continuation.isActive) {
                    continuation.resume(null)
                }
            }
            .setPositiveButton(android.R.string.ok, null)
            .create()

        dialog.setOnShowListener {
            dialog.getButton(AlertDialog.BUTTON_POSITIVE).setOnClickListener {
                val firstValue = firstInput.text.toString()
                if (firstValue.isBlank()) {
                    firstInput.error = requiredMessage
                    return@setOnClickListener
                }

                if (secondInput != null) {
                    val secondValue = secondInput.text.toString()
                    if (secondValue.isBlank()) {
                        secondInput.error = requiredMessage
                        return@setOnClickListener
                    }
                    if (firstValue != secondValue) {
                        secondInput.error = mismatchMessage
                        return@setOnClickListener
                    }
                }

                if (continuation.isActive) {
                    continuation.resume(firstValue)
                }
                dialog.dismiss()
            }
        }

        dialog.setOnCancelListener {
            if (continuation.isActive) {
                continuation.resume(null)
            }
        }

        continuation.invokeOnCancellation {
            dialog.dismiss()
        }

        dialog.show()
    }
}

private fun buildPasswordInput(context: Context, hint: String): EditText {
    return EditText(context).apply {
        this.hint = hint
        inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD
        transformationMethod = PasswordTransformationMethod.getInstance()
        setSelectAllOnFocus(true)
        layoutParams = LinearLayout.LayoutParams(
            LinearLayout.LayoutParams.MATCH_PARENT,
            LinearLayout.LayoutParams.WRAP_CONTENT
        )
    }
}

private fun Int.dp(context: Context): Int {
    val density = context.resources.displayMetrics.density
    return (this * density).toInt()
}
