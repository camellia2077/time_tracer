from build_config import config

def print_header(message):
    """打印带有标题格式的日志消息"""
    print(f"{config.HEADER}{config.BOLD}--- {message} ---{config.ENDC}")

def print_warning(message):
    """打印警告消息"""
    print(f"{config.WARNING}{message}{config.ENDC}")

def print_error(message):
    """打印错误消息"""
    print(f"{config.FAIL}{message}{config.ENDC}")

def print_success(message):
    """打印成功消息"""
    print(f"{config.OKGREEN}{message}{config.ENDC}")