
## queries复用代码
##namespace {
    // 用于转义 TeX 特殊字符的局部辅助函数
    std::string escape_tex_local(const std::string& s) {
        std::string escaped;
        escaped.reserve(s.length());
        for (char c : s) {
            if (c == '&' || c == '%' || c == '$' || c == '#' || c == '_' || c == '{' || c == '}') {
                escaped += '\\';
            }
            escaped += c;
        }
        return escaped;
    }
}
复用这个函数

## 活动链接符号如果用emoji
typst不能正确转换为pdf，但是在vscode预览的时候可以
latex可以正确打印emoji


## tex日报告增加行间距配置
typ日报已经增加行间距




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
