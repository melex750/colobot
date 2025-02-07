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

#include "CBot/CBotInstr/CBotExprConstant.h"

#include "CBot/CBotCStack.h"
#include "CBot/CBotStack.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/context/cbot_context.h"

namespace CBot
{

template<>
CBotExprConstant<signed char>::CBotExprConstant(signed char val) : m_type(CBotTypByte), m_value(val)
{}

template<>
CBotExprConstant<short>::CBotExprConstant(short val) : m_type(CBotTypShort), m_value(val)
{}

template<>
CBotExprConstant<char32_t>::CBotExprConstant(char32_t val) : m_type(CBotTypChar), m_value(val)
{}

template<>
CBotExprConstant<int>::CBotExprConstant(int val) : m_type(CBotTypInt), m_value(val)
{}

template<>
CBotExprConstant<long>::CBotExprConstant(long val) : m_type(CBotTypLong), m_value(val)
{}

template<>
CBotExprConstant<float>::CBotExprConstant(float val) : m_type(CBotTypFloat), m_value(val)
{}

template<>
CBotExprConstant<double>::CBotExprConstant(double val) : m_type(CBotTypDouble), m_value(val)
{}

template<>
CBotExprConstant<std::string>::CBotExprConstant(std::string val) : m_type(CBotTypString), m_value(val)
{}

template<typename T>
CBotExprConstant<T>::~CBotExprConstant()
{}

CBotInstr* CompileExprConstant(CBotToken* &p, CBotCStack* pStack)
{
    CBotCStack* pStk = pStack->TokenStack();

    const auto& name = p->GetString();

    if (p->GetType() != TokenTypDef)
    {
        assert(false);
        return nullptr;
    }

    CBotInstr* inst = nullptr;

    auto context = pStack->GetContext();
    const auto& var = context->GetDefinedConstant(name);
    if (var == nullptr)
    {
        assert(false);
        return nullptr;
    }

    switch (var->GetType())
    {
        case CBotTypByte:
            inst = new CBotExprConstant<signed char>(*var); break;

        case CBotTypShort:
            inst = new CBotExprConstant<short>(*var); break;

        case CBotTypChar:
            inst = new CBotExprConstant<char32_t>(*var); break;

        case CBotTypInt:
            inst = new CBotExprConstant<int>(*var); break;

        case CBotTypLong:
            inst = new CBotExprConstant<long>(*var); break;

        case CBotTypFloat:
            inst = new CBotExprConstant<float>(*var); break;

        case CBotTypDouble:
            inst = new CBotExprConstant<double>(*var); break;

        case CBotTypString:
            inst = new CBotExprConstant<std::string>(*var); break;

        default:
            assert(false);
            return nullptr;
    }

    inst->SetToken(p);
    p = p->GetNext();
    pStk->SetCopyVar(var.get());
    return pStack->Return(inst, pStk);
}

template<>
bool CBotExprConstant<std::string>::Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar* var = CBotVar::Create("", m_type);

    *var = m_value;
    pile->SetVar(var);
    return pj->Return(pile);
}

template<typename T>
bool CBotExprConstant<T>::Execute(CBotStack* &pj)
{
    CBotStack* pile = pj->AddStack(this);

    if (pile->IfStep()) return false;

    CBotVar* var = CBotVar::Create("", m_type);

    if (m_type.Eq(CBotTypInt))
    {
        var->SetValInt(m_value, m_token.GetString());
    }
    else
    {
        *var = m_value;
    }
    pile->SetVar(var);
    return pj->Return(pile);
}

template<typename T>
void CBotExprConstant<T>::RestoreState(CBotStack* &pj, bool bMain)
{
    if (bMain) pj->RestoreStack(this);
}

template<typename T>
std::string CBotExprConstant<T>::GetDebugData()
{
    return m_token.GetString();
}

} // namespace CBot
