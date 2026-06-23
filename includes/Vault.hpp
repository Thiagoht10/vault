#ifndef VAULT_HPP
#define VAULT_HPP

#include <vector>
#include <sstream>
#include "Entry.hpp"

class Vault
{
private:
    std::vector<Entry>  _entry;

public:
    Vault(void);
    Vault(const Vault& other);
    Vault&  operator=(const Vault& other);
    ~Vault();

    void        addEntry(Entry& entry);
    bool        removeEntry(size_t index);
    void        printAll(void) const;
    std::string serialize(void) const;
    void        deserialize(std::string& data);
};

#endif