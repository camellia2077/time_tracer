tex文件无法编译，看看tex的结构有什么问题


写文档，mapping_config.json只支持替换同义词。

如何把检验dll这些的结果，用符合编程规范的方式输出，以便以后编写gui的时候也能后台看见
检验dll是否缺失的环节，是否可以默认不打印，出问题才打印？
例如这些内容
正在初始化文件控制器...
主应用配置加载成功。
正在验证预处理配置文件...
[Validator] All preprocessing configuration data is valid.
正在验证查询配置文件...
[Validator] All query configuration files are valid.
正在验证插件...
[Validator] Validating required plugins in: C:\Base1\my_program\my_time_master\tm\plugins
[Validator] -- Found required plugin: DayMdFormatter
[Validator] -- Found required plugin: DayTexFormatter
[Validator] -- Found required plugin: DayTypFormatter
[Validator] -- Found required plugin: MonthMdFormatter
[Validator] -- Found required plugin: MonthTexFormatter
[Validator] -- Found required plugin: MonthTypFormatter
[Validator] -- Found required plugin: PeriodMdFormatter
[Validator] -- Found required plugin: PeriodTexFormatter
[Validator] -- Found required plugin: PeriodTypFormatter
[Validator] All required plugins were found.
[Validator] Validating required plugins in: C:\Base1\my_program\my_time_master\tm
[Validator] -- Found required plugin: reports_shared
[Validator] All required plugins were found.


interval_processor_config.json不要和mapping_config.json搞混了。


写文档，说明怎么用
输入数据
1先对txt进行数据验证
    time_tracker_cli.exe validate-source c:\Base1\my_program\my_time_master\data\2025\2025_12.txt

2 convert(json)
3 import

导出
time_master_cli export daily 20240101 -o /你的/自定义/路径


修改，检查每个月时间是否连续，完整的功能放到json中，同时支持命令行，命令行优先级＞json配置
命令行增加是否生成中间数据json文件，json配置中增加这个功能


"根据提供的代码实现，目前 run-pipeline 命令无法通过命令行参数直接关闭日期检查。"


import的时候要检查是否为所需的 json文件，以免用户import了txt文件


根据你提供的代码逻辑，目前的程序设计不支持通过一个命令一次性导出多种格式（例如同时导出 typ 和 md）。
比如日查询的时候。


看一下每个活动的解析，存储，以及数据库中的数据结构有没有可以优化的地方




支持某个关键词的查询

支持txt多行注释(c++风格和python风格)

给txt文本记录要求写上规则(文档)


StringUtils.hpp不应该放到common文件夹中，应该放到reports文件夹中，高内聚原则

libreports_shared.dll在测试程序中被重复地复制了两次，一个放到和exe同程序目录，一个放到了plugins文件夹中
作为程序的核心，libreports_shared.dll只应该和exe同程序目录，不应该放入plugins文件夹


## 让tex支持生成的字体从本地获取
\setCJKmainfont[Path=C:/]{NotoSerifSC-VariableFont_wght.ttf}

还没有给json验证模块验证total_exercise_time










## tex格式增加更细致的字体控制
typ全格式已经实现




## 每天的拉屎和洗漱时间应该在日报告导出的时候添加

## 给编译程序加上fonts文件夹
存放免费开源的字体


## clang开启连接时优化需要下载对应工具
pacman -S mingw-w64-ucrt-x86_64-lld


## 格式化输出，根据类型输出不同的颜色背景。
没有任何一个实现


## json中的存入数据库但是还没使用的键

                "endTimestamp": 1735699680,
                "startTimestamp": 1735693800

            "anaerobicTime": 8220,// 已经在日报告中实现
            "cardioTime": 0,// 已经在日报告中实现
            "gamingTime": 12540,
            "groomingTime": 0,// 已经在日报告中实现
            "sleepDayTime": 8460,
            "sleepNightTime": 19380,
            "sleepTotalTime": 27840, // 已经在日报告中实现
            "toiletTime": 0,
            "totalExerciseTime": 16140




## 低优先级
### dll放入plugs文件夹再读取

### 标记语言编译python的配置使用toml

### 输出报告的hpp使用外置的json配置文件来传入字体
### 把log测试程序生成部分封装成python库，用pybind

