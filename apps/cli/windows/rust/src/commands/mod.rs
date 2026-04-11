pub mod handler;
mod handlers;
#[cfg(test)]
pub mod testing;

use crate::cli::{Cli, Command};
use crate::commands::handler::{CommandContext, CommandHandler};
use crate::commands::handlers::chart::ChartHandler;
use crate::commands::handlers::doctor::DoctorHandler;
use crate::commands::handlers::exchange::ExchangeHandler;
use crate::commands::handlers::licenses::LicensesHandler;
use crate::commands::handlers::motto::MottoHandler;
use crate::commands::handlers::pipeline::PipelineHandler;
use crate::commands::handlers::query::QueryHandler;
use crate::commands::handlers::report::ReportHandler;
use crate::commands::handlers::tracer::TracerHandler;
use crate::commands::handlers::txt::TxtHandler;
use crate::error::AppError;

pub fn execute(cli: Cli) -> Result<(), AppError> {
    let ctx = CommandContext {
        db_path: cli.db,
        output_path: cli.output,
    };

    match cli.command {
        Command::Query(args) => QueryHandler.handle(args, &ctx),
        Command::Chart(args) => ChartHandler.handle(args, &ctx),
        Command::Pipeline(args) => PipelineHandler.handle(args, &ctx),
        Command::Report(args) => ReportHandler.handle(args, &ctx),
        Command::Exchange(args) => ExchangeHandler.handle(args, &ctx),
        Command::Txt(args) => TxtHandler.handle(args, &ctx),
        Command::Doctor(args) => DoctorHandler.handle(args, &ctx),
        Command::Licenses(args) => LicensesHandler.handle(args, &ctx),
        Command::Tracer => TracerHandler.handle((), &ctx),
        Command::Motto => MottoHandler.handle((), &ctx),
    }
}
