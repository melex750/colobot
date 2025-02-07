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

#include "CBot/context/list_constant.h"

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

CBotListConstant::CBotListConstant()
{
}

CBotListConstant::~CBotListConstant()
{
}

bool CBotListConstant::IsDefinedConstant(const std::string& name) const
{
    return m_list.count(name) != 0;
}

const CBotVarUPtr& CBotListConstant::GetDefinedConstant(const std::string& name)
{
    return m_list[name];
}

template<>
bool CBotListConstant::AddConstant<int>(const std::string& name, const int& value)
{
    auto& pt = m_list[name];
    if ( pt != nullptr ) return false;

    pt.reset(CBotVar::Create("", CBotTypInt));
    pt->SetName(name);
    pt->SetValInt(value, name);
    return true;
}

template<>
bool CBotListConstant::AddConstant<float>(const std::string& name, const float& value)
{
    auto& pt = m_list[name];
    if ( pt != nullptr ) return false;

    pt.reset(CBotVar::Create("", CBotTypFloat));
    pt->SetName(name);
    *pt = value;
    return true;
}

template<>
bool CBotListConstant::AddConstant<std::string>(const std::string& name, const std::string& value)
{
    auto& pt = m_list[name];
    if ( pt != nullptr ) return false;

    pt.reset(CBotVar::Create("", CBotTypString));
    pt->SetName(name);
    *pt = value;
    return true;
}

} // namespace CBot
