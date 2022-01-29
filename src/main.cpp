#include "automanager.h"
#include "configmanager.h"

namespace {

void EnableVirtualTerminal()
{
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(handle, &dwMode);
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(handle, dwMode);
}

}


int main()
{   
    try
    {
        EnableVirtualTerminal();
    
        AutoManager::log(fmt::format("current path: {}\n", fs::current_path().string()));
        auto configPath = fs::current_path() / "updateConfigV2.json";
        if (!fs::exists(configPath))
        {
            throw fs::filesystem_error(fmt::format("configure file: {} does not exists!", configPath.string()), std::error_code{});
        }

        ConfigManager mgr;
        mgr.loadConfigFile(configPath.string());
        mgr.call();
    }
    catch (std::exception& e)
    {
        AutoManager::log(fmt::format(fg(fmt::terminal_color::bright_red), "{}\n", e.what()));
    }

}

