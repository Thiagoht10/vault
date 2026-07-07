#include "ConsoleUI.hpp"

#include <unistd.h>

namespace
{
    void    consumeEscapeSequence(void)
    {
        char    seq;

        if (read(STDIN_FILENO, &seq, 1) != 1)
            return;
        if (seq == '[')
        {
            while (read(STDIN_FILENO, &seq, 1) == 1
                    && (seq < '@' || seq > '~'))
                ;
        }
        else if (seq == 'O')
            read(STDIN_FILENO, &seq, 1);
    }
}

IUserInterface::InputResult ConsoleUI::readHiddenInput(SecureBuffer& pass,
        std::string prompt)
{
    std::cout << prompt << "\n" << "> ";
    std::cout.flush();

    if (!readHiddenLine(pass))
    {
        std::cerr << "\n";
        return INPUT_INTERRUPTED;
    }
    return INPUT_OK;
}

void    ConsoleUI::clearTerminal(void) const
{
    std::cout << "\033[2J" << "\033[3J" << "\033[2H";

    std::cout.flush();
}

bool    ConsoleUI::getLine(std::string& input) const
{
    char    c;
    bool    completed;

    input.clear();
    completed = false;
    std::cout.flush();
    {
        TerminalEchoGuard guard(true);
        while (read(STDIN_FILENO, &c, 1) == 1)
        {
            if (c == '\n' || c == '\r')
            {
                completed = true;
                break;
            }

            if (c == 127 || c == '\b')
            {
                if (!input.empty())
                {
                    input.erase(input.size() - 1);
                    std::cout << "\b \b";
                    std::cout.flush();
                }
                continue;
            }

            if (c == '\033')
            {
                consumeEscapeSequence();
                continue;
            }
            if (c < 32 || c == 127)
                continue;
            input += c;
            std::cout << c;
            std::cout.flush();
        }
    }
    std::cout << '\n';
    return completed || !input.empty();
}

bool    ConsoleUI::readHiddenLine(SecureBuffer& input) const
{
    char    c;
    bool    completed;

    input.erase();
    completed = false;
    {
        TerminalEchoGuard guard(true);
        while (true)
        {
            if (read(STDIN_FILENO, &c, 1) == 1)
            {
                if (c == '\n' || c == '\r')
                {
                    completed = true;
                    break;
                }
                if (c == 127 || c == '\b')
                {
                    input.popByte();
                    continue;
                }
                if (c == '\033')
                {
                    consumeEscapeSequence();
                    continue;
                }
                if (c < 32 || c == 127)
                    continue;
                input.pushByte(static_cast<unsigned char>(c));
            }
            else
                return false;
        }
    }
    std::cout << '\n';
    return completed || !input.empty();
}

IUserInterface::MenuInput   ConsoleUI::askMainMenuAction(std::string& msg)
{
    std::string option;
    MenuInput   input;

    clearTerminal();
    if(!msg.empty())
        showMessage(msg);
    std::cout << "------------------" << std::endl;
    std::cout << "\nselect one option\n" << std::endl;
    std::cout << "1. add" << std::endl;
    std::cout << "2. show" << std::endl;
    std::cout << "3. delete" << std::endl;
    std::cout << "0. exit\n";
    input.result = INPUT_OK;
    input.action = ACTION_INVALID;

    std::cout << "\n> ";
    if (!getLine(option))
    {
        std::cerr << "\n";
        input.result = INPUT_INTERRUPTED;
        return input;
    }

    if (option == "0")
        input.action = ACTION_EXIT;
    else if (option == "1")
        input.action = ACTION_ADD;
    else if (option == "2")
        input.action = ACTION_SHOW;
    else if (option == "3")
        input.action = ACTION_DELETE;
    return input;
}

IUserInterface::InputResult ConsoleUI::askPassWord(SecureBuffer& pass,
        std::string prompt)
{
    return readHiddenInput(pass, prompt);
}

void    ConsoleUI::showEntryList(const Vault& vault) const
{
    std::stringstream   ss;

    for (size_t i = 0; i < vault.size(); i++)
    {
        const Entry& entry = vault.getEntry(i);

        if (i > 0)
            std::cout << "\n" << "------------------" << std::endl;
        std::cout << "\n[" << i << "]" << std::endl;
        std::cout << "service: " << entry.getService() << std::endl;
    }
}

void    ConsoleUI::showEntryDetais(const Entry& entry) const
{
    std::cout << "\n" << "------------------" << std::endl;
    std::cout << "service: ";
    std::cout.write(reinterpret_cast<const char*>(entry.getService()),
            entry.getServiceSize()) << "\n";
    std::cout << "username: ";
    std::cout.write(reinterpret_cast<const char*>(entry.getUsername()),
            entry.getUserNameSize()) << "\n";
    std::cout << "password: ********" << std::endl;
    std::cout << "------------------" << std::endl;
}

IUserInterface::InputResult ConsoleUI::showPasswordTemporarily(const Entry& entry) const
{
    std::string tmp;

    std::cout << "\033[2J\033[H";

    std::cout << "password: ";
    std::cout.write(reinterpret_cast<const char*>(entry.getPassword()),
            entry.getPasswordSize()) << "\n";

    std::cout << "\npress Enter to continue...";
    std::cout.flush();
    if (!getLine(tmp))
        return INPUT_INTERRUPTED;

    std::cout << "\033[2J\033[H";
    std::cout.flush();
    return INPUT_OK;
}

