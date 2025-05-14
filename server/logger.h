/**
* @file logger.h
* @author Крестина С.Д.
* @version 1.0
* @brief Заголовочный файл для модуля logger, отвечающий за запись логов
*/

#pragma once
#include <string>
#include <chrono>
#include <string>
#include <time.h>
#include <fstream>
#include "interface.h"
#include "programmerror.h"

class Logger
{
private:
    std::string path_to_logfile; ///< путь до файла с журналом работы
public:
	int writelog(std::string s);
    
    void set_path(std::string path_file);
    
    Logger(){};
    
    Logger(std::string s){path_to_logfile = s;};
    
    static std::string getDateTime();
    
};



