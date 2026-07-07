#ifndef TERMINAL_ECHO_GUARD_HPP
#define TERMINAL_ECHO_GUARD_HPP

#include <termios.h>
#include <unistd.h>
#include <stdexcept>

class TerminalEchoGuard
{
private:
    termios _oldSettings;
    bool    _active;

public:
    TerminalEchoGuard(bool disableCanonical);
    ~TerminalEchoGuard();
};

#endif