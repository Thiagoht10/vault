#include "SecureBuffer.hpp"

SecureBuffer::SecureBuffer(void) 
{
    _increaseFactor = 128;
    _sizeUsed = 128;
    createSpace(128);
}

SecureBuffer::SecureBuffer(SecureBuffer&& other) noexcept
    : _buffer(other._buffer),
      _sizeUsed(other._sizeUsed),
      _increaseFactor(other._increaseFactor)
{
    other._buffer = NULL;
    other._sizeUsed = 0;
}

SecureBuffer& SecureBuffer::operator=(SecureBuffer&& other) noexcept
{
    if (this != &other)
    {
        erase();
        _buffer = other._buffer;
        _sizeUsed = other._sizeUsed;
        _increaseFactor = other._increaseFactor;
        other._buffer = NULL;
        other._sizeUsed = 0;
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
}

std::size_t SecureBuffer::size(const unsigned char* data) const
{
    std::size_t size = 0;

    if (!data)
        return 0;
    for (size_t i = 0; data[i]; i++)
        size++;
    return size;
}

unsigned char* SecureBuffer::copy(const unsigned char* data)
{
    unsigned char* tmp;
    std::size_t    length;

    if (!data)
        throw std::runtime_error("failure to copy datas");

    length = size(data);
    tmp = new unsigned char [length + 1]();
    if(!tmp)
        throw std::runtime_error("failure to copy datas");
    
    for (size_t i = 0; i < length; i++)
        tmp[i] = data[i];

    return tmp;
}

void    SecureBuffer::eraseBuffer(unsigned char* data)
{
    if (!data)
        return ;
    sodium_memzero(data, size(data));

    delete[] data;
}

void    SecureBuffer::increaseSpace(void)
{
    unsigned char* tmp;

    tmp = copy(_buffer);
    
    erase();
    createSpace(size(tmp) + _increaseFactor);
    assign(tmp);
    _sizeUsed +=_increaseFactor;
    eraseBuffer(tmp);
}

std::size_t SecureBuffer::size(void) const
{
    return size(_buffer);
}

void    SecureBuffer::erase(void)
{
    if (_buffer)
    {
        sodium_memzero(_buffer, _sizeUsed);
        delete[] _buffer;
    }
    _buffer = NULL;
    _sizeUsed = 0;
}

unsigned char*  SecureBuffer::data(void)
{
    return _buffer;
}

const unsigned char*  SecureBuffer::data(void) const
{
    return _buffer;
}

void    SecureBuffer::readBytes(void)
{
    char    c;
    size_t  i = 0;

    while(std::cin.get(c))
    {
        if(c == '\n')
            return ;
        if(size() < _sizeUsed)
        {
            _buffer[i] = c;
            i++;
        }
        else
        {
            increaseSpace();
            _buffer[i] = c;
            i++;
        }
    }
}

void    SecureBuffer::clear(unsigned char* data, std::size_t length)
{
    if (data && length > 0)
        sodium_memzero(data, length);
}

void    SecureBuffer::assign(const unsigned char* data)
{
    assign(data, size(data));
}

void    SecureBuffer::assign(const unsigned char* data, std::size_t length)
{
    size_t  i = 0;

    if (!data)
        throw std::runtime_error("failure to assign data");
    if (_sizeUsed < length + 1)
    {
        erase();
        createSpace(length + 1);
        _sizeUsed = length + 1;
    }
    else
        clear(_buffer, _sizeUsed);
    while (i < length)
    {
        _buffer[i] = data[i];
        i++;
    }
    _buffer[i] = '\0';
}

void    SecureBuffer::assign(const std::string& data)
{
    assign(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

void    SecureBuffer::swap(SecureBuffer& other) noexcept
{
    std::swap(_buffer, other._buffer);
    std::swap(_sizeUsed, other._sizeUsed);
    std::swap(_increaseFactor, other._increaseFactor);
}

void    SecureBuffer::resize(size_t value)
{
    _buffer = new unsigned char [value]();
}