IUserInterface::InputResult ConsoleUI::showEntryTemporarily(const Entry& entry) const
{
    InputResult result;
    std::string input;

    std::cout << "\033[?1049h"; // entra na tela temporaria
    std::cout << "\033[2J\033[H";

    showEntryDetais(entry);
    std::cout << "\ndo you want to reveal the password?" << std::endl;
    std::cout << "Y to yes\n> ";
    std::cout.flush();
    if (!getLine(input))
        result = INPUT_INTERRUPTED;
    else if (input == "Y" || input == "y")
        result = showPasswordTemporarily(entry);
    else
        result = INPUT_OK;

    std::cout << "\033[2J\033[H";
    std::cout << "\033[?1049l";
    std::cout.flush();
    return result;
}

IUserInterface::InputResult ConsoleUI::askEntryIndex(size_t& index,
        const Vault& vault) const
{
    std::stringstream   ss;
    std::string         input;

    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "please, select an index" << std::endl;
    std::cout << "type /cancel to cancel\n> ";

    if (!getLine(input))
    {
        std::cerr << "\n";
        return INPUT_INTERRUPTED;
    }

    if (input.empty())
        return INPUT_INVALID;

    if (input == "/cancel")
        return INPUT_CANCEL;
    
    for (size_t i = 0; input[i]; i++)
    {
        if (input[i] < '0' || input[i] > '9')
            return INPUT_INVALID;
    }
    ss << input;
    ss >> index;

    if (!vault.isValidIndex(index))
    {
        std::cerr << "\ninvalid index\n" << std::endl;
        return INPUT_INVALID;
    }
    return INPUT_OK;
}

void    ConsoleUI::showError(std::string error) const
{
    std::cerr << "\n" << error << "\n" << std::endl;
}


IUserInterface::InputResult ConsoleUI::askNewEntry(Entry& entry)
{
    SecureBuffer    password;
    SecureBuffer    checkPass;
    SecureString    tmp;

    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "please, insert datas:" << std::endl;
    std::cout << "type /cancel to cancel\n" << std::endl;

    while(1)
    {
        tmp.erase();
        std::cout << "service: \n" << "> ";
        std::cout.flush();
        if (!tmp.readBytes())
        {
            std::cerr << "\n";
            return INPUT_INTERRUPTED;
        }
        if(tmp.empty())
        {
            std::cerr << "can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
            return INPUT_CANCEL;

        entry.setService(tmp.data(), tmp.size());
        tmp.erase();
        break;
    }

    while (1)
    {
        tmp.erase();
        std::cout << "username: \n" << "> ";
        std::cout.flush();
        if (!tmp.readBytes())
        {
            std::cerr << "\n";
            return INPUT_INTERRUPTED;
        }
        if (tmp.empty())
        {
            std::cerr << "can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
            return INPUT_CANCEL;

        entry.setUsername(tmp.data(), tmp.size());
        tmp.erase();
        break;
    }
    
	while (1)
	{
        bool    unlocked = false;

        for (size_t attempt = 0; attempt < 3 && !unlocked; attempt++)
        {
            password.erase();
            checkPass.erase();

            if (readHiddenInput(password, "password:") == INPUT_INTERRUPTED)
                return INPUT_INTERRUPTED;
            
            if(password == "/cancel")
                return INPUT_CANCEL;

            if (password.empty())
            {
                std::cerr << "can't have empyt inputs\n" << std::endl;
                continue;
            }

            if (readHiddenInput(checkPass, "\nplease, confirm your password")
                    == INPUT_INTERRUPTED)
                return INPUT_INTERRUPTED;
            
            if(checkPass == "/cancel")
                return INPUT_CANCEL;

            if(password == checkPass)
                unlocked = true;
            else
            {
                std::cerr << "\n\npassword not match" << std::endl;
                if (attempt < 3)
                    std::cerr << "you have " << (3 - attempt - 1) << " attempts\n" << std::endl;
                password.erase();
                checkPass.erase();
            }
            if (attempt + 1 == 3 && !unlocked)
                return INPUT_INVALID;
        }
        entry.setPassword(password.data(), password.size());
        password.erase();
        checkPass.erase();
        break;
	}
    return INPUT_OK;
}

IUserInterface::ConfirmationInput ConsoleUI::askConfirmation(const Entry& entry) const
{
    std::string         option;
    ConfirmationInput   input;

    input.result = INPUT_OK;
    input.confirmed = false;
    std::cout << "\nare you sure you want to delete this credential?\n";
    std::cout << "service: " << entry.getService() << std::endl;
    std::cout << "\nY to yes | N to not\n" << std::endl;

    while (option != "N" && option != "n" && option != "Y" && option != "y")
    {
        std::cout << "> ";
        if (!getLine(option))
        {
            std::cerr << "\n";
            input.result = INPUT_INTERRUPTED;
            return input;
        }
        if (option != "N" && option != "n" && option != "Y" && option != "y")
            showError("invalid option");
    }

    if (option == "N" || option == "n")
        return input;

    input.confirmed = true;
    return input;
}

void    ConsoleUI::showMessage(std::string msg) const
{
    std::cout << "\n" << msg << "\n" << std::endl;
}
