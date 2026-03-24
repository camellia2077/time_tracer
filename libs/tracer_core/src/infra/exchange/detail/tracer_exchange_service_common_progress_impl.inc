auto ToFileCryptoSecurityLevel(
    app_dto::TracerExchangeSecurityLevel security_level)
    -> file_crypto::FileCryptoSecurityLevel {
  switch (security_level) {
    case app_dto::TracerExchangeSecurityLevel::kMin:
      return file_crypto::FileCryptoSecurityLevel::kMin;
    case app_dto::TracerExchangeSecurityLevel::kInteractive:
      return file_crypto::FileCryptoSecurityLevel::kInteractive;
    case app_dto::TracerExchangeSecurityLevel::kModerate:
      return file_crypto::FileCryptoSecurityLevel::kModerate;
    case app_dto::TracerExchangeSecurityLevel::kHigh:
      return file_crypto::FileCryptoSecurityLevel::kHigh;
    case app_dto::TracerExchangeSecurityLevel::kMax:
      return file_crypto::FileCryptoSecurityLevel::kMax;
  }
  return file_crypto::FileCryptoSecurityLevel::kInteractive;
}

auto ToProgressPhaseString(file_crypto::FileCryptoPhase phase) -> std::string {
  switch (phase) {
    case file_crypto::FileCryptoPhase::kScan:
      return "scan";
    case file_crypto::FileCryptoPhase::kReadInput:
      return "read_input";
    case file_crypto::FileCryptoPhase::kCompress:
      return "compress";
    case file_crypto::FileCryptoPhase::kDeriveKey:
      return "derive_key";
    case file_crypto::FileCryptoPhase::kEncrypt:
      return "encrypt";
    case file_crypto::FileCryptoPhase::kDecrypt:
      return "decrypt";
    case file_crypto::FileCryptoPhase::kDecompress:
      return "decompress";
    case file_crypto::FileCryptoPhase::kWriteOutput:
      return "write_output";
    case file_crypto::FileCryptoPhase::kCompleted:
      return "completed";
    case file_crypto::FileCryptoPhase::kCancelled:
      return "cancelled";
    case file_crypto::FileCryptoPhase::kFailed:
      return "failed";
  }
  return "unknown";
}

auto ToTracerExchangeProgressSnapshot(
    const file_crypto::FileCryptoProgressSnapshot& snapshot)
    -> app_dto::TracerExchangeProgressSnapshot {
  return {
      .input_root_path = snapshot.input_root_path,
      .output_root_path = snapshot.output_root_path,
      .current_input_path = snapshot.current_input_path,
      .current_output_path = snapshot.current_output_path,
      .current_group_label = snapshot.current_group_label,
      .group_index = snapshot.group_index,
      .group_count = snapshot.group_count,
      .file_index_in_group = snapshot.file_index_in_group,
      .file_count_in_group = snapshot.file_count_in_group,
      .current_file_index = snapshot.current_file_index,
      .total_files = snapshot.total_files,
      .current_file_done_bytes = snapshot.current_file_done_bytes,
      .current_file_total_bytes = snapshot.current_file_total_bytes,
      .overall_done_bytes = snapshot.overall_done_bytes,
      .overall_total_bytes = snapshot.overall_total_bytes,
      .speed_bytes_per_sec = snapshot.speed_bytes_per_sec,
      .remaining_bytes = snapshot.remaining_bytes,
      .eta_seconds = snapshot.eta_seconds,
      .is_encrypt_operation =
          snapshot.operation == file_crypto::FileCryptoOperation::kEncrypt,
      .phase = ToProgressPhaseString(snapshot.phase),
  };
}
