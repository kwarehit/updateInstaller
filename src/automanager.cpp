#include "automanager.h"

#include <regex>
#include <chrono>
#include <thread>
#include <boost/algorithm/string.hpp>

#include "funcoptions.h"

namespace{
    const char* HIDE_CURSOR = "\x1b[?25l";
    
    class SpendTime
    {
    public:
        SpendTime() 
        {
            m_start = std::chrono::high_resolution_clock::now();
        }
    
        ~SpendTime() 
        {
            auto currentTime = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::duration<double, std::chrono::minutes::period>>(currentTime - m_start);
            AutoManager::log(fmt::format(fg(fmt::terminal_color::bright_white), "durations: took {} mins\n", elapsed.count()));
        }
    
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> m_start;
    };
}



void AutoManager::log(const std::string& str) 
{
    std::cout << str;
}

void AutoManager::printInfo(FuncOptions opt, FuncOptionResult res) 
{
    auto&& keyToInfo = FuncMap.getKeyToInfo();
    
    if (auto it = keyToInfo.find(opt); it != keyToInfo.cend()) 
    {
        switch (res)
        {
            case FuncOptionResult::FuncSuccess:
                log(fmt::format(fg(fmt::terminal_color::bright_green), "{} have been finished!\n", it->second));
                break;
            case FuncOptionResult::FuncFailed:
                log(fmt::format(fg(fmt::terminal_color::bright_red), "{} have been failed!\n", it->second));
                break;
            case FuncOptionResult::FuncSkipped:
                log(fmt::format(fg(fmt::terminal_color::bright_yellow), "{} have been skipped!\n", it->second));
                break;
        }
    }
}

std::string AutoManager::getInstallerDirectory()
{
    return fmt::format(R"(\\rbcview\devl\builds\predator\installers\{}\{}\)"
        , codeLine_
        , versionNumber_);
}

std::string AutoManager::getModPrefixDiretory() 
{
    return fmt::format(R"({}\{}_output\scons\debug)", rootDir_.string(), output_);
}

std::string AutoManager::getModDiretory() 
{
    return (fs::path(getModPrefixDiretory()) / "mod").string();
}

std::string AutoManager::getExtracter() 
{
    return fmt::format(R"("{}" x)", extractorPath_.string());
}

std::string AutoManager::getApexTestCommand() 
{
    return (fs::path(getModDiretory()) / "runMSC_ApexTest.bat").string();
}

std::string AutoManager::getBuildCommand() 
{
    auto exec = fmt::format(R"("{}\{}\sand.bat")", rootDir_.string(), output_);

    std::list<std::string> typeCmds;
    for (auto&& buildType : buildTypes_) 
    {
        typeCmds.emplace_back(fmt::format(" {{0}} scons -t {} ", buildType));
    }

    auto cmd = fmt::format("{} -b debug env &&", exec) + fmt::format(boost::join(typeCmds, "&&"), exec);
    return cmd;
}

std::string AutoManager::getExtracterCommand(const std::string zipFile, const std::string destName) 
{
    return fmt::format("{} {} {}{}"
        , getExtracter()
        , zipFile
        , "-aoa -o"
        , destName);
}

std::string AutoManager::getInstallerFileName()
{
    return fmt::format("cl{}-vtune.7z", versionNumber_);
}

std::list<std::string> AutoManager::getOtherInstallerFileNames()
{
    std::list<std::string> fileNames;
    for(auto name : {"pdb", "comp"})
    {
        if(auto anotherFile = fmt::format("cl{}-vtune-{}.7z", versionNumber_, name)
            ; fs::exists(fs::path(getInstallerDirectory()) / anotherFile))
        {
            fileNames.emplace_back(anotherFile);
        }
    }
    
    return fileNames;
}

std::string AutoManager::getInstallerTestName()
{
    return fmt::format("cl{}-vtune-tests.7z", versionNumber_);
}

