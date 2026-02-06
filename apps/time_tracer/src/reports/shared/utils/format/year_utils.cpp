// reports/shared/utils/format/year_utils.cpp
#include "reports/shared/utils/format/year_utils.hpp"

#include <cctype>
#include <iomanip>
#include <sstream>

auto ParseGregorianYear(std::string_view input, int& gregorian_year) -> bool {
  constexpr int kYearLength = 4;
  constexpr int kDecimalBase = 10;
  if (input.size() != static_cast<size_t>(kYearLength)) {
    return false;
  }
  int value = 0;
  for (char digit_char : input) {
    if (std::isdigit(static_cast<unsigned char>(digit_char)) == 0) {
      return false;
    }
    value = (value * kDecimalBase) + (digit_char - '0');
  }
  if (value <= 0) {
    return false;
  }
  gregorian_year = value;
  return true;
}

auto FormatGregorianYear(int gregorian_year) -> std::string {
  std::ostringstream oss;
  oss << std::setw(4) << std::setfill('0') << gregorian_year;
  return oss.str();
}
