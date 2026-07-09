#include "Vault.hpp"

#include <stdexcept>
#include <utility>

namespace
{
void    addSize(std::size_t& total, std::size_t amount,
    std::size_t maximum)
{
    if (amount > maximum - total)
        throw std::length_error("vault data is too large");
    total += amount;
}

void    readField(SecureBuffer& data, std::size_t& position,
    const char *prefix, std::size_t prefixLength,
    std::size_t& valuePosition, std::size_t& valueLength)
{
    std::size_t lineEnd;

    if (position > data.size()
        || prefixLength > data.size() - position
        || data.compare(position, prefixLength, prefix) != 0)
        throw std::runtime_error("invalid serialized vault data");
    position += prefixLength;
    lineEnd = data.find('\n', position);
    if (lineEnd == std::string::npos)
        throw std::runtime_error("invalid serialized vault data");
    valuePosition = position;
    valueLength = lineEnd - position;
    position = lineEnd + 1;
}
}

Vault::Vault(void) {}

void    Vault::addEntry(Entry&& entry)
{
    _entry.push_back(std::move(entry));
}

bool    Vault::removeEntry(size_t index)
{
    if(index >= _entry.size())
        return false;

    _entry[index].eraseField();
    _entry.erase(_entry.begin() + index);
    return true;
}

void    Vault::serialize(SecureBuffer& data) const
{
    const std::size_t   entryOverhead = 41;
    std::size_t         requiredSize = 0;

    data.erase();
    for (std::size_t i = 0; i < _entry.size(); i++)
    {
        addSize(requiredSize, entryOverhead, data.max_size());
        addSize(requiredSize, _entry[i].getServiceSize(),
            data.max_size());
        addSize(requiredSize, _entry[i].getUserNameSize(),
            data.max_size());
        addSize(requiredSize, _entry[i].getPasswordSize(),
            data.max_size());
    }
    data.reserve(requiredSize + 1);
    for (std::size_t i = 0; i < _entry.size(); i++)
    {
        data.append("service:");
        data.append(reinterpret_cast<const char*>(_entry[i].getService()),
            _entry[i].getServiceSize());
        data.append("\nusername:");
        data.append(reinterpret_cast<const char*>(_entry[i].getUsername()),
            _entry[i].getUserNameSize());
        data.append("\npassword:");
        data.append(reinterpret_cast<const char*>(_entry[i].getPassword()),
            _entry[i].getPasswordSize());
        data.append("\n----------\n\n");
    }
}

void    Vault::deserialize(SecureBuffer& data)
{
    const char  separator[] = "----------\n\n";
    std::size_t position = 0;
    std::size_t valuePosition;
    std::size_t valueLength;

    _entry.clear();
    while (position < data.size())
    {
        Entry entry;

        readField(data, position, "service:", 8,
            valuePosition, valueLength);
        entry.setService(data.data() + valuePosition, valueLength);
        readField(data, position, "username:", 9,
            valuePosition, valueLength);
        entry.setUsername(data.data() + valuePosition, valueLength);
        readField(data, position, "password:", 9,
            valuePosition, valueLength);
        entry.setPassword(data.data() + valuePosition, valueLength);
        if (position > data.size()
            || sizeof(separator) - 1 > data.size() - position
            || data.compare(position, sizeof(separator) - 1,
                separator) != 0)
            throw std::runtime_error("invalid serialized vault data");
        position += sizeof(separator) - 1;
        _entry.push_back(std::move(entry));
    }
}

size_t  Vault::size(void) const
{
    return _entry.size();
}

bool    Vault::isValidIndex(size_t index) const
{
    if (index < size())
        return true;
    return false;
}

const Entry&  Vault::getEntry(size_t index) const
{
    if (!isValidIndex(index))
        throw std::runtime_error("\ninvalid index\n");
    return _entry[index];
    
}

bool    Vault::editPassword(size_t index, SecureBuffer password)
{
    if (index >= size())
        return false;

    _entry[index].setPassword(password.data(), password.size());
    return true;
}

bool    Vault::editUsername(size_t index, SecureBuffer username)
{
    if (index >= size())
        return false;

    _entry[index].setUsername(username.data(), username.size());
    return true;
}
