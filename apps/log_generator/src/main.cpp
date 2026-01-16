// main.cpp
#include "core/Application.hpp"

int main(int argc, char* argv[]) {
    // 实例化 Core 模块中的 Application 类
    Core::Application app;
    
    // 将控制权移交给 Application
    return app.run(argc, argv);
}