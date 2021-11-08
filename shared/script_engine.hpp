/*
 * script_engine.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/filesystem.hpp>
#include <common/traits.hpp>
#include <duktape.h>

// Here's a quick explanation how scripting system works.
//
//  Scripting system in Voxelius IS NOT a tool to make
//  mods or whatever like that (mods will be available for
//  servers in form of native modules (SO or DLL files)).
//
//  Scripting system in Voxelius IS a way for users to make
//  some tweaks to the configuration of the application.
//  For the client-side I have some plans for actual add-ons
//  that should not affect the performance in any way.
//
//  This header provides a bunch of base classes that are
//  used to make defining new functions and constants easier.
//  Builder pattern is widely used here because it's awesome.
//

namespace detail
{
class ScriptObjectBuilder final : public NonCopyable, public NonMovable {
public:
    ScriptObjectBuilder(duk_context *context, const std::string &name);

    ScriptObjectBuilder &constant(const std::string &name, int value);
    ScriptObjectBuilder &constant(const std::string &name, float value);
    ScriptObjectBuilder &constant(const std::string &name, const std::string &value);
    ScriptObjectBuilder &function(const std::string &name, duk_c_function func, duk_idx_t nargs);

    void submit();

private:
    duk_context *context;
    std::string name;
};

} // namespace detail

class ScriptEngine {
public:
    ScriptEngine();
    virtual ~ScriptEngine();

    void eval(const std::string &s);
    void exec(const stdfs::path &path);

    void constant(const std::string &name, int value);
    void constant(const std::string &name, float value);
    void constant(const std::string &name, const std::string &value);
    void function(const std::string &name, duk_c_function func, duk_idx_t nargs);
    detail::ScriptObjectBuilder object(const std::string &name);

    constexpr inline duk_context *get() const
    {
        return context;
    }

private:
    duk_context *context;
};
