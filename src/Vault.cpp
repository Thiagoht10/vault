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

void    Vault::print(void)
{
    size_t  size;
    size_t  index;
    std::string input;

    size = _entry.size();
    if(size == 0)
    {
        std::cout << "\nNothing to print\n" << std::endl;
        return ;
    }
    
    std::cout << "\n";
    printCredentialsHeader();
    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "please, select one index to view the credentials\n";
    std::cout << "type /cancel to cancel\n> ";
    if(!std::getline(std::cin, input))
    {
        std::cerr << "failure to cat your input" << std::endl;
        return;
    }
    if (!validInputIdx(input, index))
    {
        std::cerr << "\ninvalid index\n" << std::endl;
        return ;
    }
    else
    {
        if (index == std::string::npos)
            return ;
    }
    std::cout << "\n" << "------------------" << "\n" << std::endl;
    _entry[index].print();
}

void    Vault::printCredentialsHeader(void) const
{
    size_t  size = _entry.size();

    for (size_t i = 0; i < size; i++)
    {
        if (i > 0)
            std::cout << "\n" << "------------------" << "\n" << std::endl;
        std::cout << "[" << i << "]" << std::endl;
        std::cout << "service: " << _entry[i].getService() << std::endl;
    }
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

bool    Vault::validInputIdx(std::string& input, size_t& index)
{
    std::stringstream ss;

    if (input == "/cancel")
    {
        index = std::string::npos;
        return true;
    }
    ss << input;
    for (size_t i = 0; input[i]; i++)
    {
        if (input[i] < '0' || input[i] > '9')
            return false;
    }
    ss >> index;
    if (index >= _entry.size())
        return (false);
    return true;
}
