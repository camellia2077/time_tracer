// time_master_app/menu_processor/LogProcessorMenu.hpp
#ifndef LOG_PROCESSOR_MENU_HPP
#define LOG_PROCESSOR_MENU_HPP

class FileProcessingHandler; // 前向声明新的处理器

class LogProcessorMenu {
public:
    explicit LogProcessorMenu(FileProcessingHandler* handler);
    
    void run();

private:
    FileProcessingHandler* file_processing_handler_;

    void print_submenu() const;
    void handle_choice(int choice);
};

#endif // LOG_PROCESSOR_MENU_HPP