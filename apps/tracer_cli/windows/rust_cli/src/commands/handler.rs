use crate::error::AppError;

#[derive(Debug, Clone, Default)]
pub struct CommandContext {
    pub db_path: Option<String>,
    pub output_path: Option<String>,
}

pub trait CommandHandler<A> {
    fn handle(&self, args: A, ctx: &CommandContext) -> Result<(), AppError>;
}
