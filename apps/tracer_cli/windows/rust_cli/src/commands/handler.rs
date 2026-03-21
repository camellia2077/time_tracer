use crate::error::AppError;

#[derive(Debug, Clone, Default)]
pub struct CommandContext {
    pub db_path: Option<String>,
    pub output_path: Option<String>,
}

impl CommandContext {
    pub fn without_output(&self) -> Self {
        Self {
            db_path: self.db_path.clone(),
            output_path: None,
        }
    }
}

pub trait CommandHandler<A> {
    fn handle(&self, args: A, ctx: &CommandContext) -> Result<(), AppError>;
}
