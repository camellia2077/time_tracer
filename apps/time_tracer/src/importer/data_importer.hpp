// importer/data_importer.hpp

#ifndef IMPORTER_DATA_IMPORTER_H_
#define IMPORTER_DATA_IMPORTER_H_

#include <map>
#include <string>
#include <utility>  // for std::pair
#include <vector>

struct DailyLog;

void handle_process_memory_data(
    const std::string& db_name,
    const std::map<std::string, std::vector<DailyLog>>& data_map);

#endif  // IMPORTER_DATA_IMPORTER_H_