#include "Entry.hpp"
#include "SecureMemory.hpp"

Entry::Entry(void) {}

Entry::Entry(const std::string& svc, const std::string& usr,
    const std::string& pass)
    :_service(svc), _username(usr), _password(pass) {}

Entry::Entry(Entry&& other) noexcept
{
    _service.swap(other._service);
    _username.swap(other._username);
    _password.swap(other._password);
}

Entry&  Entry::operator=(Entry&& other) noexcept
{
    if(this != &other)
    {
        eraseField();
        _service.swap(other._service);
        _username.swap(other._username);
        _password.swap(other._password);
    }
    return (*this);
}

Entry::~Entry()
{
    eraseField();
}

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

void    Entry::eraseField(void)
{
    secureErase(_password);
    secureErase(_username);
    secureErase(_service);
}

void    Entry::print(void) const
{
    std::cout << "Service: " << _service << std::endl;
    std::cout << "Username: " << _username << std::endl;
    std::cout << "Password: " << _password << std::endl;
}
