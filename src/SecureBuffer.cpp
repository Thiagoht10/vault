#include "SecureBuffer.hpp"

SecureBuffer::SecureBuffer(void) 
    : _buffer(NULL),
      _sizeUsed(0),
      _capacity(0),
      _increaseFactor(128)
{
    reserve(_increaseFactor);
}

SecureBuffer::SecureBuffer(SecureBuffer&& other) noexcept
    : _buffer(other._buffer),
      _sizeUsed(other._sizeUsed),
      _capacity(other._capacity),
      _increaseFactor(other._increaseFactor)
{
    other._buffer = NULL;
    other._sizeUsed = 0;
    other._capacity = 0;
}

SecureBuffer& SecureBuffer::operator=(SecureBuffer&& other) noexcept
{
    if (this != &other)
    {
        erase();
        _buffer = other._buffer;
        _sizeUsed = other._sizeUsed;
        _capacity = other._capacity;
        _increaseFactor = other._increaseFactor;
        other._buffer = NULL;
        other._sizeUsed = 0;
        other._capacity = 0;
    }
    return *this;
}

SecureBuffer::~SecureBuffer() 
{
    erase();
}

void    SecureBuffer::createSpace(std::size_t value)
{
    _buffer = new unsigned char [value]();
    if (!_buffer)
        throw std::runtime_error("failure to create memory");
    _capacity = value;
}

void    SecureBuffer::increaseSpace(void)
{
    reserve(_capacity + _increaseFactor);
}

std::size_t SecureBuffer::size(void) const
{
    return _sizeUsed;
}

void    SecureBuffer::erase(void)
{
    if (_buffer)
    {
        sodium_memzero(_buffer, _capacity);
        delete[] _buffer;
    }
    _buffer = NULL;
    _sizeUsed = 0;
    _capacity = 0;
}

unsigned char*  SecureBuffer::data(void)
{
    return _buffer;
}

const unsigned char*  SecureBuffer::data(void) const
{
    return _buffer;
}

const char*  SecureBuffer::c_data(void) const
{
    return reinterpret_cast<const char*>(_buffer);
}

void    SecureBuffer::readBytes(void)
{
    char    c;

    if (_buffer)
        clear(_buffer, _capacity);
    _sizeUsed = 0;

    while(std::cin.get(c))
    {
        if(c == '\n')
            return ;
        if (_sizeUsed + 1 >= _capacity)
            increaseSpace();
        _buffer[_sizeUsed] = static_cast<unsigned char>(c);
        _sizeUsed++;
        _buffer[_sizeUsed] = '\0';
    }
}

void    SecureBuffer::clear(unsigned char* data, std::size_t length)
{
    if (data && length > 0)
        sodium_memzero(data, length);
}

void    SecureBuffer::assign(const unsigned char* data, std::size_t length)
{
    size_t  i = 0;

    if (!data)
        throw std::runtime_error("failure to assign data");
    reserve(length + 1);
    clear(_buffer, _capacity);
    while (i < length)
    {
        _buffer[i] = data[i];
        i++;
    }
    _buffer[i] = '\0';
    _sizeUsed = length;
}

void    SecureBuffer::assign(const std::string& data)
{
    assign(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

void    SecureBuffer::swap(SecureBuffer& other) noexcept
{
    std::swap(_buffer, other._buffer);
    std::swap(_sizeUsed, other._sizeUsed);
    std::swap(_capacity, other._capacity);
    std::swap(_increaseFactor, other._increaseFactor);
}

void    SecureBuffer::reserve(size_t value)
{
    unsigned char*  tmp;

    if (value <= _capacity)
        return ;
    tmp = new unsigned char[value]();
    if (!tmp)
        throw std::runtime_error("failure to create memory");
    for (std::size_t i = 0; i < _sizeUsed; i++)
        tmp[i] = _buffer[i];
    if (_buffer)
    {
        sodium_memzero(_buffer, _capacity);
        delete[] _buffer;
    }
    _buffer = tmp;
    _capacity = value;
    if (_sizeUsed < _capacity)
        _buffer[_sizeUsed] = '\0';
}

void    SecureBuffer::resize(size_t value)
{
    reserve(value + 1);
    clear(_buffer, _capacity);
    _sizeUsed = value;
    _buffer[_sizeUsed] = '\0';
}

bool    SecureBuffer::empty(void) const
{
    return _sizeUsed == 0;
}

int SecureBuffer::compare(std::size_t position,
        std::size_t length, const char* data) const
{
    std::size_t i = 0;

    if (!data)
        throw std::invalid_argument("null data");

    if (position > _sizeUsed)
        throw std::out_of_range("position out of range");

    while (i < length && position + i < _sizeUsed && data[i])
    {
        if (_buffer[position + i] != static_cast<unsigned char>(data[i]))
            return _buffer[position + i] - static_cast<unsigned char>(data[i]);
        i++;
    }

    if (i == length)
        return 0;

    if (position + i == _sizeUsed && data[i] == '\0')
        return 0;

    if (position + i == _sizeUsed)
        return 0 - static_cast<unsigned char>(data[i]);

    return _buffer[position + i] - static_cast<unsigned char>(data[i]);
}

size_t  SecureBuffer::find(char character, size_t position) const
{
    size_t i = position;

    while(i < _sizeUsed)
    {
        if (_buffer[i] == static_cast<unsigned char>(character))
            return i;
        i++;
    }

    return std::string::npos;
}

size_t  SecureBuffer::max_size(void) const
{
    return static_cast<std::size_t>(-1);
}

void    SecureBuffer::append(const char* str)
{
    std::size_t length = 0;

    if (!str)
        throw std::invalid_argument("null data");
    while (str[length])
        length++;
    append(str, length);
}

void    SecureBuffer::append(const char* str, std::size_t length)
{
    std::size_t i = 0;

    if (!str)
        throw std::invalid_argument("null data");
    reserve(_sizeUsed + length + 1);
    while (i < length)
    {
        _buffer[_sizeUsed + i] = static_cast<unsigned char>(str[i]);
        i++;
    }
    _sizeUsed += length;
    _buffer[_sizeUsed] = '\0';
}

bool    SecureBuffer::operator==(const char* str)
{
    size_t i = 0;
    
    while (str[i])
        i++;

    if (size() != i)
        return false;
    if (compare(0, i, str) == 0)
        return true;
    return false;
}

bool    SecureBuffer::operator==(const SecureBuffer& other)
{
    if (size() != other.size())
        return false;
    
    for (size_t i = 0; i < size(); i++)
    {
        if(data()[i] != other.data()[i])
            return false;
    }

    return true;
}

bool    SecureBuffer::operator!=(const SecureBuffer& other)
{
    if (operator==(other))
        return false;
    
    return true;
}
