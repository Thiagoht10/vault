#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

class Message
{
public:
    enum Type
    {
        NONE,
        SUCCESS,
        INFO,
        ERROR
    };

private:
    std::string _text;
    Type        _type;

public:
    Message(void);
    Message(const std::string& text, Type type);
    ~Message(void);

    void                set(const std::string& text, Type type);
    void                clear(void);
    bool                empty(void) const;
    const std::string&  text(void) const;
    Type                type(void) const;
};

#endif
