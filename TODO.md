config中的json文件夹命名，需要修改
reprocessing修改为converter,并且在io中需要修改，在config检验也需要更新


cli模块目录太乱


MemoryParser.cpp内parse函数承担了过多的职责

导出的日报告的时候，没用按照年份文件夹，月份文件夹来分类

把config配置检验从io中独立出来



md文档加入convert的特殊机制，即使没用输入指令，也会把转换后的json存储到本地
md文档说明db_insert现在是接受内存中的结构

md中写一下当前程序的设计思路，比如core只设计业务逻辑的协调，io只设计文件读写，预处理，插入，导出模块只涉及具体业务而没有参与文件读写

问一下cmake是如何把print库链接到某个库的
gcc和clang编译的dll为什么不一样



高优先级
0
把python的热力图相关代码看看还能不能用



2 对于日期检查，是否把预处理过程中生成的中间文件json保存到本地。类的命令，是不是可以先从json中读取，如果没有则查看命令行是否传入
让命令行优先级大于json优先级

3
自定义标题的输出顺序
reports文件夹中json自定义格式化输出的标题，输出为true ,不输出为false

4 看一下每个活动的解析，存储，以及数据库中的数据结构有没有可以优化的地方
    gemini对话"递归转迭代优化项目树格式化"
    
5 constexpr std::string_view LAST_UPDATED 使用编译器自动确定编译时间，而不是手动输入
gemini "CMake 自动编译日期实现"

6
看看cli的输入以及实现是否符合变成规范



中优先级
-4
因为现在blink或者run-pipeline程序会把json存储到内存中
但是python还是一步一步的测试，你需要给python加入一个blink模式
用toml来选择如何开启测试
-3
anction_handler命名修改为core

-1
import的时候要检查是否为所需的 json文件，以免用户import了txt文件

0
支持txt增量插入，存储已经存入sqlite的txt的哈希值，如果哈希值改变，则说明txt有改动，则需要重新插入
这个功能应该只在全流程命令下实现？

1 写文档，mapping_config.json只支持替换同义词。
interval_processor_config.json不要和mapping_config.json搞混了。
写文档，说明怎么用

2
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

3 
根据你提供的代码逻辑，目前的程序设计不支持通过一个命令一次性导出多种格式（例如同时导出 typ 和 md）。
比如日查询的时候。


4
修改，检查每个月时间是否连续，完整的功能放到json配置中，
同时支持命令行来修改，每个月时间是否连续，让全流程支持是否启动日期连续性检查
命令行优先级＞json配置

5
命令行增加是否生成中间数据json文件，json配置中增加这个功能
这个功能是是否把txt转换为json后，把json存储到本地

6
gemini "项目目录与json配置改进建议"


7
使用字体的时候注意，可变字体必须是当前字体支持的语言
你用新罗马那肯定不能显示中文字体
用宋体也是不支持西文字体可变
用新罗马才支持西文字体可变

tex无法编译是因为不支持当前的可变字体，tex内容由加粗
解决方法就是代码中取消注释，手动指定字体的目录，但是这样只能编译，无法正确渲染字体加粗



7
导出的时候
time_master_cli export daily 20240101 -o /你的/自定义/路径
不知道这个功能有没有实现


8
支持某个关键词的查询？不知道有没有实现





低优先级
gemini "主要模块剥离逻辑和定义 C 接口。"
0. 核心逻辑库 (Core Library) 的接口设计

不要局限于“现在就编译成 DLL”，而是应该关注 “核心逻辑库 (Core Library) 的接口设计”。

核心就是这两件事：主要模块剥离逻辑和定义 C 接口。

代码解耦（现在做）： 将 Data Preprocessing、DB Insertion、Export 三个部分的代码从 CLI 的 main 逻辑中剥离出来，放入独立的库项目（Library Project）中。

定义 C-API 边界（关键）： 为这三个模块设计一层纯 C 的头文件接口。无论内部是用 C++ 还是未来用 Rust 实现，对外都只暴露这一层 C 接口。

示例： 不要传递 std::string，而是传递 const char* 和长度。

构建系统配置（CMake/Meson）： 配置你的构建脚本，使其支持通过开关切换 静态链接 (Static Lib) 和 动态链接 (Shared Lib/DLL)。

CLI 阶段： 建议优先使用静态链接。这样生成的 EXE 单文件便于分发，且编译器能进行全局优化 (LTO)。

移动端/Rust 替换阶段： 基于之前定义的 C-API，你可以随时用 Rust 编写实现，并编译成对应平台的动态库（.dll/.so）或静态库（.a），供上层调用。

4. 移动端复用路径
如果采用上述架构，未来在手机上复用的路径如下：

Android (JNI): Java/Kotlin 层通过 JNI 调用你的 C/C++ (或 Rust) 编译出的 .so 库。

iOS (FFI): Swift/Obj-C 直接调用编译出的静态库 .a 或 Framework。

Flutter/React Native: 通过 FFI (Foreign Function Interface) 调用 C 接口。



1
tex文件无法编译，看看tex的结构有什么问题




未来
1 
第一步（必须做）： 将所有 std::cout/cerr 替换为 ILogger 回调。这能解决“其他平台看不见日志”的问题。

第二步（优化）： 将“业务结果”（如错误列表、耗时统计）作为函数的返回值返回，而不是在函数内部处理（打印）。让调用者决定如何展示这些数据。

这会使你的 reprocessing 模块变成一个纯粹的库（Library），可以被任何程序调用。


为了实现彻底的模块化并支持多平台（如手机 App、GUI、后台服务），核心业务逻辑模块（如 reprocessing 和 PipelineManager）绝对不能包含 std::cout、std::cerr 或 printf。

当前代码中的 std::cout 是硬编码的控制台输出，这在手机 App（没有控制台）或 GUI（需要弹窗或进度条）中是完全不可见的，甚至会导致崩溃或资源浪费。

如何解耦？（架构改造方案）
你需要将“发生什么（Logic）”与“如何通知用户（Presentation）”分离开。通常有两种互补的方法来实现这一点：

方法 1：引入抽象日志接口 (Dependency Injection)
这是最标准的方法。让 reprocessing 模块不依赖具体的 iostream，而是依赖一个抽象的接口。

2. 和sliqte解耦，采取注册模式?支持多种数据库
















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




### 输出报告的hpp使用外置的json配置文件来传入字体


