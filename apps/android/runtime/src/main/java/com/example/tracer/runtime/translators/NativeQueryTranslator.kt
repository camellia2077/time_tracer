package com.example.tracer

internal class NativeQueryTranslator(
    private val responseCodec: NativeResponseCodec
) {
    fun toDataQueryTextResult(queryResult: NativeCallResult): DataQueryTextResult {
        val contentResult = toContentResult(
            queryResult = queryResult,
            defaultFailureMessage = "data query failed."
        )
        return contentResult.toLegacyDataQueryTextResult(successMessage = { "query ok" })
            .copy(operationId = queryResult.operationId)
    }

    fun toTreeQueryResult(queryResult: NativeCallResult): TreeQueryResult {
        val contentResult = toContentResult(
            queryResult = queryResult,
            defaultFailureMessage = "tree query failed."
        )
        return when (contentResult) {
            is DomainResult.Failure -> TreeQueryResult(
                ok = false,
                found = false,
                message = contentResult.error.legacyMessage(),
                operationId = contentResult.error.operationId
            )

            is DomainResult.Success -> {
                val parsedPayload = parseTreeQueryContent(contentResult.value)
                    ?: return TreeQueryResult(
                        ok = false,
                        found = false,
                        message = appendFailureContext(
                            message = "tree query returned invalid payload.",
                            operationId = queryResult.operationId
                        ),
                        operationId = queryResult.operationId
                    )
                if (!parsedPayload.ok) {
                    TreeQueryResult(
                        ok = false,
                        found = parsedPayload.found,
                        roots = parsedPayload.roots,
                        nodes = parsedPayload.nodes,
                        message = parsedPayload.errorMessage.ifBlank { "tree query failed." },
                        operationId = queryResult.operationId
                    )
                } else {
                    TreeQueryResult(
                        ok = true,
                        found = parsedPayload.found,
                        roots = parsedPayload.roots,
                        nodes = parsedPayload.nodes,
                        message = buildTreeResultMessage(
                            found = parsedPayload.found,
                            roots = parsedPayload.roots,
                            nodes = parsedPayload.nodes
                        ),
                        operationId = queryResult.operationId
                    )
                }
            }
        }
    }

    fun toContentResult(
        queryResult: NativeCallResult,
        defaultFailureMessage: String
    ): DomainResult<String> {
        if (!queryResult.initialized) {
            return DomainResult.Failure(
                CoreError(
                    userMessage = extractInitFailureMessage(queryResult.rawResponse),
                    debugMessage = queryResult.rawResponse,
                    errorLogPath = queryResult.errorLogPath,
                    operationId = queryResult.operationId
                )
            )
        }

        val payload = responseCodec.parse(queryResult.rawResponse)
        if (!payload.ok) {
            return DomainResult.Failure(
                CoreError(
                    userMessage = payload.errorMessage.ifEmpty { defaultFailureMessage },
                    debugMessage = queryResult.rawResponse,
                    errorLogPath = queryResult.errorLogPath,
                    operationId = queryResult.operationId
                )
            )
        }

        return DomainResult.Success(payload.content)
    }

    fun extractInitFailureMessage(rawResponse: String): String {
        return responseCodec.parse(rawResponse).errorMessage.ifEmpty { "native init failed." }
    }
}
