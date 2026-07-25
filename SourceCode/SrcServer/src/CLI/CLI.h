#ifndef CLI_H
#define CLI_H

#include <string>
#include <atomic>

class CLI
{
private:
    std::atomic_bool* done;

    struct command
    {
    };

public:
    CLI();
    ~CLI();

    bool run(std::atomic_bool* running);
    bool process(const std::string& cmd);

    void handleHelpCommand();
    void handleExitCommand();
    void handleShutdownCommand();
    void handleConnectedCommand();
    void handleKickCommand(const std::string& cmd);

    void handlePrintGMSCommand();
    void handleReloadGMSCommand();
    void handleReloadHotuk();
    void handleBossTIME();
    void handleBossTIMEChange();

    static CLI* getInstance();
};

#endif // CLI_H