/*
 * script_engine.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/filesystem.hpp>
#include <common/traits.hpp>
#include <duktape.h>

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
    void eval(const std::string &source);
    void exec(const stdfs::path &path);
    detail::ScriptObjectBuilder build(const std::string &name);

    constexpr inline duk_context *get() const
    {
        return context;
    }

private:
    duk_context *context;
};


