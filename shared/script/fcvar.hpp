/*
 * fcvar.hpp
 * Copyright (c) 2021, Kirill GPRB.
 * All Rights Reserved.
 */
#pragma once
#include <common/math/types.hpp>

using fcvar_t = uint32_t;
constexpr static const fcvar_t FCVAR_ARCHIVE    = (1 << 0); // Dumped to init.js on shutdown
constexpr static const fcvar_t FCVAR_READONLY   = (1 << 1); // Writable only during init.js and user.js
