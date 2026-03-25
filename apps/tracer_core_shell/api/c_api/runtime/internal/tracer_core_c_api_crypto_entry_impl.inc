template <typename Runner>
auto ExecuteCryptoTextEndpoint(std::string_view unexpected_message,
                               Runner&& runner) -> const char* {
  try {
    ClearLastError();
    return BuildTextResponse(runner());
  } catch (const std::exception& error) {
    return BuildFailureResponse(error.what());
  } catch (...) {
    return BuildFailureResponse(std::string(unexpected_message));
  }
}
