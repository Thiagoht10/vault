#include "SecureString.hpp"

size_t  SecureString::size(void) const
{
    return _buffer.size();
}

void    SecureString::assign(const std::string& data)
{
    _buffer.assign(data);
}

void    SecureString::assign(const unsigned char* data, size_t lenght)
{
    _buffer.assign(data, lenght);
}

void    SecureString::swap(SecureString& other)
{
    _buffer.swap(other._buffer);
}

void    SecureString::erase(void)
{
    _buffer.erase();
}

const unsigned char*    SecureString::data(void) const
{
    return _buffer.data();
}

void    SecureString::readBytes(void)
{
    _buffer.readBytes();
}

bool    SecureString::empty()
{
    return _buffer.empty();
}

bool    SecureString::operator==(const char* str)
{
    return _buffer.operator==(str);
}