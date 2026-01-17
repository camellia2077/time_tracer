from .colors import AnsiColors

def print_header(message):
    print(f"{AnsiColors.HEADER}{AnsiColors.BOLD}--- {message} ---{AnsiColors.ENDC}")

def print_warning(message):
    print(f"{AnsiColors.WARNING}{message}{AnsiColors.ENDC}")

def print_error(message):
    print(f"{AnsiColors.FAIL}{message}{AnsiColors.ENDC}")

def print_success(message):
    print(f"{AnsiColors.OKGREEN}{message}{AnsiColors.ENDC}")