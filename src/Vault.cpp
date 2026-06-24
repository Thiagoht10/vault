#include "Vault.hpp"

#include <utility>

Vault::Vault(void) {}

void    Vault::addEntry(Entry&& entry)
{
    _entry.push_back(std::move(entry));
}

bool    Vault::removeEntry(size_t index)
{
    if(index >= _entry.size())
        return false;

    _entry[index].eraseField();
    _entry.erase(_entry.begin() + index);
    return true;
}

void    Vault::printAll(void) const
{
    size_t  size;

    size = _entry.size();
    if(size == 0)
    {
        std::cout << "\nNothing to print\n" << std::endl;
        return ;
    }
    
    std::cout << "\n";
    for (size_t i = 0; i < size; i++)
    {
        std::cout << "[" << i << "]" << std::endl;
        _entry[i].print();
        std::cout << "\n" << "------------------" << "\n" << std::endl;
    }
}

std::string Vault::serialize(void) const
{
    std::ostringstream  data;
    size_t              size;

    size = _entry.size();
    if(size == 0)
        return std::string();
    
    for (size_t i = 0; i < size; i++)
    {
        data << "service:" << _entry[i].getService() << "\n";
        data << "username:" << _entry[i].getUsername() << "\n";
        data << "password:" << _entry[i].getPassword() << "\n";
        data << "----------\n\n";
    }
    return data.str();
}

void    Vault::deserialize(std::string& data)
{
    std::istringstream  stream(data);
    std::string line;
    std::string service;
    std::string username;
    std::string password;

    _entry.clear();

    while (std::getline(stream, line))
    {
        if (line.find("service:") == 0)
            service = line.substr(7);
        else if (line.find("username:") == 0)
            username = line.substr(8);
        else if (line.find("password:") == 0)
            password = line.substr(8);
        else if (line.find("----------") == 0)
        {
            _entry.push_back(Entry(service, username, password));
            service.clear();
            username.clear();
            password.clear();
        }
    }
}
