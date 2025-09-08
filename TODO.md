
## 文本的headers内容都改为小写
## 把布尔类型修改为0或1存储到数据库中
JsonDataParser.cpp

    day_data.status = headers.value("Status", false) ? "True" : "False";
    day_data.sleep = headers.value("Sleep", false) ? "True" : "False";
