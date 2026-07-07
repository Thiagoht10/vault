#include "TerminalEchoGuard.hpp"

TerminalEchoGuard::TerminalEchoGuard(bool disableCanonical)
    :_active(false)
{
    termios newSettings;

    if (tcgetattr(STDIN_FILENO, &_oldSettings) != 0)
        throw std::runtime_error("failure to read terminal setteings");

    newSettings = _oldSettings;
    newSettings.c_lflag &= ~ECHO;

    if (disableCanonical)
    {
        newSettings.c_lflag &= ~ICANON;
        newSettings.c_cc[VMIN] = 1;
        newSettings.c_cc[VTIME] = 0;
    }

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newSettings))
        throw std::runtime_error("failure to change terminal settings");

    _active = true;

}

TerminalEchoGuard::~TerminalEchoGuard()
{
    if (_active)
        tcsetattr(STDIN_FILENO, TCSANOW, &_oldSettings);
}