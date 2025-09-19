## 将 query_utils.cpp 和 query_utils.hpp 放在 queries/shared/utils/db 文件夹中是不太合适的。

原因如下：

功能不匹配：query_utils.hpp 和 query_utils.cpp 中的函数，add_days_to_date_str 和 get_current_date_str，是通用的日期计算辅助函数。它们的功能是处理日期字符串和获取当前日期，与数据库查询本身没有直接关联。将它们放在 db（数据库）文件夹中，会使项目结构产生误导，因为它暗示这些函数是专门用于数据库操作的。




## tex格式增加更细致的字体控制
typ全格式已经实现




## 每天的拉屎和洗漱时间应该在日报告导出的时候添加

## 给编译程序加上fonts文件夹
存放免费开源的字体


## clang开启连接时优化需要下载对应工具
pacman -S mingw-w64-ucrt-x86_64-lld


## 给routine_grooming                
增加在"generatedStats"的键和值，存储时间为秒数



## 活动颜色映射独立成一个hpp，这样tex和typst都可以读取，复用代码

## 格式化输出，根据类型输出不同的颜色背景。
没有任何一个实现




## InputParser.cpp中引用了 #include "reprocessing/validator/common/ValidatorUtils.hpp" 需要解耦

## json中的存入数据库但是还没使用的键

                "endTimestamp": 1735699680,
                "startTimestamp": 1735693800
anaerobicTime //无氧
cardioTime //有氧 
exerciseBothTime // both包含有氧和无氧的时间
totalExerciseTime //总时间




## 低优先级
### dll放入plugs文件夹再读取

### 标记语言编译python的配置使用toml
### JsonDataParser拆分，之后JsonDataParser作为facade封装
### 输出报告的hpp使用外置的json配置文件来传入字体
### 把log测试程序生成部分封装成python库，用pybind

