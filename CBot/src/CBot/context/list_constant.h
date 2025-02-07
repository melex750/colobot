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

#include <memory>
#include <string>
#include <unordered_map>

namespace CBot
{

class CBotVar;

using CBotVarUPtr = std::unique_ptr<CBotVar>;

class CBotListConstant
{
public:
    CBotListConstant();

    ~CBotListConstant();

    template<typename T>
    bool AddConstant(const std::string& name, const T& value)
    {
        static_assert(sizeof(T) == 0, "Only specializations of AddConstant can be used");
        return false;
    }

    bool IsDefinedConstant(const std::string& name) const;

    const CBotVarUPtr& GetDefinedConstant(const std::string& name);

private:
   std::unordered_map<std::string, CBotVarUPtr> m_list;
};

template<>
bool CBotListConstant::AddConstant<int>(const std::string& name, const int& value);

template<>
bool CBotListConstant::AddConstant<float>(const std::string& name, const float& value);

template<>
bool CBotListConstant::AddConstant<std::string>(const std::string& name, const std::string& value);

} // namespace CBot
