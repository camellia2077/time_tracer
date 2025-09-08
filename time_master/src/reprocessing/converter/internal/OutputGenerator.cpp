// reprocessing/converter/internal/OutputGenerator.cpp
#include "OutputGenerator.hpp"
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

/*
 * [开发者备注] 关于JSON键的排序行为:
 *
 * 1. 排序规则:
 * 此文件使用 nlohmann::json 的默认类型，它会按照键(key)的字母顺序对JSON对象进行排序。
 * 因此，最终输出文件中键的顺序（例如 "Activities" 会在 "Headers" 之前）是由它们的字母顺序决定的，
 * 而不是由它们在下方代码中的创建或赋值顺序决定的。任何依赖于键顺序的下游程序都应被视为不稳定的。
 *
 * 2. 选型考量 (字母排序 vs. 插入顺序):
 * - 性能: 曾考虑使用 nlohmann::ordered_json 来维持插入顺序（即 Headers 先于 Activities），
 * 但这会带来一定的性能开销。如果使用插入顺序，在处理键非常多（成千上万个）的JSON对象时，
 * 其查找操作的性能会略低于默认的 nlohmann::json。
 * - 文件用途: 当前生成的JSON文件是一个中间数据格式，其主要消费者是后续的数据库入库程序，而不是人类。
 * 对于偶尔的人工检查，即便 Activities 先于 Headers，通过简单的鼠标滚动也能轻松定位，可读性影响有限。
 * - 最终输出: 当需要查看数据时，应从数据库中导出并格式化。在那个环节，可以轻松地将 Headers 置于 Activities 之前，
 * 以获得最佳的可读性。
 *
 * 结论:
 * 综合考虑，我们选择遵循更标准、性能更优的默认字母排序行为。
 */
using json = nlohmann::json;

void OutputGenerator::write(std::ostream& outputStream, const std::vector<InputData>& days, const ConverterConfig& config) {
    if (days.empty()) {
        outputStream << "[]" << std::endl;
        return;
    }

    json root_array = json::array();

    for (const auto& day : days) {
        if (day.date.empty()) continue;

        json day_obj;

        json headers_obj;
        headers_obj["Date"] = day.date;
        headers_obj["Status"] = day.hasStudyActivity;
        headers_obj["Sleep"] = day.isContinuation ? false : day.endsWithSleepNight;
        headers_obj["Getup"] = day.isContinuation ? "Null" : (day.getupTime.empty() ? "00:00" : day.getupTime);
        
        if (!day.generalRemarks.empty()) {
            headers_obj["Remark"] = day.generalRemarks[0];
        } else {
            headers_obj["Remark"] = "";
        }
        
        day_obj["Headers"] = headers_obj;
        
        json activities = json::array();
        for (const auto& activity_data : day.processedActivities) {
            json activity_obj;
            
            activity_obj["startTime"] = activity_data.startTime;
            activity_obj["endTime"] = activity_data.endTime;

            json activity_details;
            activity_details["top_parents"] = activity_data.title;
            if (!activity_data.parents.empty()) {
                activity_details["parents"] = activity_data.parents;
            }
            
            activity_obj["activity"] = activity_details;
            activities.push_back(activity_obj);
        }
        day_obj["Activities"] = activities;

        root_array.push_back(day_obj);
    }

    outputStream << root_array.dump(4) << std::endl;
}