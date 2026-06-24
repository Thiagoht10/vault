#include "SecureMemory.hpp"

#include <sodium.h>

void    secureErase(std::string& value) noexcept
{
    if (!value.empty())
        sodium_memzero(&value[0], value.size());
    value.clear();
}

SecureEraseGuard::SecureEraseGuard(std::string& value) noexcept
    : _value(value) {}

SecureEraseGuard::~SecureEraseGuard() noexcept
{
    secureErase(_value);
}
