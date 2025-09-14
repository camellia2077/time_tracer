## 结构
本模块分为验证和转换，验证是用来验证txt的内容是否合。合法后可以转换为json

txt是要被转换为json的，因为json作为中间数据方便解析。而且json中键的命名已经提供了足够多的信息。没有使用注释的必要。
即使是人也能一眼看懂。因此选择了json作为存储中间数据的库。

## 验证

## 转换

### 根据字段来生成在generatedStats中的统计
**具体设计方案如下：**

1.  **定义规则 (Config a Rule)**: 在`ConverterConfig`的JSON配置文件中，我们定义一个名为 `generated_stats_rules` 的新数组。数组中的每个对象就是一个统计规则，它描述了：

      * `key_name`: 最终输出到 `generatedStats` JSON对象中的键名（例如 `"groomingTime"`）。
      * `topParent`: 必须匹配的 `topParent` 值。
      * `parents` (可选): 一个数组，活动中**至少要包含一个**这里列出的 `parent` 才会G被计入统计。如果此字段不存在，则只匹配 `topParent`。

2.  **动态存储 (Dynamic Storage)**: 修改 `InputData.hpp` 中的 `GeneratedStats` 结构体。不再使用固定的 `int` 成员（如 `groomingTime`, `toiletTime`），而是改用一个 `std::map<std::string, int>`。这样，我们可以动态地存储任意数量的统计数据。

3.  **规则处理器 (Rule Processor)**: 重构 `DayStatsCalculator` 类。它将接收 `ConverterConfig` 的引用，并根据 `generated_stats_rules` 中的规则来处理活动列表。它会遍历所有活动，再遍历所有规则，如果一个活动符合某个规则，就将其时长累加到 `GeneratedStats` 的map中对应的 `key_name` 上。

4.  **动态输出 (Dynamic Output)**: 修改 `OutputGenerator.cpp`，让它在生成 `generatedStats` 对象时，遍历 `GeneratedStats` 的map，而不是去访问固定的成员变量。
