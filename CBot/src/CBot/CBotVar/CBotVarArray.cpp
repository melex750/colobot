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

#include "CBot/CBotVar/CBotVarArray.h"
#include "CBot/CBotVar/CBotVarClass.h"
#include "CBot/CBotToken.h"

#include "CBot/CBotEnums.h"

#include <cassert>


namespace CBot
{

////////////////////////////////////////////////////////////////////////////////
CBotVarArray::CBotVarArray(const CBotToken& name, CBotTypResult& type) : CBotVar(name)
{
    if ( !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) assert(0);

    m_next        = nullptr;
    m_pMyThis    = nullptr;

    m_type        = type;
    m_type.SetType(CBotTypArrayPointer);
    m_binit        = CBotVar::InitType::UNDEF;

    m_pInstance    = nullptr;                        // the list of the array elements
}

////////////////////////////////////////////////////////////////////////////////
CBotVarArray::~CBotVarArray()
{
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarArray::Copy(CBotVar* pSrc, bool bName)
{
    if ( pSrc->GetType() != CBotTypArrayPointer )
        assert(0);

    CBotVarArray*    p = static_cast<CBotVarArray*>(pSrc);

    if ( bName) *m_token    = *p->m_token;
    m_type        = p->m_type;
    m_pInstance = p->GetPointer();

    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_pMyThis    = nullptr;//p->m_pMyThis;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;
}

void CBotVarArray::SetPointer(const CBotVarSPtr& pVarClass)
{
    m_binit = CBotVar::InitType::DEF;         // init, even on a null pointer

    if ( m_pInstance == pVarClass) return;    // Special, not decrement and reincrement
                                            // because the decrement can destroy the object

    if ( pVarClass != nullptr )
    {
        if ( !pVarClass->m_type.Eq(CBotTypClass) &&
             !pVarClass->m_type.Eq(CBotTypArrayBody))
            assert(0);
    }
    m_pInstance = pVarClass;
}

CBotVarSPtr CBotVarArray::GetPointer()
{
    return m_pInstance;
}

bool CBotVarArray::PointerIsUnique() const
{
    return m_pInstance.use_count() == 1;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarArray::GetItem(int n, bool bExtend)
{
    if ( m_pInstance == nullptr )
    {
        if ( !bExtend ) return nullptr;
        // creates an instance of the table

        CBotVarClass* instance = new CBotVarClass(CBotToken(), m_type);
        SetPointer( instance->GetPointer() );
    }
    return m_pInstance->GetItem(n, bExtend);
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarArray::GetItemList()
{
    if ( m_pInstance == nullptr) return nullptr;
    return m_pInstance->GetItemList();
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarArray::GetValString() const
{
    if ( m_pInstance == nullptr ) return ( std::string( "Null pointer" ) ) ;
    return m_pInstance->GetValString();
}

bool CBotVarArray::Save1State(std::ostream &ostr, CBotContext& context)
{
    if (!WriteType(ostr, m_type)) return false;

    if (!m_pInstance) return WriteWord(ostr, 0); // save nullptr

    // save the instance
    if (!m_pInstance->Save0State(ostr)) return false; // common header
    if (!m_pInstance->Save1State(ostr, context)) return false; // saves the data or reference
    return true;
}

} // namespace CBot
