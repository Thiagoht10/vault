#ifndef PASSWORD_POLICY_HPP
#define PASSWORD_POLICY_HPP

#include "SecureBuffer.hpp"

class PasswordPolicy
{
private:
    bool        checkCommonPassword(const SecureBuffer& pass) const;
    bool        hasRepeatChars(const SecureBuffer& pass, size_t limit) const;
    bool        hasSequenceChars(const SecureBuffer& pass, size_t limit) const;

public:
    enum    Result
    {
        PW_OK,
        PW_EMPTY,
        PW_TOO_LONG,
        PW_TOO_SHORT,
        PW_TOO_COMMON,
        PW_REPEAT_CHAR,
        PW_SEQUENCE_CHAR
    };

    PasswordPolicy() = default;
    ~PasswordPolicy() = default;
    
    Result      checkPasswordPolity(const SecureBuffer& pass) const;
};


#endif