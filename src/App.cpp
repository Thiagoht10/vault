#include "App.hpp"
#include "SecureMemory.hpp"

#include <utility>

App::App(void) {}

App::~App() 
{
    erasePassword();
}

void    App::parseArgs(int argc, char *argv[])
{
    if (argc < 2)
        throw std::runtime_error("wrong number argument");

    _fileManeger.setPath(argv[1]);
}

void    App::add(void)
{
    Entry   entry;
    std::string tmp;
    bool cancel = false;
    SecureEraseGuard tmpGuard(tmp);

    std::cout << "\n" << "------------------" << "\n" << std::endl;
    std::cout << "please, insert datas:" << std::endl;
    std::cout << "type /cancel to cancel\n" << std::endl;

    while(1)
    {
        std::cout << "Service: \n" << "> ";
        if(!std::getline(std::cin, tmp))
            return ;
        if(tmp.empty())
        {
            std::cout << "Can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
        {
            cancel = true;
            break;
        }
        entry.setService(tmp);
        secureErase(tmp);
        break;
    }

    while (1)
    {
        if (cancel)
            break;
        std::cout << "Username: \n";
        if (!std::getline(std::cin, tmp))
            return ;
        if (tmp.empty())
        {
            std::cout << "Can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
        {
            cancel = true;
            break;
        }
        entry.setUsername(tmp);
        secureErase(tmp);
        break;
    }
    
    while (1)
    {
        if (cancel)
            break;
        std::cout << "Password: \n";
        if (!std::getline(std::cin, tmp))
            return ;
        if (tmp.empty())
        {
            std::cout << "Can't have empyt inputs\n" << std::endl;
            continue ;
        }
        if(tmp == "/cancel")
        {
            cancel = true;
            break;
        }
        entry.setPassword(tmp);
        secureErase(tmp);
        break;
    }
    if (!cancel)
        _vault.addEntry(std::move(entry));
}

void    App::show(void)
{
    _vault.printAll();
}

void    App::del(void)
{
    std::string input;
    size_t      index;
    std::stringstream   ss;

    while (1)
    {
        std::cout << "please, select an index" << std::endl;
        std::cout << "type /cancel to cancel\n" << std::endl;

        if (!std::getline(std::cin, input))
            return;
        if(input == "/cancel")
            return;
        ss << input;
        for (int i = 0; input[i]; i++)
        {
            if(input[i] < '0' || input[i] > '9')
            {
                std::cout << "invalid index\n" << std::endl;
                continue;
            }
        }
        ss >> index;
        _vault.removeEntry(index);
        break;
    }
}

void    App::erasePassword(void)
{
    secureErase(_masterPassword);
}

void    App::run(int argc, char *argv[])
{
    EncryptedData data;
    std::string plaintext;
    SecureEraseGuard plaintextGuard(plaintext);

    parseArgs(argc, argv);
    std::cout << "Please, put your password\n" << "> ";
    if (!std::getline(std::cin, _masterPassword))
    {
        throw std::runtime_error("invalid password");
    }
    if (_fileManeger.ifExist())
    {
        data = _fileManeger.readEncrypted();
        plaintext = _crypto.decrypt(data, _masterPassword);
        _vault.deserialize(plaintext);
        secureErase(plaintext);
    }

    while(1)
    {
        std::cout << "\n" << "------------------" << "\n" << std::endl;
        std::cout << "\nSelect one option\n" << std::endl;
        std::cout << "1. add" << std::endl;
        std::cout << "2. show" << std::endl;
        std::cout << "3. delete" << std::endl;
        std::cout << "4. exit\n" << "\n> ";

        if (!std::getline(std::cin, _option))
        {
            throw std::runtime_error("failure in insert of dates");
        }
        
        if(_option == "1")
            add();
        else if(_option == "2")
            show();
        else if(_option == "3")
            del();
        else if(_option == "4")
            break;
        else
            std::cout << "option not found" << std::endl;
    }
    _vault.serialize(plaintext);
    data = _crypto.encrypt(plaintext, _masterPassword);
    secureErase(plaintext);
    _fileManeger.writeEncrypted(data);
}
