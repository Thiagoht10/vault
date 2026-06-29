#include "ConsoleUI.hpp"

void    ConsoleUI::readHiddenInput(SecureBuffer& pass, std::string prompt)
{
    std::cout << prompt << "\n" << "> ";

    TerminalEchoGuard guard;

    pass.readBytes();
}

IUserInterface::MenuAction  ConsoleUI::askMainMenuAction(void)
{
    std::string option;

    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "\nselect one option\n" << std::endl;
    std::cout << "1. add" << std::endl;
    std::cout << "2. show" << std::endl;
    std::cout << "3. delete" << std::endl;
    std::cout << "0. exit\n" << "\n> ";
    if (!std::getline(std::cin, option))
        throw std::runtime_error("failure in insert of dates");

    if (option == "0")
        return ACTION_EXIT;
    else if (option == "1")
        return ACTION_ADD;
    else if (option == "2")
        return ACTION_SHOW;
    else if (option == "3")
        return ACTION_DELETE;
    else
        return ACTION_INVALID;
}

void       ConsoleUI::askPassWord(SecureBuffer& pass, std::string prompt)
{
    readHiddenInput(pass, prompt);
}

void    ConsoleUI::showEntryList(const Vault& vault) const
{
    for (size_t i = 0; i < vault.size(); i++)
    {
        const Entry& entry = vault.getEntry(i);

        if (i > 0)
            std::cout << "\n" << "------------------" << std::endl;
        std::cout << "\n[" << i << "]" << std::endl;
        std::cout << "service: " << entry.getService() << std::endl;
    }
}

bool    ConsoleUI::askEntryIndex(size_t& index, const Vault& vault)
{
    std::stringstream   ss;
    std::string         input;

    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "please, select an index" << std::endl;
    std::cout << "type /cancel to cancel\n> ";

    if (!std::getline(std::cin, input))
        return false;

    if (input == "/cancel")
        return false;
    
    for (size_t i = 0; input[i]; i++)
    {
        if (input[i] < '0' || input[i] > '9')
            return false;
    }
    ss << input;
    ss >> index;

    if (!vault.isValidIndex(index))
    {
        std::cerr << "\ninvalid index\n" << std::endl;
        return false;
    }
    return true;
}

void    ConsoleUI::showError(std::string error) const
{
    std::cerr << "\n" << error << "\n" << std::endl;
}


bool    ConsoleUI::askNewEntry(Entry& entry)
{
    SecureBuffer    password;
    SecureBuffer    checkPass;
    SecureString    tmp;

    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "please, insert datas:" << std::endl;
    std::cout << "type /cancel to cancel\n" << std::endl;

    while(1)
    {
        std::cout << "service: \n" << "> ";
        tmp.erase();
        tmp.readBytes();
        if(tmp.empty())
        {
            std::cerr << "can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
            return false;
        
        entry.setService(tmp.data());
        tmp.erase();
        break;
    }

    while (1)
    {
        std::cout << "username: \n" << "> ";
        tmp.erase();
        tmp.readBytes();
        if (tmp.empty())
        {
            std::cerr << "can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
            return false;
        
        entry.setUsername(tmp.data());
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

            readHiddenInput(password, "password:");
            if(password == "/cancel")
                return false;

            if (password.empty())
            {
                std::cerr << "can't have empyt inputs\n" << std::endl;
                continue;
            }

            readHiddenInput(checkPass, "\nplease, confirm your password");
            if(checkPass == "/cancel")
                return false;

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
                return false;
        }
        entry.setPassword(password.data(), password.size());
        password.erase();
        checkPass.erase();
        break;
	}
    return true;
}
