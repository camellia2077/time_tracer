pub mod handler;
mod handlers;

use crate::cli::{Cli, Command};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::commands::handlers::chart::ChartHandler;
use crate::commands::handlers::convert::ConvertHandler;
use crate::commands::handlers::crypto::CryptoHandler;
use crate::commands::handlers::doctor::DoctorHandler;
use crate::commands::handlers::export::ExportHandler;
use crate::commands::handlers::import::ImportHandler;
use crate::commands::handlers::ingest::IngestHandler;
use crate::commands::handlers::licenses::LicensesHandler;
use crate::commands::handlers::motto::MottoHandler;
use crate::commands::handlers::query::QueryHandler;
use crate::commands::handlers::tracer::TracerHandler;
use crate::commands::handlers::tree::TreeHandler;
use crate::commands::handlers::validate::ValidateHandler;
use crate::error::AppError;

pub fn execute(cli: Cli) -> Result<(), AppError> {
    let ctx = CommandContext {
        db_path: cli.db,
        output_path: cli.output,
    };

    match cli.command {
        Command::Query(args) => QueryHandler.handle(args, &ctx),
        Command::Chart(args) => ChartHandler.handle(args, &ctx),
        Command::Crypto(args) => CryptoHandler.handle(args, &ctx),
        Command::Export(args) => ExportHandler.handle(args, &ctx),
        Command::Convert(args) => ConvertHandler.handle(args, &ctx),
        Command::Import(args) => ImportHandler.handle(args, &ctx),
        Command::Ingest(args) => IngestHandler.handle(args, &ctx),
        Command::Validate(args) => ValidateHandler.handle(args, &ctx),
        Command::Tree(args) => TreeHandler.handle(args, &ctx),
        Command::Doctor(args) => DoctorHandler.handle(args, &ctx),
        Command::Licenses(args) => LicensesHandler.handle(args, &ctx),
        Command::Tracer => TracerHandler.handle((), &ctx),
        Command::Motto => MottoHandler.handle((), &ctx),
    }
}
