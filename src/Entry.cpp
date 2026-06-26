#include "Entry.hpp"
#include "SecureMemory.hpp"

Entry::Entry(void) {}

Entry::Entry(const std::string& svc, const std::string& usr,
    const std::string& pass)
    :_service(svc), _username(usr)
{
    _password.assign(pass);
}

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
    secureErase(_service);
    _service = service;
}

void    Entry::setUsername(const std::string& login)
{
    secureErase(_username);
    _username = login;
}

void    Entry::setPassword(const std::string& password)
{
    _password.assign(password);
}

void    Entry::setService(const unsigned char *service, std::size_t length)
{
    secureErase(_service);
    _service.assign(reinterpret_cast<const char*>(service), length);
}

void    Entry::setUsername(const unsigned char *username, std::size_t length)
{
    secureErase(_username);
    _username.assign(reinterpret_cast<const char*>(username), length);
}

/* void    Entry::setPassword(const char *password, std::size_t length)
{
    _password.assign(reinterpret_cast<const unsigned char*>(password), length);
} */

void    Entry::setPassword(const unsigned char *password, std::size_t length)
{
    _password.assign(password, length);
}

const std::string& Entry::getService(void) const
{
    return (_service);
}

const std::string& Entry::getUsername(void) const
{
    return (_username);
}

const unsigned char* Entry::getPassword(void) const
{
    return _password.data();
}

void    Entry::eraseField(void)
{
    _password.erase();
    secureErase(_username);
    secureErase(_service);
}

void    Entry::print(void) const
{
    std::cout << "Service: " << _service << std::endl;
    std::cout << "Username: " << _username << std::endl;
    std::cout << "Password: " << _password.c_data() << std::endl;
}

size_t  Entry::getPasswordSize(void) const
{
    return _password.size();
}
