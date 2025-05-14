/**
* @file filelogger.h
* @author Крестина С.Д.
* @version 1.0
* @brief Заголовочный файл для модуля filelogger, отвечающий за запись, полученных от клиента строк
*/
#pragma once
#include <string>
#include <fstream>
#include <mutex>
#include <memory>

class FileLogger{
	public:
		FileLogger(){};
    	FileLogger(std::string k);
		~FileLogger();
		void write(const std::string& message); // запись
		void flush(); 
	private:
		std::string path_to_logfile; ///< путь до файла с журналом работы
		std::ofstream logFile; //поток для записи в файл
		std::mutex fileMutex; // для защиты записи 
		std::string path_mes; ///< путь до файла с журналом работы
};
