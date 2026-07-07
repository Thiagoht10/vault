#ifndef SECURE_STRING_HPP
#define SECURE_STRING_HPP

#include "SecureBuffer.hpp"

class SecureString
{
private:
    SecureBuffer    _buffer;

public:
    SecureString() = default;
    SecureString(const SecureString& other) = delete;
    SecureString&   operator=(const SecureString& other) = delete;
    SecureString(SecureString&& other) noexcept = default;
    SecureString&   operator=(SecureString&& other) noexcept = default;
    ~SecureString() = default;

    size_t                  size(void) const;
    void                    assign(const std::string& data);
    void                    assign(const unsigned char* data, size_t lenght);
    void                    swap(SecureString& other);
    void                    erase(void);
    const unsigned char*    data(void) const;
    bool                    readBytes(void);
    bool                    empty(void);
    bool                    operator==(const char* str);    
};

#endif