std::string AutoManager::getTestDiretory() 
{
    return (fs::path(getModPrefixDiretory()) / "tests").string();
}

std::string AutoManager::getObjDiretory()
{
    return (fs::path(getModPrefixDiretory()) / "obj").string();
}

std::string AutoManager::getComponentsPath()
{
    return componentsPath_.string();
}

void AutoManager::removeDirectory(const std::string dir) 
{
    try
    {
        if(!fs::exists(dir))
        {
            log(fmt::format(fg(fmt::terminal_color::bright_white), "\nDirectory {} is not exist\n", dir));
            return;
        }
        
        log(fmt::format("start deleting directory: {}\n", dir));

        unsigned long counts{};
        removeDirectory(dir, counts);
        log(fmt::format(fg(fmt::terminal_color::bright_white), "\nthe total of {} files were deleted\n", counts));
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}

void AutoManager::removeDirectory(const fs::path& dir, unsigned long& counts)
{
    try
    {
        for (auto&& p : fs::directory_iterator(dir)) 
        {
            if (fs::is_directory(p.path()))
            {
                removeDirectory(p.path(), counts);
            }

            if ((p.status().permissions() & fs::perms::others_read) != fs::perms::none)
            {
                fs::permissions(p, fs::perms::others_write);
            }

            fs::remove(p.path());

            log(fmt::format(fg(fmt::terminal_color::bright_cyan)
                ,"\r{}: {:<110.110}"
                , ++counts
                , getDisplayFileName(p.path())));

            log(HIDE_CURSOR);
        }
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}

std::string AutoManager::getDisplayFileName(const fs::path& file)
{
    try
    {
        return fs::relative(file, fs::path(getModPrefixDiretory())).string();
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}

void AutoManager::shell(const std::string cmd) 
{
    bp::system(cmd);
}

void AutoManager::copyFile(const std::string sourcePath, const std::string destPath)
{
    auto copyProgress = [](
        LARGE_INTEGER TotalFileSize,            
        LARGE_INTEGER TotalBytesTransferred,    
        LARGE_INTEGER StreamSize,               
        LARGE_INTEGER StreamBytesTransferred,   
        DWORD dwStreamNumber,                   
        DWORD dwCallbackReason,                 
        HANDLE hSourceFile,                     
        HANDLE hDestinationFile,                
        LPVOID lpData                           
    )->DWORD
    {
        long long trans = TotalBytesTransferred.QuadPart;
        long long totalSize = TotalFileSize.QuadPart;
    
        log(fmt::format("\r{}%", trans*100/totalSize));
    
        return PROGRESS_CONTINUE;
    };

    CopyFileExA(sourcePath.c_str()
        , destPath.c_str()
        , (LPPROGRESS_ROUTINE)copyProgress
        , 0, 0, COPY_FILE_OPEN_SOURCE_FOR_WRITE);
        
    log("\n");
}

template <>
void AutoManager::handler<FuncOptions::RemoveInstallerDir>()
{
    try
    {
        SpendTime t;
        removeDirectory(getModDiretory());
    }
    catch (std::exception&)
    {
        throw;
    }
}

template <>
void AutoManager::handler<FuncOptions::CopyInstaller>() 
{
    try
    {
        SpendTime t;
        std::string sourcePath = (fs::path(getInstallerDirectory()) / getInstallerFileName()).string();
        std::string targetPath = (fs::path(getModDiretory()) / getInstallerFileName()).string();

        log(fmt::format("start downloading installer from {}\n", sourcePath));

        copyFile(sourcePath, targetPath);
        
        for(auto&& name : getOtherInstallerFileNames())
        {
            std::string srcPath = (fs::path(getInstallerDirectory()) / name).string();
            std::string desPath = (fs::path(getModDiretory()) / name).string();
    
            log(fmt::format("start downloading file from {}\n", srcPath));
    
            copyFile(srcPath, desPath);
        }
        
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}

template <>
void AutoManager::handler<FuncOptions::UnzipInstaller>() 
{
    try
    {
        std::string sourcePath = (fs::path(getModDiretory()) / getInstallerFileName()).string();
        std::string targetPath = getModDiretory();

        shell(getExtracterCommand(sourcePath, targetPath));
        
        for(auto&& name : getOtherInstallerFileNames())
        {
            std::string srcPath = (fs::path(getModDiretory()) / name).string();
            std::string desPath = getModDiretory();
    
            shell(getExtracterCommand(srcPath, desPath));
        }
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}

template <>
void AutoManager::handler<FuncOptions::MoveInstallerToParentDir>() 
{
    try
    {
        using namespace std::chrono_literals;
        
        std::this_thread::sleep_for(2s);
        
        fs::path parentPath = fs::path(getModDiretory());
        fs::path sourcePath = parentPath / "mod";

        std::list<fs::path> subPaths;
        for(auto&& p : fs::directory_iterator(sourcePath))
        {
            fs::rename(p.path(), parentPath/p.path().filename());
        }
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}

template<>
void AutoManager::handler<FuncOptions::RemoveTestCasesDir>() 
{
    try
    {
        SpendTime t;
        removeDirectory(getTestDiretory());
    }
    catch (std::exception&)
    {
        throw;
    }
}

template<>
void AutoManager::handler<FuncOptions::RemoveTestFile>() 
{
    try
    {
        std::regex pattern{"cl\\d+-vtune-tests\\.7z"};
        
        for (auto&& entry : fs::directory_iterator(fs::path(getModPrefixDiretory())))
        {
            if (fs::is_regular_file(entry.path()) && std::regex_match(entry.path().filename().string(), pattern))
            {
                fs::remove(entry.path());
            }
        }
    }
    catch (std::exception&)
    {
        throw;
    }
}

template<>
void AutoManager::handler<FuncOptions::CopyTestCasesInstaller>() 
{
    try
    {
        SpendTime t;
        std::string sourcePath = (fs::path(getInstallerDirectory()) / getInstallerTestName()).string();
        std::string targetPath = (fs::path(getModPrefixDiretory()) / getInstallerTestName()).string();

        log(fmt::format("start downloading testcase installer from {}\n", sourcePath));

        copyFile(sourcePath, targetPath);
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}

template <>
void AutoManager::handler<FuncOptions::UnzipTestCasesInstaller>() 
{
    try
    {
        std::string sourcePath = (fs::path(getModPrefixDiretory()) / getInstallerTestName()).string();
        std::string targetPath = getModPrefixDiretory();

        shell(getExtracterCommand(sourcePath, targetPath));
    }
    catch (fs::filesystem_error&)
    {
        throw;
    }
}


template <>
void AutoManager::handler<FuncOptions::RemoveComponents>()
{
    try
    {
        SpendTime t;
        removeDirectory(getComponentsPath());
    }
    catch (std::exception&)
    {
        throw;
    }
}

template <>
void AutoManager::handler<FuncOptions::RemoveObjFileDir>() 
{
    try
    {
        SpendTime t;
        std::list<std::string> subDirName = { 
                  "WIN8664_SOURCEPOST_DEBUG"
                , "WIN8664_POSTPLUGIN_DEBUG"
                , "PostPluginUI" 
                , "post"
                , "post_plugin"
                , "post_plugin_ui"
                , "uipost_plugin"
                , "modeling_plugin"
                , "modeling_plugin_ui"
                , "ModelingPluginUI"
            };

        for (auto&& name : subDirName)
        {
            removeDirectory((fs::path(getObjDiretory()) / name).string());
        }
    }
    catch (std::exception&)
    {
        throw;
    }
}

template <>
void AutoManager::handler<FuncOptions::BuildPost>() 
{
    shell(getBuildCommand());
}

template <>
void AutoManager::handler<FuncOptions::RunApex>() 
{
    shell(getApexTestCommand());
}