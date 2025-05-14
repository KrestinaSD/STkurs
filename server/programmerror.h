/**
* @file programmerror.h
* @author Крестина С.Д.
* @version 1.0
* @brief Заголовочный файл для модуля server_error, отвечающий за ошибки
*/
#pragma once

#include <exception>
#include <ctime>
#include <string>
#include <cmath>
#include <cstdlib>
#include <iostream>


class server_error: public std::invalid_argument {
	private:
	bool State = false; ///<Статус критичности ошибки
	public:
	
	explicit server_error (const std::string& what_arg, bool critical = false):
		std::invalid_argument(what_arg) {State = critical;}
	
	explicit server_error (const char* what_arg,  bool critical = false):
		std::invalid_argument(what_arg) {State = critical;}
	std::string getState() const {return State ? "Критическая" : "Штатная";} ///<Возвращает статус критичности ошибки
};
