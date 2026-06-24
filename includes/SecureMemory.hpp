#ifndef SECURE_MEMORY_HPP
#define SECURE_MEMORY_HPP

#include <string>

void    secureErase(std::string& value) noexcept;

class SecureEraseGuard
{
private:
    std::string&    _value;

    SecureEraseGuard(const SecureEraseGuard& other);
    SecureEraseGuard&   operator=(const SecureEraseGuard& other);

public:
    explicit SecureEraseGuard(std::string& value) noexcept;
    ~SecureEraseGuard() noexcept;
};

#endif
