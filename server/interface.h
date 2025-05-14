/**
* @file interface.h
* @author Крестина С.Д.
* @version 1.0
* @brief Заголовочный файл для модуля interface, отвечающий за разбор ПКС, и включение других модулей
*/
#pragma once
#include <string>
#include <optional>

class interface
{
private:
 	std::string ServerAddress = "127.0.0.1";///< Адрес сервера
    std::string DataBaseName = "bas.txt"; ///< Путь к файлу с базой данных
    std::string LogFileName = "log.txt"; ///< Путь к файлу для записи логов
    int Port = 33333; ///< Порт, на котором работает сервер
    void usage(const char* progName); ///< Вывод подсказки и останов
public:
	int Opts(int argc, char **argv);
    std::string getLogFileName() /// Возвращает путь к файлу для записи логов
    {
        return LogFileName;
    }   
};

