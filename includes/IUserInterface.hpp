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
        ACTION_EDIT,
        ACTION_INVALID
    };

    enum InputResult
    {
        INPUT_OK,
        INPUT_CANCEL,
        INPUT_INVALID,
        INPUT_INTERRUPTED
    };

    struct MenuInput
    {
        InputResult result;
        MenuAction  action;
    };

    struct ConfirmationInput
    {
        InputResult result;
        bool        confirmed;
    };

    virtual MenuInput   askMainMenuAction(std::string& msg) = 0;
    virtual InputResult askPassWord(SecureBuffer& pass, std::string prompt) = 0;
    virtual InputResult askEntryIndex(size_t& index, const Vault& vault) const = 0;
    virtual InputResult askNewEntry(Entry& entry) = 0;
    virtual ConfirmationInput   askConfirmation(const Entry& entry) const = 0;
    virtual InputResult askEditEntry(SecureBuffer& pass, SecureBuffer& usr) = 0;

    virtual void        showEntryList(const Vault& vault) const = 0;
    virtual InputResult showEntryTemporarily(const Entry& entry) const = 0;
    virtual void        showError(std::string error) const = 0;
};


#endif
