#include "Vault.hpp"
#include "FileManeger.hpp"
#include "App.hpp"


int main(int argc, char *argv[])
{
    App app;

    try
    {
        app.run(argc, argv);
    }
    catch(const std::exception& e)
    {
        std::cerr << '\n' << e.what() << '\n';
        return 1;
    }
    return 0;
}