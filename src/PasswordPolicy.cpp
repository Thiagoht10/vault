#include "PasswordPolicy.hpp"

static const char* COMMON_PASSWORDS[] = {
    "password",
    "123456",
    "123456789",
    "qwerty",
    "admin",
    "letmein",
    "senha",
    "senha123",
    "password123",
    "12345678",
    0
    };

PasswordPolicy::Result  PasswordPolicy::checkPasswordPolity(const
        SecureBuffer& pass) const
{
    if (pass.size() == 0)
        return PW_EMPTY;

    if (pass.size() < 15)
        return PW_TOO_SHORT;
    
    if (pass.size() > 64)
        return PW_TOO_LONG;

    if (!checkCommonPassword(pass))
        return PW_TOO_COMMON;
    
    return PW_OK;
}

bool    PasswordPolicy::checkCommonPassword(const SecureBuffer& pass) const
{
    size_t  i = 0;
    SecureBuffer    tmp;

    while (COMMON_PASSWORDS[i])
    {
        tmp.assign(COMMON_PASSWORDS[i]);
        if (pass == tmp)
            return false;
        tmp.erase();
        i++;
    }
    return true;
}