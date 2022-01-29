#pragma once

#include "automanager.h"

enum class FuncOptions
{
    RemoveInstallerDir,
    CopyInstaller,
    UnzipInstaller,
    MoveInstallerToParentDir,

    RemoveTestCasesDir,
    RemoveTestFile,
    CopyTestCasesInstaller,
    UnzipTestCasesInstaller,
    
    RemoveComponents,
    RemoveObjFileDir,
    BuildPost,
    RunApex,
    FuncOptionsNum,
};

enum class FuncOptionResult 
{
    FuncSuccess,
    FuncFailed,
    FuncSkipped,
};

template <FuncOptions>
struct FuncOptionsTraits;

#define FUNC_OPTIONS_TRAITS(E, KeyStr, InfoStr)                     \
template<>                                                          \
struct FuncOptionsTraits<FuncOptions::E>                            \
{                                                                   \
    static constexpr const char* infoString = InfoStr;              \
    static constexpr const char* keyString = KeyStr;                \
};                                                                  \


FUNC_OPTIONS_TRAITS(RemoveInstallerDir,       "remove_installerDir"         , "1.  remove installer directory"        )
FUNC_OPTIONS_TRAITS(CopyInstaller,            "copy_installer"              , "2.  copy installer"                    )
FUNC_OPTIONS_TRAITS(UnzipInstaller,           "unzip_installer"             , "3.  unzip installer"                   )
FUNC_OPTIONS_TRAITS(MoveInstallerToParentDir, "move_installer_to_parentdir" , "4.  move installer to parent directory")
FUNC_OPTIONS_TRAITS(RemoveTestCasesDir,       "remove_testcasesdir"         , "5.  remove testcase directory"         )
FUNC_OPTIONS_TRAITS(RemoveTestFile,           "remove_testcaseinstaller"    , "6.  remove testcase installer"         )
FUNC_OPTIONS_TRAITS(CopyTestCasesInstaller,   "copy_testcaseinstaller"      , "7.  copy testcase installer"           )
FUNC_OPTIONS_TRAITS(UnzipTestCasesInstaller,  "unzip_testcaseinstaller"     , "8.  unzip testcase installer"          )
FUNC_OPTIONS_TRAITS(RemoveComponents,         "remove_componentsdir"        , "9.  remove components files"           )
FUNC_OPTIONS_TRAITS(RemoveObjFileDir,         "remove_objdir"               , "10. remove object files"               )
FUNC_OPTIONS_TRAITS(BuildPost,                "build_post"                  , "11. build post"                        )
FUNC_OPTIONS_TRAITS(RunApex,                  "run_apex"                    , "12. run apex"                          )

struct FuncOptionsMap
{
    FuncOptionsMap()
    {
        initInfoMap<
            FuncOptions::RemoveInstallerDir,
            FuncOptions::CopyInstaller,
            FuncOptions::UnzipInstaller,
            FuncOptions::MoveInstallerToParentDir,
            FuncOptions::RemoveTestCasesDir,
            FuncOptions::RemoveTestFile,
            FuncOptions::CopyTestCasesInstaller,
            FuncOptions::UnzipTestCasesInstaller,
            FuncOptions::RemoveComponents,
            FuncOptions::RemoveObjFileDir,
            FuncOptions::BuildPost,
            FuncOptions::RunApex
        >();
    }
    
    const auto& getKeyToInfo()
    {
        return keyToInfo_;
    }
    
    const auto& getkeyToOptions()
    {
        return keyToOptions_;
    }
    
private:
    template<FuncOptions... E>
    void initInfoMap()
    {
        keyToInfo_ = {
            {E, FuncOptionsTraits<E>::infoString}...
        };
        
        keyToOptions_ = {
            {FuncOptionsTraits<E>::keyString, {E, &AutoManager::handler<E>}}...
        };
    }

    std::map<FuncOptions, std::string> keyToInfo_;
    std::map<std::string, std::pair<FuncOptions, void(AutoManager::*)(void)>> keyToOptions_;
};

template <typename T = FuncOptionsMap>
struct StaticInstance
{
    static T instance;
};

template <typename T>
T StaticInstance<T>::instance = {};

static FuncOptionsMap& FuncMap = StaticInstance<>::instance;