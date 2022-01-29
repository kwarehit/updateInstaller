#include "configmanager.h"

#include <boost/json/src.hpp>

#include "automanager.h"
#include "funcoptions.h"

namespace json = boost::json;

namespace {
    
json::value readJson( std::istream& is, json::error_code& ec )
{
    json::stream_parser p;
    for(std::string line; std::getline( is, line ); )
    {
        p.write( line, ec );
        if (ec) 
        {
            return nullptr;
        }
    }

    p.finish( ec );

    if (ec) 
    {
        return nullptr;
    }

    return p.release();
}

}

struct ConfigFile 
{
    bool use_;
    std::string codeLine_;
    std::string output_;
    std::string versionNumber_;
    std::string rootDir_;
    std::map<std::string, bool> funcStatus_;
    std::list<std::string> buildTypes_;
};

ConfigFile tag_invoke( json::value_to_tag< ConfigFile >, const json::value& jv )
{
    const json::object& obj = jv.as_object();
    return ConfigFile {
          json::value_to<bool>(obj.at( "use" ) )
        , json::value_to<std::string>( obj.at( "config" ).as_object().at("code_line") )
        , json::value_to<std::string>( obj.at( "config" ).as_object().at("output") )
        , json::value_to<std::string>( obj.at( "config" ).as_object().at("version_num") )
        , json::value_to<std::string>( obj.at( "config" ).as_object().at("root_dir") )
        , json::value_to<std::map<std::string, bool>>( obj.at( "config" ).as_object().at("func_options") )
        , json::value_to<std::list<std::string>>( obj.at( "config" ).as_object().at("build_type"))
    };
}

void ConfigManager::loadConfigFile(const std::string file) 
{
    try
    {
        std::ifstream inConfig(file, std::ios::in);

        json::error_code ec;
        auto val = readJson(inConfig, ec);
        if(ec)
        {
            inConfig.close();
            throw std::system_error(ec);
        }
        inConfig.close();
        
        auto&& j = val.as_object();

        fs::path extractorPath = fs::canonical(fs::path(json::value_to<std::string>(j.at("7z_path"))));
        fs::path componentsPath = fs::canonical(fs::path(json::value_to<std::string >(j.at("components_path"))));
        for (auto&&kv : j) 
        {
            auto k = kv.key();
            auto v = kv.value();
            
            if (k == "7z_path" || k == "components_path") 
            {
                continue;
            }

            ConfigFile config{ json::value_to<ConfigFile>(kv.value()) };

            auto autoMgr = std::make_shared<AutoManager>();
            autoMgr->use_ = config.use_;
            autoMgr->rootDir_ =  fs::canonical(fs::path(config.rootDir_));
            autoMgr->codeLine_ = config.codeLine_;
            autoMgr->versionNumber_ = config.versionNumber_;
            autoMgr->buildTypes_ = config.buildTypes_;
            autoMgr->output_ = config.output_;
            autoMgr->extractorPath_ = extractorPath;
            autoMgr->componentsPath_ = componentsPath;

            auto&& keyToOptions = FuncMap.getkeyToOptions();
            for (auto&& item : config.funcStatus_) 
            {
                if (auto it = keyToOptions.find(item.first); it != keyToOptions.cend())
                {
                    autoMgr->funcExec_.emplace(it->second.first, std::pair(std::bind(it->second.second, autoMgr), item.second));
                }
            }

            codelineMgr_.emplace(k, autoMgr);
        }
    }
    catch (std::exception&e)
    {
        AutoManager::log(fmt::format(fg(fmt::terminal_color::bright_red), "{}\n", e.what()));
    }
}

void ConfigManager::call() 
{
    FuncOptions opt{};
    try
    {
        for (auto&& item : codelineMgr_)
        {
            if (!item.second->getUse())
            {
                continue;
            }

            for (auto&& fn : item.second->funcExec_)
            {
                opt = fn.first;
                if (fn.second.second)
                {
                    fn.second.first();
                    AutoManager::printInfo(fn.first, FuncOptionResult::FuncSuccess);
                }
                else 
                {
                    AutoManager::printInfo(fn.first, FuncOptionResult::FuncSkipped);
                }
            }
        }
    }
    catch (std::exception& e)
    {
        AutoManager::printInfo(opt, FuncOptionResult::FuncFailed);
        AutoManager::log(fmt::format(fg(fmt::terminal_color::bright_red), "{}\n", e.what()));
    }

}