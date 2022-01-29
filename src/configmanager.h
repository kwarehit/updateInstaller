#pragma once

#include <fstream>
#include <string>
#include <map>
#include <memory>

class AutoManager;

class ConfigManager
{
public:
    void loadConfigFile(const std::string file);
    void call();

private:
    std::map<std::string, std::shared_ptr<AutoManager>> codelineMgr_;
};

