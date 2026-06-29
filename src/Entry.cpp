#include "Entry.hpp"

Entry::Entry(void) {}

Entry::Entry(const std::string& svc, const std::string& usr,
    const std::string& pass)
{
    _service.assign(svc);
    _username.assign(usr);
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

void    Entry::setService(const unsigned char* service)
{
    size_t i = 0;

    while(service[i])
        i++;
    _service.erase();
    setService(service, i);
}

void    Entry::setUsername(const unsigned char* login)
{
    size_t i = 0;

    while(login[i])
        i++;
    _username.erase();
    setUsername(login, i);
}

void    Entry::setService(const unsigned char *service, std::size_t length)
{
    _service.erase();
    _service.assign(service, length);
}

void    Entry::setUsername(const unsigned char *username, std::size_t length)
{
    _username.erase();
    _username.assign(username, length);
}

void    Entry::setPassword(const unsigned char *password, std::size_t length)
{
    _password.assign(password, length);
}

const unsigned char* Entry::getService(void) const
{
    return _service.data();
}

const unsigned char* Entry::getUsername(void) const
{
    return _username.data();
}

const unsigned char* Entry::getPassword(void) const
{
    return _password.data();
}

void    Entry::eraseField(void)
{
    _service.erase();
    _username.erase();
    _password.erase();
}

void    Entry::print(void) const
{
    std::cout << "Service: " << _service.data() << std::endl;
    std::cout << "Username: " << _username.data() << std::endl;
    std::cout << "Password: " << _password.data() << std::endl;
}

size_t  Entry::getPasswordSize(void) const
{
    return _password.size();
}

size_t  Entry::getServiceSize(void) const
{
    return _service.size();
}

size_t  Entry::getUserNameSize(void) const
{
    return _username.size();
}
