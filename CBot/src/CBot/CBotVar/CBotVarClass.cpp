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

#include "CBot/CBotVar/CBotVarClass.h"

#include "CBot/CBotClass.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotDefines.h"

#include "CBot/CBotInstr/CBotInstr.h"

#include "CBot/context/cbot_context.h"
#include "CBot/context/cbot_user_pointer.h"

#include <cassert>

namespace CBot
{

CBotVarClass::CBotVarClass(const CBotToken& name, const CBotTypResult& type) : CBotVar(name)
{
    if ( !type.Eq(CBotTypClass)        &&
         !type.Eq(CBotTypIntrinsic)    &&                // by convenience there accepts these types
         !type.Eq(CBotTypPointer)      &&
         !type.Eq(CBotTypArrayPointer) &&
         !type.Eq(CBotTypArrayBody)) assert(0);

    m_next        = nullptr;
    m_pMyThis    = nullptr;
    m_InitExpr = nullptr;
    m_LimExpr = nullptr;
    m_pVar        = nullptr;
    m_type        = type;
    if ( type.Eq(CBotTypArrayPointer) )    m_type.SetType( CBotTypArrayBody );
    else if ( !type.Eq(CBotTypArrayBody) ) m_type.SetType( CBotTypClass );
                                                 // official type for this object

    m_pClass    = nullptr;
    m_binit        = InitType::UNDEF;
    m_bStatic    = false;
    m_mPrivate    = ProtectionLevel::Public;
    m_bConstructor = false;

    CBotClass* pClass = type.GetClass();

    SetClass( pClass );

}

////////////////////////////////////////////////////////////////////////////////
CBotVarClass::~CBotVarClass( )
{
    CallDestructor(); // TODO
    delete    m_pVar;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::ConstructorSet()
{
    m_bConstructor = true;
}

////////////////////////////////////////////////////////////////////////////////
void CBotVarClass::Copy(CBotVar* pSrc, bool bName)
{
    auto srcType = pSrc->GetType();
    if ( srcType != CBotTypClass && srcType != CBotTypIntrinsic)
        assert(0);

    CBotVarClass*    p = static_cast<CBotVarClass*>(pSrc);

    if (bName)    *m_token    = *p->m_token;

    m_type        = p->m_type;
    m_binit        = p->m_binit;
//-    m_bStatic    = p->m_bStatic;
    m_pClass    = p->m_pClass;

    m_pMyThis    = nullptr;//p->m_pMyThis;

    // keeps indentificator the same (by default)
    if (m_ident == 0 ) m_ident     = p->m_ident;

    delete        m_pVar;
    m_pVar        = nullptr;

    CBotVar*    pv = p->m_pVar;
    while( pv != nullptr )
    {
        CBotVar*    pn = CBotVar::Create(pv);
        pn->Copy( pv );
        if ( m_pVar == nullptr ) m_pVar = pn;
        else m_pVar->AddNext(pn);

        pv = pv->GetNext();
    }
}

void CBotVarClass::SetClass(CBotClass* pClass)//, int &nIdent)
{
    if ( m_pClass == pClass ) return;

    m_pClass = pClass;

    // initializes the variables associated with this class
    delete m_pVar;
    m_pVar = nullptr;

    if (pClass == nullptr) return;

    m_type.m_class = pClass;

    auto context = pClass->GetContext();
    if (!context) return;

    CBotVar* pv = nullptr;
    while (pClass != nullptr)
    {
        if ( pv == nullptr ) pv = pClass->GetVar();
        if ( pv == nullptr ) { pClass = pClass->GetParent(); continue; }
        // seeks the maximum dimensions of the table
        CBotInstr*    p  = pv->m_LimExpr;                            // the different formulas
        if ( p != nullptr )
        {
            CBotStack* pile = CBotStack::AllocateStack(context.get()); // an independent stack
            int     n = 0;
            int     max[100];

            while (p != nullptr)
            {
                while( pile->IsOk() && !p->Execute(pile) ) ;        // calculate size without interruptions
                CBotVar*    v = pile->GetVar();                        // result
                max[n] = v->GetValInt();                            // value
                n++;
                p = p->GetNext3();
            }
            while (n<100) max[n++] = 0;

            pv->m_type.SetArray(max);                    // stores the limitations
            pile->Delete();
        }

        CBotVar*    pn = CBotVar::Create( pv );        // a copy
        pn->SetStatic(pv->IsStatic());
        pn->SetPrivate(pv->GetPrivate());

        if ( pv->m_InitExpr != nullptr )                // expression for initialization?
        {
#if    STACKMEM
            CBotStack* pile = CBotStack::AllocateStack(context.get()); // an independent stack

            while(pile->IsOk() && !pv->m_InitExpr->Execute(pile, pn));    // evaluates the expression without timer

            pile->Delete();
#else
            CBotStack* pile = new CBotStack(nullptr);     // an independent stack
            while(!pv->m_InitExpr->Execute(pile));    // evaluates the expression without timer
            pn->SetVal( pile->GetVar() ) ;
            delete pile;
#endif
        }

//        pn->SetUniqNum(CBotVar::NextUniqNum());        // enumerate elements
        pn->SetUniqNum(pv->GetUniqNum());    //++nIdent
        pn->m_pMyThis = this;

        if ( m_pVar == nullptr) m_pVar = pn;
        else m_pVar->AddNext( pn );
        pv = pv->GetNext();
        if ( pv == nullptr ) pClass = pClass->GetParent();
    }
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotVarClass::GetClass()
{
    return    m_pClass;
}

void CBotVarClass::Update()
{
    if (!m_userPtr) return;

    void* user = m_userPtr->GetPointerAs<void>();

    if (user == nullptr) return;

    m_pClass->Update(this, user);
}

void CBotVarClass::SetUserPointer(std::unique_ptr<CBotUserPointer> user)
{
    if (m_userPtr)
        assert(false);
    else
        m_userPtr = std::move(user);
}

const std::unique_ptr<CBotUserPointer>& CBotVarClass::GetUserPointer()
{
    return m_userPtr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarClass::GetItem(const std::string& name)
{
    CBotVar*    p = m_pVar;

    while ( p != nullptr )
    {
        if ( p->GetName() == name ) return p;
        p = p->GetNext();
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarClass::GetItemRef(int nIdent)
{
    CBotVar*    p = m_pVar;

    while ( p != nullptr )
    {
        if ( p->GetUniqNum() == nIdent ) return p;
        p = p->GetNext();
    }

    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarClass::GetItem(int n, bool bExtend)
{
    CBotVar*    p = m_pVar;

    if ( n < 0 ) return nullptr;
    if ( n > MAXARRAYSIZE ) return nullptr;

    if ( m_type.GetLimite() >= 0 && n >= m_type.GetLimite() ) return nullptr;

    if ( p == nullptr && bExtend )
    {
        p = CBotVar::Create("", m_type.GetTypElem());
        m_pVar = p;
    }

    if ( n == 0 ) return p;

    while ( n-- > 0 )
    {
        if ( p->m_next == nullptr )
        {
            if ( bExtend ) p->m_next = CBotVar::Create("", m_type.GetTypElem());
            if ( p->m_next == nullptr ) return nullptr;
        }
        p = p->m_next;
    }

    return p;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotVarClass::GetItemList()
{
    return m_pVar;
}

////////////////////////////////////////////////////////////////////////////////
std::string CBotVarClass::GetValString() const
{
    std::string    res;

    if ( m_pClass != nullptr )                        // not used for an array
    {
        res = m_pClass->GetName() + std::string("( ");

        CBotClass* pClass = m_pClass;
        long prevID = 0;
        {
            CBotVar* pv = m_pVar;
            if (pv != nullptr) while (true)
            {
                if (pv->GetUniqNum() < prevID)
                {
                    pClass = pClass->GetParent();
                    if (pClass == nullptr) break;
                    res += " ) extends ";
                    res += pClass->GetName();
                    res += "( ";
                    if (pClass->GetVar() == nullptr) continue;
                }

                prevID = pv->GetUniqNum();

                res += pv->GetName() + std::string("=");

                if ( pv->IsStatic() )
                {
                    res += pClass->GetItemRef(prevID)->GetValString();
                }
                else
                {
                    res += pv->GetValString();
                }
                pv = pv->GetNext();
                if ( pv == nullptr ) break;
                if ( pv->GetUniqNum() > prevID ) res += ", ";
            }

            if (pClass != nullptr) while (true)
            {
                pClass = pClass->GetParent();
                if (pClass == nullptr) break;
                res += " ) extends ";
                res += pClass->GetName();
                res += "( ";
            }
        }

        res += " )";
    }
    else
    {
        res = "{ ";

        CBotVar*    pv = m_pVar;
        while ( pv != nullptr )
        {
            res += pv->GetValString();
            if ( pv->GetNext() != nullptr ) res += ", ";
            pv = pv->GetNext();
        }

        res += " }";
    }

    return    res;
}

void CBotVarClass::CallDestructor()
{
    {
        // if there is one, call the destructor
        // but only if a constructor had been called.
        if ( m_bConstructor )
        {
            m_bConstructor = false;
            auto context = m_pClass->GetContext();

            CBotStack*  pile = CBotStack::AllocateStack(context.get());
            CBotVar*    ppVars[1];
            ppVars[0] = nullptr;

            CBotVar*    pThis  = CBotVar::Create("this", CBotTypNullPointer);
            // create a non-deleting shared_ptr
            std::shared_ptr<CBotVar> tempPtr(this, [](CBotVar *p){});
            m_weakPtr = tempPtr;
            pThis->SetPointer(tempPtr);

            std::string    nom = std::string("~") + m_pClass->GetName();
            long        ident = 0;

            CBotToken token(nom); // TODO

            while ( pile->IsOk() && !m_pClass->ExecuteMethode(ident, pThis, ppVars, CBotTypResult(CBotTypVoid), pile, &token)) ;    // waits for the end

            pile->Delete();
            delete pThis;
        }
    }
}

CBotVarSPtr CBotVarClass::GetPointer()
{
    auto sharedPtr = m_weakPtr.lock();
    if (!sharedPtr)
    {
        if (m_pClass!=nullptr && m_pClass->IsIntrinsic())
            assert(0);
        else
            sharedPtr.reset(this, CBotVar::Destroy);
        m_weakPtr = sharedPtr;
    }
    return sharedPtr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Eq(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GetItemList();
    CBotVar*    r = right->GetItemList();

    while ( l != nullptr && r != nullptr )
    {
        if ( l->Ne(l, r) ) return false;
        l = l->GetNext();
        r = r->GetNext();
    }

    // should always arrived simultaneously at the end (same classes)
    return l == r;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotVarClass::Ne(CBotVar* left, CBotVar* right)
{
    CBotVar*    l = left->GetItemList();
    CBotVar*    r = right->GetItemList();

    while ( l != nullptr && r != nullptr )
    {
        if ( l->Ne(l, r) ) return true;
        l = l->GetNext();
        r = r->GetNext();
    }

    // should always arrived simultaneously at the end (same classes)
    return l != r;
}

bool CBotVarClass::Save1State(std::ostream &ostr, CBotContext& context)
{
    auto pClass = m_type.GetClass();
    if (pClass != nullptr && pClass->IsIntrinsic())
    {
        if (!WriteInt(ostr, /*isExisting*/ 0)) return false;
        if (!WriteInt(ostr, /*id*/ 0)) return false;
    }
    else
    {
        int id = context.FindInstance(this);
        if (id == -1)
        {
            if (!WriteInt(ostr, /*isExisting*/ 0)) return false;
            id = context.DeclareInstance(this);
            if (!WriteInt(ostr, id)) return false;
        }
        else // save only the id of the already saved instance
        {
            if (!WriteInt(ostr, /*isExisting*/ 1)) return false;
            return WriteInt(ostr, id);
        }
    }

    if (!WriteType(ostr, m_type)) return false;
    if (!WriteVarListAsArray(ostr, m_pVar, context)) return false;

    if (!m_type.Eq(CBotTypArrayBody))
    {
        for (auto var = m_pVar; var != nullptr; var = var->GetNext())
        {
            if (!WriteLong(ostr, var->m_ident)) return false;
        }
    }
    return true;
}

} // namespace CBot
