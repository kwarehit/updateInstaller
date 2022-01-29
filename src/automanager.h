#pragma once

#pragma warning(disable : 4996)

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <filesystem>
#include <list>
#include <utility>
#include <functional>
#include <vector>
#include <map>
#include <initializer_list>

#include <fmt/color.h>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <fmt/printf.h>
#include <fmt/core.h>

#include <boost/process.hpp>


namespace fs = std::filesystem;
namespace bp = boost::process;



enum class FuncOptionResult; 
enum class FuncOptions;
class ConfigManager;

class AutoManager 
{
    friend ConfigManager;

public:

    template <FuncOptions E>
    void handler();

    static void printInfo(FuncOptions opt, FuncOptionResult res);

    static void log(const std::string& str);

    bool getUse() const 
    {
        return use_;
    }

private:
    std::string getInstallerDirectory();
    std::string getInstallerFileName();
    std::list<std::string> getOtherInstallerFileNames();
    std::string getInstallerTestName();
    std::string getTestDiretory();
    std::string getObjDiretory();
    std::string getModPrefixDiretory();
    std::string getModDiretory();
    std::string getExtracter();
    std::string getComponentsPath();
    std::string getExtracterCommand(const std::string zipFile, const std::string destName);
    std::string getApexTestCommand();
    std::string getBuildCommand();

    void removeDirectory(const std::string dir);
    void removeDirectory(const fs::path& dir, unsigned long& counts); 
    std::string getDisplayFileName(const fs::path& file);
    void shell(const std::string cmd);
    void copyFile(const std::string sourcePath, const std::string destPath);

    bool use_;
    std::string codeLine_;
    std::string output_;
    std::string versionNumber_;
    std::list<std::string> buildTypes_;
    fs::path rootDir_;
    fs::path extractorPath_;
    fs::path componentsPath_;

    std::map<FuncOptions, std::pair<std::function<void(void)>, bool>> funcExec_;
};




