/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2023, Daniel Roux, EPSITEC SA & TerranovaTeam
 * http://epsitec.ch; http://colobot.info; http://github.com/colobot
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://gnu.org/licenses
 */

#pragma once

#include "CBot/stdlib/Compilation.h"

#include "CBot/context/cbot_user_pointer.h"

#include <filesystem>
#include <memory>
#include <unordered_map>

namespace CBot
{

class CBotContext;

void InitErrorConstants(CBotContext& context);
void InitFileIOLibrary(CBotContext& context);
void InitMathLibrary(CBotContext& context);
void InitStringFunctions(CBotContext& context);

class CBotFile
{
public:
    virtual ~CBotFile() {}

    virtual bool Opened() = 0;
    virtual bool Errored() = 0;
    virtual bool IsEOF() = 0;

    virtual std::string ReadLine() = 0;
    virtual void Write(const std::string& s) = 0;
};

class CBotFileAccessHandler
{
public:
    virtual ~CBotFileAccessHandler() {}

    enum class OpenMode : char { Read = 'r', Write = 'w', Append = 'a' };
    virtual std::unique_ptr<CBotFile> OpenFile(const std::filesystem::path& filename, OpenMode mode) = 0;
    virtual bool DeleteFile(const std::filesystem::path& filename) = 0;

    int m_nextFileId = 1;

    std::unordered_map<int, std::unique_ptr<CBotFile>> m_files;
};

// TODO: provide default implementation of CBotFileAccessHandler

} // namespace CBot
