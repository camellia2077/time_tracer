from typing import List, Dict

# Difficulty weights for common Clang-Tidy checks (lower is easier)
CHECK_WEIGHTS = {
    "readability-identifier-naming": 1,
    "modernize-": 2,
    "readability-": 3,
    "performance-": 5,
    "cppcoreguidelines-": 8,
    "bugprone-": 10,
    "llvm-": 5,
    "google-": 5,
}

def calculate_priority_score(warnings: List[Dict], file_path: str) -> float:
    """Computes a 'difficulty' score. Lower is easier."""
    if not warnings:
        return 999.0
    
    # Base weight based on the "scariest" check in the task
    max_check_weight = 5.0 # Default
    for w in warnings:
        check = w["check"]
        for pattern, weight in CHECK_WEIGHTS.items():
            if check.startswith(pattern):
                max_check_weight = max(max_check_weight, weight)
                break
    
    # File factor: header files are riskier to refactor
    file_factor = 2.0 if any(file_path.endswith(ext) for ext in [".hpp", ".h", ".hxx"]) else 1.0
    
    # Secondary factor: number of warnings (slightly increases difficulty)
    count_factor = len(warnings) * 0.05
    
    return (max_check_weight * file_factor) + count_factor
