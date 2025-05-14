#include "filelogger.h"
#include "logger.h" 
#include <iostream>


FileLogger::FileLogger(std::string k) : path_mes(k) {
    logFile.open(path_mes, std::ios_base::app);
    if (!logFile.is_open()) {
        throw server_error("Open error file-log for message");
    }
}

FileLogger::~FileLogger() {
 	std::lock_guard<std::mutex> lock(fileMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
}

void FileLogger::write(const std::string& message) {
    if (!logFile.is_open()) {
        logFile.open(path_mes, std::ios::app);
        if (!logFile.is_open()) {
            throw server_error("Файл закрыт и не может быть открыт: " + path_mes);
        }
    }
    logFile << message << std::endl;
    logFile.flush();
}

void FileLogger::flush() {
    std::lock_guard<std::mutex> lock(fileMutex);
    if (logFile.is_open()) {
        logFile.flush(); // Принудительная запись буфера на диск
    }
}

