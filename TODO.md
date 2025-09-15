## tex日报告增加行间距配置
typ日报已经增加行间距


## interval_processor_config.json拆分
因为映射行数太多
拆分完成之后，给generatedStats设置单独的配置json

每天的拉屎和洗漱时间应该在日报告导出的时候添加

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
