#include "userbase.h"
using namespace std;

DB::DB(std::string DBName) : path_to_userbase(DBName) {
    load_from_file();
}

// Константная версия (для чтения)
const std::map<std::string, std::string>& DB::get_data() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(db_mutex));
     return DataBaseP;
}

// Не константная версия (для модификации)
std::map<std::string, std::string>& DB::get_data() {
    std::lock_guard<std::mutex> lock(db_mutex);
    return DataBaseP;
}

void DB::load_from_file() {
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ifstream file(path_to_userbase);
    
    if(!file.good()) {
        throw server_error("Wrong DB File Name", true);
    }
    
    DataBaseP.clear();
    std::string Pair;
    
    while(std::getline(file, Pair)) {
        size_t sep_pos = Pair.find(sep);
        if(sep_pos != std::string::npos) {
            std::string login = Pair.substr(0, sep_pos);
            std::string password = Pair.substr(sep_pos + 1);
            DataBaseP[login] = password;
        }
    }
}

void DB::add_user(const std::string& login, const std::string& password) {
    if(login.empty() || password.empty()) {
        throw server_error("Login and password cannot be empty");
    }
    
    if(login.find(sep) != std::string::npos) {
        throw server_error("Login contains invalid character");
    }
    
     std::cout << "[DB] Попытка блокировки для добавления юзера мьютекса..." << std::endl;
    std::lock_guard<std::mutex> lock(db_mutex);
    std::cout << "[DB] Мьютекс заблокирован!" << std::endl;
    DataBaseP[login] = password;
    save_to_file();
}

void DB::save_to_file() {
     /*std::cout << "[DB] Попытка блокировки сохранения в файл мьютекса..." << std::endl;
    std::lock_guard<std::mutex> lock(db_mutex);
    std::cout << "[DB] Мьютекс заблокирован!" << std::endl;*/
    std::ofstream file(path_to_userbase);
    
    if(!file.is_open()) {
        throw server_error("Failed to open DB file for writing");
    }
    
    for(const auto& pair : DataBaseP) {
        file << pair.first << sep << pair.second << "\n";
    }
}

std::string DB::get_password(const std::string& login) const {
     std::cout << "[DB] Попытка блокировки для получения пароля мьютекса..." << std::endl;
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(db_mutex));
    std::cout << "[DB] Мьютекс заблокирован!" << std::endl;
    return DataBaseP.at(login);
}

bool DB::user_exists(const std::string& login) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(db_mutex));
    return DataBaseP.find(login) != DataBaseP.end();
}

