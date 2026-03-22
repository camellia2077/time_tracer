use std::env;
use std::time::Duration;

pub(crate) const ENV_CORE_LOG: &str = "TRACER_CLI_CORE_LOG";
pub(crate) const ENV_LOG_TIMESTAMP: &str = "TRACER_CLI_LOG_TIMESTAMP";
pub(crate) const ENV_TIMING: &str = "TRACER_CLI_TIMING";
pub(crate) const ENV_CRYPTO_PROGRESS: &str = "TRACER_CLI_CRYPTO_PROGRESS";

pub(crate) fn env_flag(name: &str, default: bool) -> bool {
    match env::var(name) {
        Ok(value) => {
            let normalized = value.trim().to_ascii_lowercase();
            match normalized.as_str() {
                "1" | "true" | "yes" | "on" => true,
                "0" | "false" | "no" | "off" => false,
                _ => default,
            }
        }
        Err(_) => default,
    }
}

pub(crate) fn log_timing(label: &str, elapsed: Duration) {
    if !env_flag(ENV_TIMING, false) {
        return;
    }
    eprintln!("[timing] {label}: {:.3} ms", elapsed.as_secs_f64() * 1000.0);
}
