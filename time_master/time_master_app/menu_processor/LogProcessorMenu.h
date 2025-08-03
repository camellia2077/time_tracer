// time_master_app/menu_processor/LogProcessorMenu.h
#ifndef LOG_PROCESSOR_MENU_H
#define LOG_PROCESSOR_MENU_H

class FileProcessingHandler; // [修改] 前向声明新的处理器

class LogProcessorMenu {
public:
    // [修改] 构造函数现在接收 FileProcessingHandler 指针
    explicit LogProcessorMenu(FileProcessingHandler* handler);
    
    void run();

private:
    // [修改] 成员变量更新
    FileProcessingHandler* file_processing_handler_;

    void print_submenu() const;
    void handle_choice(int choice);
};

#endif // LOG_PROCESSOR_MENU_H