/*
 * script_engine.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <exception>
#include <shared/script_engine.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

static void fatalHandler(void *, const char *message)
{
    spdlog::critical("ScriptEngine: {}", message);
    std::terminate();
}

detail::ScriptObjectBuilder::ScriptObjectBuilder(duk_context *context, const std::string &name)
    : context(context), name(name)
{
    duk_push_object(context);
}

detail::ScriptObjectBuilder &detail::ScriptObjectBuilder::constant(const std::string &name, int value)
{
    duk_push_int(context, static_cast<duk_int_t>(value));
    duk_put_prop_string(context, -2, name.c_str());
    return *this;
}

detail::ScriptObjectBuilder &detail::ScriptObjectBuilder::constant(const std::string &name, float value)
{
    duk_push_number(context, static_cast<duk_double_t>(value));
    duk_put_prop_string(context, -2, name.c_str());
    return *this;
}

detail::ScriptObjectBuilder &detail::ScriptObjectBuilder::constant(const std::string &name, const std::string &value)
{
    duk_push_string(context, value.c_str());
    duk_put_prop_string(context, -2, name.c_str());
    return *this;
}

detail::ScriptObjectBuilder &detail::ScriptObjectBuilder::function(const std::string &name, duk_c_function func, duk_idx_t nargs)
{
    duk_push_c_function(context, func, nargs);
    duk_put_prop_string(context, -2, name.c_str());
    return *this;
}

void detail::ScriptObjectBuilder::submit()
{
    duk_put_global_string(context, name.c_str());
    context = nullptr;
}

static inline void execShared(const stdfs::path &path, duk_context *ctx)
{
    std::string source;
    if(!fs::readText(path, source)) {
        spdlog::warn("Unable to read {}", path.string());
        return;
    }

    if(duk_peval_string(ctx, source.c_str())) {
        spdlog::error(duk_safe_to_string(ctx, -1));
        duk_pop(ctx);
    }
}

namespace script_api
{
static inline duk_ret_t logX(spdlog::level::level_enum level, duk_context *ctx)
{
    const duk_idx_t argc = duk_get_top(ctx);
    if(argc) {
        std::stringstream ss;
        for(duk_idx_t i = 0; i < argc; i++)
            ss << duk_safe_to_string(ctx, i);
        spdlog::log(level, ss.str());
    }

    return 0;
}

static duk_ret_t logInfo(duk_context *ctx) { return script_api::logX(spdlog::level::info, ctx); }
static duk_ret_t logWarn(duk_context *ctx) { return script_api::logX(spdlog::level::warn, ctx); }
static duk_ret_t logErr(duk_context *ctx) { return script_api::logX(spdlog::level::err, ctx); }

static duk_ret_t scriptExec(duk_context *ctx)
{
    execShared(duk_safe_to_string(ctx, 0), ctx);
    return 0;
}
} // namespace script_api

ScriptEngine::ScriptEngine()
    : context(duk_create_heap(nullptr, nullptr, nullptr, nullptr, fatalHandler))
{
    object("Log")
        .function("info", &script_api::logInfo, DUK_VARARGS)
        .function("warn", &script_api::logWarn, DUK_VARARGS)
        .function("error", &script_api::logErr, DUK_VARARGS)
        .submit();
    object("Script")
        .function("exec", &script_api::scriptExec, 1)
        .submit();
}

ScriptEngine::~ScriptEngine()
{
    duk_destroy_heap(context);
}

void ScriptEngine::eval(const std::string &s)
{
    if(!duk_peval_string(context, s.c_str()))
        return;
    spdlog::error(duk_safe_to_string(context, -1));
    duk_pop(context);
}

void ScriptEngine::exec(const stdfs::path &path)
{
    execShared(path, context);
}

void ScriptEngine::constant(const std::string &name, int value)
{
    duk_push_int(context, static_cast<duk_int_t>(value));
    duk_put_global_string(context, name.c_str());
}

void ScriptEngine::constant(const std::string &name, float value)
{
    duk_push_number(context, static_cast<duk_double_t>(value));
    duk_put_global_string(context, name.c_str());
}

void ScriptEngine::constant(const std::string &name, const std::string &value)
{
    duk_push_string(context, value.c_str());
    duk_put_global_string(context, name.c_str());
}

void ScriptEngine::function(const std::string &name, duk_c_function func, duk_idx_t nargs)
{
    duk_push_c_function(context, func, nargs);
    duk_put_global_string(context, name.c_str());
}

detail::ScriptObjectBuilder ScriptEngine::object(const std::string &name)
{
    return detail::ScriptObjectBuilder(context, name);
}
