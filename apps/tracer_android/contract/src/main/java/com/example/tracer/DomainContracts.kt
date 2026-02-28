package com.example.tracer

sealed interface DomainResult<out T> {
    data class Success<T>(val value: T) : DomainResult<T>
    data class Failure(val error: DomainError) : DomainResult<Nothing>
}

sealed interface DomainError {
    val userMessage: String
    val debugMessage: String
    val errorLogPath: String
    val operationId: String
}

data class ValidationError(
    override val userMessage: String,
    override val debugMessage: String = userMessage,
    override val errorLogPath: String = "",
    override val operationId: String = ""
) : DomainError

data class CoreError(
    override val userMessage: String,
    override val debugMessage: String = userMessage,
    override val errorLogPath: String = "",
    override val operationId: String = ""
) : DomainError

data class IoError(
    override val userMessage: String,
    override val debugMessage: String = userMessage,
    override val errorLogPath: String = "",
    override val operationId: String = ""
) : DomainError

data class UnknownError(
    override val userMessage: String,
    override val debugMessage: String = userMessage,
    override val errorLogPath: String = "",
    override val operationId: String = ""
) : DomainError

inline fun <T, R> DomainResult<T>.fold(
    onSuccess: (T) -> R,
    onFailure: (DomainError) -> R
): R {
    return when (this) {
        is DomainResult.Success -> onSuccess(value)
        is DomainResult.Failure -> onFailure(error)
    }
}

fun DomainError.legacyMessage(): String {
    val baseMessage = when {
        userMessage.isNotBlank() -> userMessage
        debugMessage.isNotBlank() -> debugMessage
        else -> "unknown domain error."
    }

    val details = buildList {
        if (operationId.isNotBlank()) {
            add("op=$operationId")
        }
        if (errorLogPath.isNotBlank()) {
            add("log=$errorLogPath")
        }
    }
    if (details.isEmpty()) {
        return baseMessage
    }
    return "$baseMessage [${details.joinToString(", ")}]"
}
