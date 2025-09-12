## 支持对活动使用// 或#进行备注
在预处理转换后的json中，活动新增键，用于存储备注的文字
如果没有备注的活动,使用null来

(已经实现md,typ的格式化输出)

## 给每年第一月第一天生成睡眠键的活动天，不进行json转换
因为这个天的唯一作用就是给第一月第一天生成睡眠时间

## 测试程序起床的关键词从
"wake_keywords": ["起床", "醒", "w", "wake","新的一天开始了A"],中选择

## 修改测试程序，增加通宵的模拟


## headers中增加是否运动的bool值
generatedStats中增加运动的总时间，秒

## 格式化输出，根据类型输出不同的颜色。
typst已经实现

## 格式化输出，根据类型输出不同的颜色背景。
没有任何一个实现




## InputParser.cpp中引用了 #include "reprocessing/validator/common/ValidatorUtils.hpp" 需要解耦

## json中的存入数据库但是还没使用的键

                "endTimestamp": 1735699680,
                "startTimestamp": 1735693800



## 低优先级
### dll放入plugs文件夹再读取

### 标记语言编译python的配置使用toml
### JsonDataParser拆分，之后JsonDataParser作为facade封装
### 输出报告的hpp使用外置的json配置文件来传入字体
