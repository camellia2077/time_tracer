auto BuildCryptoOptions(
    app_dto::TracerExchangeSecurityLevel security_level,
    const app_dto::TracerExchangeProgressObserver& progress_observer)
    -> file_crypto::FileCryptoOptions {
  file_crypto::FileCryptoOptions options{};
  options.security_level = ToFileCryptoSecurityLevel(security_level);
  if (progress_observer) {
    options.progress_callback =
        [progress_observer](
            const file_crypto::FileCryptoProgressSnapshot& snapshot) {
          try {
            return progress_observer(
                       ToTracerExchangeProgressSnapshot(snapshot)) ==
                           app_dto::TracerExchangeProgressControl::kCancel
                       ? file_crypto::FileCryptoControl::kCancel
                       : file_crypto::FileCryptoControl::kContinue;
          } catch (...) {
            return file_crypto::FileCryptoControl::kContinue;
          }
        };
  }
  return options;
}
