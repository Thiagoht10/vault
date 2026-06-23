#include "Entry.hpp"

Entry::Entry(void)
    :_service("none"), _username("none"), _password("none") {}

Entry::Entry(std::string svc, std::string usr, std::string pass)
    :_service(svc), _username(usr), _password(pass) {}

Entry::Entry(const Entry& other)
    :_service(other._service), _username(other._username), _password(other._password) {}

Entry&  Entry::operator=(const Entry& other)
{
    if(this != &other)
    {
        _service = other._service;
        _username = other._username;
        _password = other._password;
    }
    return (*this);
}

Entry::~Entry() {}

void    Entry::setService(const std::string& service)
{
    _service = service;
}

void    Entry::setUsername(const std::string& login)
{
    _username = login;
}

void    Entry::setPassword(const std::string& password)
{
    _password = password;
}

const std::string& Entry::getService(void) const
{
    return (_service);
}

const std::string& Entry::getUsername(void) const
{
    return (_username);
}

const std::string& Entry::getPassword(void) const
{
    return (_password);
}

void    Entry::print(void) const
{
    std::cout << "Service: " << _service << std::endl;
    std::cout << "Username: " << _username << std::endl;
    std::cout << "Password: " << _password << std::endl;
}
