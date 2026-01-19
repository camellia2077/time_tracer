// importer/data_importer.hpp

#ifndef IMPORTER_DATA_IMPORTER_HPP_
#define IMPORTER_DATA_IMPORTER_HPP_

#include <string>
#include <vector>
#include <map>
#include <utility> // for std::pair

struct DailyLog; 

void handle_process_memory_data(const std::string& db_name, const std::map<std::string, std::vector<DailyLog>>& data_map);

#endif // IMPORTER_DATA_IMPORTER_HPP_