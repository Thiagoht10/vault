#ifndef I_USER_INTERFACE_HPP
#define I_USER_INTERFACE_HPP

#include "Message.hpp"
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
        ACTION_INVALID,
        ACTION_CHANGE_PASSWORD
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

    struct InputOutcome
    {
        InputResult result;
        Message     message;
    };

    virtual MenuInput   askMainMenuAction(const Message& message) = 0;
    virtual InputResult askPassWord(SecureBuffer& pass, std::string prompt) = 0;
    virtual InputOutcome askEntryIndex(size_t& index, const Vault& vault) const = 0;
    virtual InputOutcome askNewEntry(Entry& entry) = 0;
    virtual ConfirmationInput   askConfirmation(const Entry& entry) const = 0;
    virtual InputOutcome askEditEntry(SecureBuffer& pass, SecureBuffer& usr) = 0;

    virtual void        showEntryList(const Vault& vault) const = 0;
    virtual InputResult showEntryTemporarily(const Entry& entry) const = 0;
    virtual void        showError(std::string error) const = 0;
};


#endif
