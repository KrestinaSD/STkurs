/**
* @file userbase.h
* @author Крестина С.Д.
* @version 1.0
* @brief Заголовочный файл для модуля userbase, отвечающий за работу с базой клиентов
* @details Класс предоставляет функциональность для:
* - Загрузки данных из файла
* - Добавления новых пользователей
* - Сохранения изменений в файл
* - Поиска пользователей
*/
#pragma once

#include <map>
#include <string>
#include <fstream>
#include <exception>
#include <mutex>
#include <algorithm>
#include "programmerror.h"
using namespace std;
/** 
 * @brief Класс для работы с базой пользователей
 */
class DB
{
private:
    char sep = ' '; ///< Разделитель логина и пароля в файле
    std::string path_to_userbase; ///< Путь к файлу базы данных
    std::map<std::string, std::string> DataBaseP; ///< Контейнер с парами логин:пароль
    std::mutex db_mutex; ///< Мьютекс для потокобезопасности

    /**
     * @brief Загружает данные из файла в память
     * @throw server_error в случае ошибок чтения файла
     */
    void load_from_file();
    void save_to_file();

public:
    DB(std::string DBName);
    
    const std::map<std::string, std::string>& get_data() const; // Для чтения
	std::map<std::string, std::string>& get_data(); // Для модификации
	
	// Метод для проверки существования пользователя
    bool user_exists(const std::string& login) const;
    
    void add_user(const std::string& login, const std::string& password);
    std::string get_password(const std::string& login) const;
};
