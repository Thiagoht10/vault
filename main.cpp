#include "Vault.hpp"
#include "FileManeger.hpp"


int main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    Vault cofre;
    Entry entry("Google", "thde-sou", "123");
    std::string data;

    cofre.addEntry(entry);
    cofre.printAll();

    data = cofre.serialize();
    std::cout << data;

    cofre.deserialize(data);
    cofre.printAll();

    cofre.removeEntry(0);
    cofre.printAll();
    return (0);
}