#include "App.hpp"
#include "ConsoleUI.hpp"


int main(int argc, char *argv[])
{
    ConsoleUI ui;
    App app(ui);

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