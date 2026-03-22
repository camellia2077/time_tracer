use clap::{ArgAction, Args};

#[derive(Debug, Args)]
pub struct DoctorArgs {
    #[arg(long = "json", action = ArgAction::SetTrue, help = "Reserved: emit machine-readable diagnostics")]
    pub json: bool,
}
