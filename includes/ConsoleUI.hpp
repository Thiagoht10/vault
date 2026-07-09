#ifndef CONSOLE_U_I_HPP
#define CONSOLE_U_I_HPP

#include "IUserInterface.hpp"
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

    MenuInput   askMainMenuAction(std::string& msg);
    InputResult askPassWord(SecureBuffer& pass, std::string prompt);
    InputResult askEntryIndex(size_t& index, const Vault& vault) const;
    InputResult askNewEntry(Entry& entry);
    ConfirmationInput   askConfirmation(const Entry& entry) const;
    InputResult askEditEntry(SecureBuffer& pass, SecureBuffer& usr);

    void        showEntryList(const Vault& valt) const;
    void        showEntryDetais(const Entry& entry) const;
    InputResult showPasswordTemporarily(const Entry& entry) const;
    InputResult showEntryTemporarily(const Entry& entry) const;
    void        showError(std::string error) const;
    void        showMessage(std::string msg) const;
};


#endif
