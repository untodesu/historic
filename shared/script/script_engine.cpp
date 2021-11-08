/*
 * script_engine.cpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#include <exception>
#include <shared/script/cvars.hpp>
#include <shared/script/script_engine.hpp>
#include <spdlog/spdlog.h>
#include <sstream>

static void onFatalError(void *, const char *message)
{
    spdlog::critical("ScriptEngine: {}", message);
    std::terminate();
}

static duk_ret_t onScriptExec(duk_context *context)
{
    const stdfs::path path = duk_safe_to_string(context, 0);
    std::string source;
    if(!fs::readText(path, source))
        return duk_eval_error(context, "unable to read %s", path.string().c_str());
    duk_peval_string(context, source.c_str());
    return 0;
}

static inline duk_ret_t onLogWhatever(spdlog::level::level_enum level, duk_context *context)
{
    if(const duk_idx_t argc = duk_get_top(context)) {
        std::stringstream ss;
        for(duk_idx_t i = 0; i < argc; i++)
            ss << duk_safe_to_string(context, i);
        spdlog::log(level, ss.str());
    }

    return 0;
}

static duk_ret_t onLogInfo(duk_context *context) { return onLogWhatever(spdlog::level::info, context); }
static duk_ret_t onLogWarn(duk_context *context) { return onLogWhatever(spdlog::level::warn, context); }
static duk_ret_t onLogError(duk_context *context) { return onLogWhatever(spdlog::level::err, context); }

static duk_ret_t onCVarGet(duk_context *context)
{
    const CVarBase *cvar = cvars::find(duk_safe_to_string(context, 0));
    if(cvar) {
        duk_push_string(context, cvar->get().c_str());
        return 1;
    }

    duk_push_null(context);
    return 1;
}

static duk_ret_t onCVarSet(duk_context *context)
{
    CVarBase *cvar = cvars::find(duk_safe_to_string(context, 0));
    if(cvar)
        cvar->set(duk_safe_to_string(context, 1));
    return 0;
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

ScriptEngine::ScriptEngine()
    : context(duk_create_heap(nullptr, nullptr, nullptr, nullptr, &onFatalError))
{
    build("CVar")
        .function("get", &onCVarGet, 1)
        .function("set", &onCVarSet, 2)
        .submit();

    build("Log")
        .function("info", &onLogInfo, DUK_VARARGS)
        .function("warn", &onLogWarn, DUK_VARARGS)
        .function("error", &onLogError, DUK_VARARGS)
        .submit();

    build("Script")
        .function("exec", &onScriptExec, 1)
        .submit();
}

ScriptEngine::~ScriptEngine()
{
    duk_destroy_heap(context);
}

void ScriptEngine::eval(const std::string &source)
{
    if(!duk_peval_string(context, source.c_str()))
        return;
    spdlog::error(duk_safe_to_string(context, -1));
    duk_pop(context);
}

void ScriptEngine::exec(const stdfs::path &path)
{
    std::string source;
    if(!fs::readText(path, source)) {
        spdlog::warn("Unable to read {}", path.string());
        return;
    }

    if(!duk_peval_string(context, source.c_str()))
        return;
    spdlog::error(duk_safe_to_string(context, -1));
    duk_pop(context);
}

detail::ScriptObjectBuilder ScriptEngine::build(const std::string &name)
{
    return detail::ScriptObjectBuilder(context, name);
}
