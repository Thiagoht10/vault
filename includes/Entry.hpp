#ifndef ENTRY_HPP
#define ENTRY_HPP

#include <string>
#include <iostream>

class Entry
{
private:
    std::string _service;
    std::string _username;
    std::string _password;

public:
    Entry(void);
    Entry(std::string svc, std::string usr, std::string pass);
    Entry(const Entry& other);
    Entry&  operator=(const Entry& other);
    ~Entry();

    void    setService(const std::string& service);
    void    setUsername(const std::string& login);
    void    setPassword(const std::string& login);

    const std::string& getService(void) const;
    const std::string& getUsername(void) const;
    const std::string& getPassword(void) const;

    void    print(void) const;
};

#endif