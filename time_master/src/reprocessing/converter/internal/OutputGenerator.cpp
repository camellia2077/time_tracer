// converter/internal/OutputGenerator.cpp
#include "OutputGenerator.hpp"
#include <fstream>

void OutputGenerator::write(std::ostream& outputStream, const InputData& day, const ConverterConfig& config) {
    if (day.date.empty()) return;
    for (const auto& header : config.getHeaderOrder()) {
        if (header == "Date:") {
            outputStream << "Date:" << day.date << "\n";
        } else if (header == "Status:") {
            outputStream << "Status:" << (day.hasStudyActivity ? "True" : "False") << "\n";
        } else if (header == "Sleep:") {
            bool sleepStatus = day.isContinuation ? false : day.endsWithSleepNight;
            outputStream << "Sleep:" << (sleepStatus ? "True" : "False") << "\n";
        } else if (header == "Getup:") {
            outputStream << "Getup:" << (day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime)) << "\n";
        } else if (header == "Remark:") {
            if (!day.generalRemarks.empty() && !day.remarksOutput.empty()) {
                outputStream << "Remark:" << day.remarksOutput[0] << "\n";
                for (size_t i = 1; i < day.remarksOutput.size(); ++i) {
                    outputStream << day.remarksOutput[i] << "\n";
                }
            } else {
                outputStream << "Remark:\n";
                for (const auto& remark_line : day.remarksOutput) {
                    outputStream << remark_line << "\n";
                }
            }
        }
    }
    outputStream << "\n";
}