// time_master_app/LogProcessorMenu.h
#ifndef LOG_PROCESSOR_MENU_H
#define LOG_PROCESSOR_MENU_H

class ActionHandler; // 前向声明

class LogProcessorMenu {
public:
    // 构造函数接收一个 ActionHandler 指针以执行操作
    explicit LogProcessorMenu(ActionHandler* action_handler);
    
    // 运行子菜单循环
    void run();

private:
    ActionHandler* action_handler_; // 指向主 ActionHandler 的指针

    void print_submenu() const;
    void handle_choice(int choice);
};

#endif // LOG_PROCESSOR_MENU_H