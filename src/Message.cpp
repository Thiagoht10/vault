#include "Message.hpp"

Message::Message(void)
    : _type(NONE) {}

Message::Message(const std::string& text, Type type)
    : _text(text), _type(type) {}

Message::~Message(void) {}

void    Message::set(const std::string& text, Type type)
{
    _text = text;
    _type = type;
}

void    Message::clear(void)
{
    _text.clear();
    _type = NONE;
}

bool    Message::empty(void) const
{
    return _text.empty();
}

const std::string&  Message::text(void) const
{
    return _text;
}

Message::Type   Message::type(void) const
{
    return _type;
}
