#ifndef SECURE_MEMORY_HPP
#define SECURE_MEMORY_HPP

#include <string>

void    secureErase(std::string& value) noexcept;

class SecureEraseGuard
{
private:
    std::string&    _value;

    
public:
    SecureEraseGuard(const SecureEraseGuard& other) = delete;
    SecureEraseGuard&   operator=(const SecureEraseGuard& other) = delete;
    
    explicit SecureEraseGuard(std::string& value) noexcept;
    ~SecureEraseGuard() noexcept;
};

#endif
