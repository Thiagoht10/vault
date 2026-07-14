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
        
    if (!checkCommonPassword(pass))
        return PW_TOO_COMMON;

    if (pass.size() < 15)
        return PW_TOO_SHORT;
    
    if (pass.size() > 64)
        return PW_TOO_LONG;

    return PW_OK;
}

PasswordPolicy::Warnings PasswordPolicy::checkPasswordWarnings(const
        SecureBuffer& pass) const
{
    Warnings warnings;

    warnings.repeatedChars = !hasRepeatChars(pass, 3);
    warnings.sequenceChars = !hasSequenceChars(pass, 4);
    return warnings;
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


bool    PasswordPolicy::hasRepeatChars(const SecureBuffer& pass, size_t limit) const
{
    size_t count = 1;
    
    for (size_t i = 1; i < pass.size(); i++)
    {
        if (pass.data()[i] == pass.data()[i - 1])
            count++;
        else
            count = 1;

        if (count >= limit)
            return false;
    }
    return true;
}


bool    PasswordPolicy::hasSequenceChars(const SecureBuffer& pass, size_t limit) const
{
    size_t countSeqPOS = 1;
    size_t countSeqNEG = 1;
    
    for (size_t i = 1; i < pass.size(); i++)
    {
        if (pass.data()[i] > pass.data()[i - 1]
                && pass.data()[i] - pass.data()[i - 1] == 1)
            countSeqPOS++;
        else
            countSeqPOS = 1;
    
        if (pass.data()[i] < pass.data()[i - 1]
                && pass.data()[i - 1] - pass.data()[i] == 1)
            countSeqNEG++;
        else
            countSeqNEG = 1;
        
        if (countSeqPOS >= limit || countSeqNEG >= limit)
            return false;
    }
    return true;
}
