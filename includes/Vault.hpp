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

    void            addEntry(Entry&& entry);
    bool            removeEntry(size_t index);
    void            serialize(SecureBuffer& data) const;
    void            deserialize(SecureBuffer& data);
    size_t          size(void) const;
    bool            isValidIndex(size_t index) const;
    const Entry&    getEntry(size_t index) const;
    bool            editPassword(size_t index, SecureBuffer password);
    bool            editUsername(size_t index, SecureBuffer username);
};

#endif
