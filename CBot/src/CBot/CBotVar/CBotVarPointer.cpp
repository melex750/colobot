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

#include "CBot/CBotVar/CBotVarPointer.h"
#include "CBot/CBotToken.h"
#include "CBot/CBotClass.h"
#include "CBot/CBotVar/CBotVarClass.h"

#include "CBot/CBotEnums.h"
#include "CBot/CBotUtils.h"

#include <cassert>

namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotVarPointer::CBotVarPointer(const CBotToken& name, CBotTypResult& type) : CBotVar(name)
{
    if ( !type.Eq(CBotTypPointer) &&
         !type.Eq(CBotTypNullPointer) &&
         !type.Eq(CBotTypClass)   &&                    // for convenience accepts Class and Intrinsic
         !type.Eq(CBotTypIntrinsic) ) assert(0);

    m_next        = nullptr;
    m_pMyThis    = nullptr;

    m_type        = type;
    if ( !type.Eq(CBotTypNullPointer) )
        m_type.SetType(CBotTypPointer);                    // anyway, this is a pointer
    m_binit        = CBotVar::InitType::UNDEF;
    m_pClass    = nullptr;
    m_pVarClass = nullptr;                                    // will be defined by a SetPointer()

    SetClass(type.GetClass());
}

////////////////////////////////////////////////////////////////////////////////
CBotVarPointer::~CBotVarPointer()
{
}

void CBotVarPointer::Update()
{
    if (m_pVarClass) m_pVarClass->Update();
}

void CBotVarPointer::SetUserPointer(void* user)
{
    if (m_pVarClass) m_pVarClass->SetUserPointer(user);
}

void CBotVarPointer::KillUserPointer()
{
    if (m_pVarClass) m_pVarClass->KillUserPointer();
}

CBotVarUserPointer CBotVarPointer::GetUserPointer()
{
    if (m_pVarClass) return m_pVarClass->GetUserPointer();
    return {};
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarPointer::GetItem(const std::string& name)
{
    if ( m_pVarClass == nullptr)                // no existing instance?
        return m_pClass->GetItem(name);        // makes the pointer in the class itself

    return m_pVarClass->GetItem(name);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarPointer::GetItemRef(int nIdent)
{
    if ( m_pVarClass == nullptr)                // no existing instance?
        return m_pClass->GetItemRef(nIdent);// makes the pointer to the class itself

    return m_pVarClass->GetItemRef(nIdent);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarPointer::GetItemList()
{
    if ( m_pVarClass == nullptr) return nullptr;
    return m_pVarClass->GetItemList();
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarPointer::GetValString() const
{
    std::string    s = "Pointer to ";
    if ( m_pVarClass == nullptr ) s = "Null pointer" ;
    else  s += m_pVarClass->GetValString();
    return s;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::ConstructorSet()
{
    if ( m_pVarClass != nullptr) m_pVarClass->ConstructorSet();
}

void CBotVarPointer::SetPointer(const CBotVarSPtr& pVarClass)
{
    m_binit = CBotVar::InitType::DEF;                            // init, even on a null pointer

    if ( m_pVarClass == pVarClass) return;    // special, not decrement and reincrement
                                            // because the decrement can destroy the object

    if ( !pVarClass )
    {
        m_pVarClass.reset();
    }
    else
    {
        auto instance = pVarClass; // the real pointer to the object
        if (instance)
        {
            if (!instance->m_type.Eq(CBotTypClass)) assert(0);
            m_pClass = (std::static_pointer_cast<CBotVarClass>(instance))->m_pClass;
            m_type = CBotTypResult(CBotTypPointer, m_pClass);    // what kind of a pointer
        }
        m_pVarClass = instance;
    }
}

CBotVarSPtr CBotVarPointer::GetPointer()
{
    return m_pVarClass;
}

bool CBotVarPointer::PointerIsUnique() const
{
    return m_pVarClass.use_count() == 1;
}

void CBotVarPointer::SetClass(CBotClass* pClass)
{
    m_type.m_class = m_pClass = pClass;
    if ( m_pVarClass ) m_pVarClass->SetClass(pClass);
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotVarPointer::GetClass()
{
    if ( m_pVarClass != nullptr ) return m_pVarClass->GetClass();

    return    m_pClass;
}

bool CBotVarPointer::Save1State(std::ostream &ostr, CBotContext& context)
{
    if ( m_type.GetClass() != nullptr )
    {
        if (!WriteString(ostr, m_type.GetClass()->GetName())) return false;  // name of the class
    }
    else
    {
        if (!WriteString(ostr, "")) return false;
    }

    if (!m_pVarClass) return WriteWord(ostr, 0); // save nullptr

    // save the instance
    if (!m_pVarClass->Save0State(ostr)) return false; // common header
    if (!m_pVarClass->Save1State(ostr, context)) return false; // saves the data or reference
    return true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarPointer::Copy(CBotVar* pSrc, bool bName)
{
    if ( pSrc->GetType() != CBotTypPointer &&
         pSrc->GetType() != CBotTypNullPointer)
        assert(0);

    CBotVarPointer*    p = static_cast<CBotVarPointer*>(pSrc);

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
//    m_pVarClass = p->m_pVarClass;
    m_pVarClass = p->GetPointer();

    m_pClass    = p->m_pClass;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_next        = nullptr;
    m_pMyThis    = nullptr;//p->m_pMyThis;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarPointer::Eq(CBotVar* left, CBotVar* right)
{
    auto l = left->GetPointer();
    auto r = right->GetPointer();

    if ( l == r ) return true;
    if ( l == nullptr && r->GetUserPointer().GetState() == PtrState::Dead) return true;
    if ( r == nullptr && l->GetUserPointer().GetState() == PtrState::Dead) return true;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarPointer::Ne(CBotVar* left, CBotVar* right)
{
    return !Eq(left, right);
}

} // namespace CBot
