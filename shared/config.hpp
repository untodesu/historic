/*
 * config.hpp
 * Author: Kirill GPRB
 * Created: Sun Dec 12 2021 15:55:10
 */
#pragma once
#include <stdexcept>
#include <common/filesystem.hpp>
#include <spdlog/spdlog.h>
#include <toml.hpp>
#include <sstream>

template<typename T>
class BaseConfig {
public:
    void read(const stdfs::path &path);
    void write(const stdfs::path &path);

    // Implementations define:
    //  void implPostRead()
    //  void implPreWrite()

protected:
    toml::table toml;
};

template<typename T>
inline void BaseConfig<T>::read(const stdfs::path &path)
{
    try {
        std::string source;
        if(!fs::readText(path, source))
            throw std::runtime_error("unable to read file");
        toml = toml::parse(source);
    }
    catch(const std::exception &ex) {
        spdlog::error("Parsing {} failed: {}", path.string(), ex.what());
        toml = toml::table();
    }

    static_cast<T *>(this)->implPostRead();
}

template<typename T>
inline void BaseConfig<T>::write(const stdfs::path &path)
{
    static_cast<T *>(this)->implPreWrite();
    fs::writeText(path, (std::stringstream() << toml).str());
}
