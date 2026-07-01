#ifndef CONSOLE_U_I_HPP
#define CONSOLE_U_I_HPP

#include "IUserInterface.hpp"
#include "TerminalEchoGuard.hpp"
#include <iostream>
#include <string>

class ConsoleUI : public IUserInterface
{
public:
    ConsoleUI() = default;
    ~ConsoleUI() = default;

    void readHiddenInput(SecureBuffer& pass, std::string prompt);

    MenuAction  askMainMenuAction(void);
    void        askPassWord(SecureBuffer& pass, std::string prompt);
    bool        askEntryIndex(size_t& index, const Vault& vault) const;
    bool        askNewEntry(Entry& entry);

    void        showEntryList(const Vault& valt) const;
    void        showEntryDetais(const Entry& entry) const;
    void        showEntryTemporarily(const Entry& entry) const;
    void        showError(std::string error) const;

};


#endif
