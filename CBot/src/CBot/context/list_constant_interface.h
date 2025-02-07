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

#include "CBot/context/list_constant.h"

namespace CBot
{

class CBotListConstantInterface
{
protected:
    ~CBotListConstantInterface() {}

public:
    template<typename T>
    bool AddConstant(const std::string& name, const T& value)
    {
        return m_listConstant.AddConstant(name, value);
    }

    virtual bool IsDefinedConstant(const std::string& name) const = 0;

    virtual const CBotVarUPtr& GetDefinedConstant(const std::string& name) = 0;

protected:
   CBotListConstant m_listConstant;
};

} // namespace CBot
