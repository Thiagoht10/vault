#ifndef CONSOLE_U_I_HPP
#define CONSOLE_U_I_HPP

#include "IUserInterface.hpp"
#include "Message.hpp"
#include "TerminalEchoGuard.hpp"
#include <iostream>
#include <string>
#include <sys/select.h>

class ConsoleUI : public IUserInterface
{
public:
    ConsoleUI() = default;
    ~ConsoleUI() = default;

    InputResult readHiddenInput(SecureBuffer& pass, std::string prompt);
    void        clearTerminal(void) const;
    bool        getLine(std::string& input) const;
    bool        readHiddenLine(SecureBuffer& input) const;
    bool        waitEnterOrTimeout(int secunds) const;

    MenuInput   askMainMenuAction(const Message& message);
    InputResult askPassWord(SecureBuffer& pass, std::string prompt);
    InputOutcome askEntryIndex(size_t& index, const Vault& vault) const;
    InputOutcome askNewEntry(Entry& entry);
    ConfirmationInput   askConfirmation(const Entry& entry) const;
    InputOutcome askEditEntry(SecureBuffer& pass, SecureBuffer& usr);

    void        showEntryList(const Vault& valt) const;
    void        showEntryDetais(const Entry& entry) const;
    InputResult showPasswordTemporarily(const Entry& entry) const;
    InputResult showEntryTemporarily(const Entry& entry) const;
    void        showError(std::string error) const;
    void        showMessage(const Message& message) const;
};


#endif
