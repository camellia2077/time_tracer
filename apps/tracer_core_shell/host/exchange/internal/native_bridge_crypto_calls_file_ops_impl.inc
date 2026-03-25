auto RunEncryptFileOperation(JNIEnv* env, const EncryptFileArgs& args)
    -> file_crypto::FileCryptoResult {
  auto options = BuildCryptoOptions(env);
  options.security_level = args.security_level;
  return file_crypto::EncryptFile(args.input_path, args.output_path,
                                  args.passphrase, options);
}

auto RunDecryptFileOperation(JNIEnv* env, const DecryptFileArgs& args)
    -> file_crypto::FileCryptoResult {
  return file_crypto::DecryptFile(args.input_path, args.output_path,
                                  args.passphrase, BuildCryptoOptions(env));
}
