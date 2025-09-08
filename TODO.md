
## 每日报告加上时间线内容(md已经实现)
## 文本的headers内容都改为小写

## 解耦json转换，分成两个模块，一个模块是直接从txt获取的信息。另一个模块是转换成json后，根据json计算，得出的信息
争取做到查询的时候用到的信息，比如活动间隔的总时间，活动数量这些数据都提取在插入数据库的时候计算出来

## InputParser.cpp中引用了 #include "reprocessing/validator/common/ValidatorUtils.hpp" 需要解耦
