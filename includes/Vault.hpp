#ifndef VAULT_HPP
#define VAULT_HPP

#include <string>
#include <vector>
#include "Entry.hpp"

class Vault
{
private:
    std::vector<Entry>  _entry;

public:
    Vault(void);
    Vault(const Vault& other) = delete;
    Vault&  operator=(const Vault& other) = delete;
    ~Vault() = default;

    void        addEntry(Entry&& entry);
    bool        removeEntry(size_t index);
    void        printAll(void) const;
    void        serialize(std::string& data) const;
    void        deserialize(const std::string& data);
};

#endif
