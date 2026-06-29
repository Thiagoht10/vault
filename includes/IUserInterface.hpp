#ifndef I_USER_INTERFACE_HPP
#define I_USER_INTERFACE_HPP

#include "SecureBuffer.hpp"
#include "Vault.hpp"

class IUserInterface
{
public:
    IUserInterface() {};
    virtual ~IUserInterface() {};

    enum MenuAction
    {
        ACTION_ADD,
        ACTION_SHOW,
        ACTION_DELETE,
        ACTION_EXIT,
        ACTION_INVALID
    };

    virtual MenuAction  askMainMenuAction(void) = 0;
    virtual void        askPassWord(SecureBuffer& pass, std::string prompt) = 0;
    virtual bool        askEntryIndex(size_t& index, const Vault& vault) = 0;
    virtual bool        askNewEntry(Entry& entry) = 0;

    virtual void        showEntryList(const Vault& vault) const = 0;
    virtual void        showError(std::string error) const = 0;

};


#endif