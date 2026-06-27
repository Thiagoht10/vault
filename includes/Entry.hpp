#ifndef ENTRY_HPP
#define ENTRY_HPP

#include <string>
#include <iostream>
#include <sstream>
#include "SecureBuffer.hpp"
#include "SecureString.hpp"

class Entry
{
private:
    SecureString    _service;
    SecureString    _username;
    SecureBuffer    _password;

public:
    Entry(void);
    Entry(const std::string& svc, const std::string& usr,
        const std::string& pass);
    Entry(const Entry& other) = delete;
    Entry&  operator=(const Entry& other) = delete;
    Entry(Entry&& other) noexcept;
    Entry&  operator=(Entry&& other) noexcept;
    ~Entry();

    void    setService(const unsigned char* service);
    void    setUsername(const unsigned char* login);
    void    setPassword(const std::string& password);
    void    setService(const unsigned char *service, std::size_t length);
    void    setUsername(const unsigned char *username, std::size_t length);
    void    setPassword(const unsigned char *password, std::size_t length);

    const unsigned char*    getService(void) const;
    const unsigned char*    getUsername(void) const;
    const unsigned char*    getPassword(void) const;
    size_t                  getPasswordSize(void) const;
    size_t                  getServiceSize(void) const;
    size_t                  getUserNameSize(void) const;

    void    eraseField(void);
    void    print(void) const;
};

#endif
