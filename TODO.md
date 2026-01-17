validate-source
调用验证txt

validate-output是
和先调用converter转化为strcut
再调用serializer把strcut转化为json
再调用json检验

和ai讨论一下更新架构后，应该怎么处理跨月，跨年的sleep生成
目标设计成，不在txt中存储上一个月的最后一天就能处理跨年，跨月

所有log_generator也要更新，不再往当前月生成上一个月的最后一天

validate_output_command修改为validate_json_logic_command或者validate_logic_command
validate_source_command修改为validate_content_structure或者validate_structure

python测试程序需要有一个一步一步插入的模式validate->convert->import

cpp要增加一个插入并且导出全部日期全部格式的命令

在core中提取时间统计输出格式，复用代码

无论是blink还是run-pipeline都应该实现插入全流程，用哪一个效果都是一样的，只不过这个插入全流程有个两个别名

导出的时候也要统计时间，成功: 共创建 1827 个日报文件，已保存至: C:\Computer\my_github\github_cpp\time_tracer\my_test\output\exported_files\Typst_logs\days



高优先级

1
日报告的typ中 出现\正常吗
+ *Remark:* 没有背叛小到可以容忍 \
  没有背叛小到可以容忍 \
  minecraft真好玩 \
  皇室战争真好玩






1reports模块把数据获取和格式化解耦
让格式化不再设计数据库操作
shared 数据库代码，daily = 纯业务逻辑 + 报告生成


2导出的日报告的时候，没用按照年份文件夹，月份文件夹来分类

3
自定义标题的输出顺序
reports文件夹中json自定义格式化输出的标题，输出为true ,不输出为false

5 constexpr std::string_view LAST_UPDATED 使用编译器自动确定编译时间，而不是手动输入
gemini "CMake 自动编译日期实现"


6
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


中优先级

-2 新建创建一个独立的 analytics（或 query）模块

模块一：基础检索与回顾 (Basic Search & Review)
1. 全局备注搜索
目标：快速找到“那一天”或“那一件事”。
应用场景: "我哪天在备注里提到了 'minecraft'？" 或 "查找包含 '#我是备注1234' 的所有活动。"

2. 专注块过滤器
功能描述: 查找单次时长超过 N 小时的活动记录。

应用场景: "上个月我有多少次超过 3 小时的连续 '数据结构' 学习？"

3. 特定项目历史回顾 (Project History)
功能描述: 列出某个项目（如“高等数学”）在指定时间段内的所有记录。

应用场景: "我想看这周所有的英语文章阅读记录。"


模块二：习惯与趋势分析 (Habits & Trends)

4. 作息规律分析 (Sleep & Wake-up Trends)

功能描述: 绘制起床时间 (getup_time) 和入睡时间曲线。

应用场景: "最近是否越来越晚睡了？" "我有几天是 6:30 之前起床的？"

技术点: days.getup_time 是字符串，需要转化为分钟数以便绘图或计算平均值。

5. 熬夜/健康预警 (Health Alerts)

功能描述: 查找睡眠不足或作息异常的日子。

应用场景: "列出所有睡眠总时间 (sleep_total_time) 小于 6 小时的日子。" (你的需求之一)

6. 娱乐/学习比率 (Life Balance Ratio)

功能描述: 计算某段时间内“学习时长”与“娱乐时长”的比例。

应用场景: "这个月我的学习时间是否超过了刷抖音+B站的时间？"

模块三：连胜与打卡 (Streaks & Heatmaps)
目标：游戏化你的时间记录，提供正向反馈。

7. 连续打卡统计 (Streaks)

功能描述: 计算连续多少天进行了某项活动（如“英语文章”或“背单词”）。

应用场景: "我已经连续学了多少天英语了？"

技术点: 这是一个经典的 SQL "Gaps and Islands" 问题，需要在 C++ 层处理逻辑，查询所有包含该项目 ID 的日期并计算连续性。

8. 年度热力图数据 (GitHub-style Heatmap)

功能描述: 获取一年中每一天的特定指标强度。

应用场景: 生成类似 GitHub 贡献图的“学习热力图”或“运动热力图”。

模块四：关联洞察 (Correlation Insights)
目标：发现数据背后的隐性逻辑。

9. "玩物丧志"分析 (Correlation Analysis)

功能描述: 分析某项娱乐活动时长与当天学习时长的相关性。

应用场景: "是不是我白天打《守望先锋》的时间越长，晚上的学习效率越低？"

技术点: 查询 gaming_time 和 study_time 的数据对，计算相关系数（可以在 C++ 中计算）。

10. 关键词画像 (Tag Profiling)

功能描述: 基于备注标签统计时间。

应用场景: 你的文本中有 #我是备注1234 这种 Tag。可以统计带有特定 Tag 的任务总耗时。

技术点: 需要解析 activity_remark 字段中的 # 标签。




关注点: 逻辑判断、数据过滤、统计聚合、趋势发现。

输出: 数据实体对象 (vector<TimeRecord>, vector<DayEntry>) 或 统计数值。


0
cli使用非模板库

-1
import的时候要检查是否为所需的 json文件，以免用户import了txt文件

0
支持txt增量插入，存储已经存入sqlite的txt的哈希值，如果哈希值改变，则说明txt有改动，则需要重新插入
这个功能应该只在全流程命令下实现？

7
使用字体的时候注意，可变字体必须是当前字体支持的语言
你用新罗马那肯定不能显示中文字体
用宋体也是不支持西文字体可变
用新罗马才支持西文字体可变

tex无法编译是因为不支持当前的可变字体，tex内容由加粗
解决方法就是代码中取消注释，手动指定字体的目录，但是这样只能编译，无法正确渲染字体加粗


8
支持某个关键词的查询？不知道有没有实现





低优先级

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
感觉没什么必要，程序主要是在生成报告的时候使用次数多
转换和插入慢一点就也没什么，反正一个人一天最多也就几百行
converter里面json使用yyjson
inserter里面json插入使用yyjson
yyjson在pacman没有包，你需要手动放入程序目录






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


