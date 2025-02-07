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

#include "CBot/CBotInstr/CBotInstr.h"

#include "CBot/CBotTypResult.h"

namespace CBot
{


CBotInstr* CompileExprConstant(CBotToken* &p, CBotCStack* pStack);
/**
 * \brief A predefined constant (see CBotListConstant::AddConstant())
 *
 * Can be of type:
 * ::CBotTypByte
 * ::CBotTypShort
 * ::CBotTypChar
 * ::CBotTypInt
 * ::CBotTypLong
 * ::CBotTypFloat
 * ::CBotTypDouble
 * ::CBotTypString
 */
template<typename T>
class CBotExprConstant : public CBotInstr
{

public:
    CBotExprConstant(T val)
    {
        static_assert(sizeof(T) == 0, "Only specializations of CBotExprConstant can be used");
    }

    ~CBotExprConstant();

    bool Execute(CBotStack* &pj) override;

    void RestoreState(CBotStack* &pj, bool bMain) override;

protected:
    virtual const std::string GetDebugName() override { return "CBotExprConstant"; }
    virtual std::string GetDebugData() override;

private:
    CBotTypResult m_type;
    //! Value
    const T m_value;
};

} // namespace CBot
