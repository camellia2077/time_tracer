use clap::{Args, ValueEnum};

#[derive(Debug, Clone, ValueEnum)]
pub enum CryptoAction {
    Encrypt,
    Decrypt,
    Inspect,
}

#[derive(Debug, Clone, ValueEnum)]
pub enum SecurityLevel {
    Min,
    Interactive,
    Moderate,
    #[value(alias = "sensitive")]
    High,
    Max,
}

#[derive(Debug, Args)]
pub struct CryptoArgs {
    #[arg(value_enum)]
    pub action: CryptoAction,
    #[arg(long = "in", value_name = "PATH")]
    pub input: String,
    #[arg(
        long = "security-level",
        value_enum,
        help = "Encryption KDF profile (encrypt only): min|interactive|moderate|high|max (alias: sensitive)"
    )]
    pub security_level: Option<SecurityLevel>,
}
