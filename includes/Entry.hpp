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
    Entry(const std::string& svc, const std::string& usr,
        const std::string& pass);
    Entry(const Entry& other) = delete;
    Entry&  operator=(const Entry& other) = delete;
    Entry(Entry&& other) noexcept;
    Entry&  operator=(Entry&& other) noexcept;
    ~Entry();

    void    setService(const std::string& service);
    void    setUsername(const std::string& login);
    void    setPassword(const std::string& login);

    const std::string& getService(void) const;
    const std::string& getUsername(void) const;
    const std::string& getPassword(void) const;

    void    eraseField(void);

    void    print(void) const;
};

#endif
