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

#include "CBot/CBotClass.h"

#include "CBot/CBotInstr/CBotInstrUtils.h"
#include "CBot/CBotInstr/CBotNew.h"
#include "CBot/CBotInstr/CBotLeftExprVar.h"
#include "CBot/CBotInstr/CBotExprLitNull.h"
#include "CBot/CBotInstr/CBotTwoOpExpr.h"
#include "CBot/CBotInstr/CBotFunction.h"
#include "CBot/CBotInstr/CBotExpression.h"
#include "CBot/CBotInstr/CBotListArray.h"
#include "CBot/CBotInstr/CBotEmpty.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/CBotExternalCall.h"
#include "CBot/CBotStack.h"
#include "CBot/CBotCStack.h"
#include "CBot/CBotDefParam.h"
#include "CBot/CBotProgram.h"
#include "CBot/CBotUtils.h"

#include "CBot/context/cbot_context.h"

#include <algorithm>

namespace CBot
{

CBotClass::CBotClass(const std::string& name,
                     CBotClass* parent,
                     const CBotContextSPtr& context,
                     bool bIntrinsic) :
    CBotContextObserver(context)
{
    m_parent    = parent;
    m_name      = name;
    m_pVar      = nullptr;
    m_externalMethods = new CBotExternalCallList();
    m_rUpdate   = nullptr;
    m_IsDef     = true;
    m_bIntrinsic= bIntrinsic;
    m_nbVar     = m_parent == nullptr ? 0 : m_parent->m_nbVar;

}

////////////////////////////////////////////////////////////////////////////////
CBotClass::~CBotClass()
{

    delete  m_pVar;
    delete  m_externalMethods;
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Create(const std::string& name,
                             CBotClass* parent,
                             const CBotContextSPtr& context,
                             bool intrinsic)
{
    return new CBotClass(name, parent, context, intrinsic);
}

void CBotClass::Purge()
{
    delete      m_pVar;
    m_pVar      = nullptr;
    m_externalMethods->Clear();
    for (CBotFunction* f : m_pMethod) delete f;
    m_pMethod.clear();
    m_IsDef     = false;

    m_nbVar     = m_parent == nullptr ? 0 : m_parent->m_nbVar;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::Lock(CBotProgram* prog)
{
    if (m_lockProg.size() == 0)
    {
        m_lockCurrentCount = 1;
        m_lockProg.push_back(prog);
        return true;
    }
    if (prog == m_lockProg[0])
    {
        m_lockCurrentCount++;
        return true;
    }

    if (std::find(m_lockProg.begin(), m_lockProg.end(), prog) != m_lockProg.end())
    {
        return false; // already pending
    }

    m_lockProg.push_back(prog);

    return false;
}

////////////////////////////////////////////////////////////////////////////////
void CBotClass::Unlock()
{
    if (--m_lockCurrentCount > 0) return; // if called Lock() multiple times, wait for all to unlock

    m_lockProg.pop_front();
}

////////////////////////////////////////////////////////////////////////////////
void CBotClass::FreeLock(CBotProgram* prog)
{
    CBotClass* pClass = this;
    {
        if (pClass->m_lockProg.size() > 0 && prog == pClass->m_lockProg[0])
        {
            pClass->m_lockCurrentCount = 0;
        }

        // Note: erasing an end iterator is undefined behaviour
        auto it = std::remove(pClass->m_lockProg.begin(), pClass->m_lockProg.end(), prog);
        if (it != pClass->m_lockProg.end())
        {
            pClass->m_lockProg.erase(it);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::AddItem(std::string name,
                        CBotTypResult type,
                        CBotVar::ProtectionLevel mPrivate)
{
    CBotClass*  pClass = type.GetClass();

    CBotVar*    pVar = CBotVar::Create( name, type );
/// pVar->SetUniqNum(CBotVar::NextUniqNum());
    pVar->SetPrivate( mPrivate );

    if ( pClass != nullptr )
    {
//      pVar->SetClass(pClass);
        if ( type.Eq(CBotTypClass) )
        {
            // adds a new statement for the object initialization
            pVar->m_InitExpr = new CBotNew() ;
            CBotToken nom( pClass->GetName() );
            pVar->m_InitExpr->SetToken(&nom);
        }
    }
    return AddItem( pVar );
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::AddItem(CBotVar* pVar)
{
    pVar->SetUniqNum(++m_nbVar);

    if ( m_pVar == nullptr ) m_pVar = pVar;
    else m_pVar->AddNext(pVar);

    return true;
}

////////////////////////////////////////////////////////////////////////////////
const std::string&  CBotClass::GetName()
{
    return m_name;
}

////////////////////////////////////////////////////////////////////////////////
CBotClass*  CBotClass::GetParent()
{
    return m_parent;
}

////////////////////////////////////////////////////////////////////////////////
bool  CBotClass::IsChildOf(CBotClass* pClass)
{
    CBotClass* p = this;
    while ( p != nullptr )
    {
        if ( p == pClass ) return true;
        p = p->m_parent;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotClass::GetVar()
{
    return  m_pVar;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotClass::GetItem(const std::string& name)
{
    CBotVar*    p = m_pVar;

    while ( p != nullptr )
    {
        if ( p->GetName() == name ) return p;
        p = p->GetNext();
    }
    if (m_parent != nullptr ) return m_parent->GetItem(name);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
CBotVar* CBotClass::GetItemRef(int nIdent)
{
    CBotVar*    p = m_pVar;

    while ( p != nullptr )
    {
        if ( p->GetUniqNum() == nIdent ) return p;
        p = p->GetNext();
    }
    if (m_parent != nullptr ) return m_parent->GetItemRef(nIdent);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::CheckVar(const std::string &name)
{
    CBotVar*    p = m_pVar;

    while ( p != nullptr )
    {
        if ( p->GetName() == name ) return true;
        p = p->GetNext();
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::IsIntrinsic()
{
    return  m_bIntrinsic;
}

bool CBotClass::AddFunction(const std::string& name,
                            ClassRuntimeFunc rExec,
                            ClassCompileFunc cCompile)
{
    return m_externalMethods->AddFunction(name, rExec, cCompile);
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::SetUpdateFunc(void rUpdate(CBotVar* thisVar, void* user))
{
    m_rUpdate = rUpdate;
    return true;
}

////////////////////////////////////////////////////////////////////////////////
CBotTypResult CBotClass::CompileMethode(CBotToken* name,
                                        CBotVar* pThis,
                                        CBotVar** ppParams,
                                        CBotCStack* pStack,
                                        long &nIdent)
{
    nIdent = 0; // forget the previous one if necessary

    // find the methods declared by AddFunction

    CBotTypResult r = m_externalMethods->CompileCall(name, pThis, ppParams, pStack);
    if ( r.GetType() >= 0) return r;

    // find the methods declared by user

    r = CBotFunction::CompileMethodCall(name->GetString(), ppParams, nIdent, pStack, this);
    if ( r.Eq(CBotErrUndefCall) && m_parent != nullptr )
        return m_parent->CompileMethode(name, pThis, ppParams, pStack, nIdent);
    return r;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::ExecuteMethode(long& nIdent,
                               CBotVar* pThis,
                               CBotVar** ppParams,
                               CBotTypResult pResultType,
                               CBotStack*& pStack,
                               CBotToken& token)
{
    int ret = m_externalMethods->DoCall(token, pThis, ppParams, pStack, pResultType);
    if (ret >= 0) return ret;

    ret = CBotFunction::DoCall(nIdent, token.GetString(), pThis, ppParams, pStack, &token, this);
    if (ret >= 0) return ret;

    if (m_parent != nullptr)
    {
        ret = m_parent->ExecuteMethode(nIdent, pThis, ppParams, pResultType, pStack, token);
    }
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
void CBotClass::RestoreMethode(long& nIdent,
                               CBotToken* name,
                               CBotVar* pThis,
                               CBotVar** ppParams,
                               CBotStack*& pStack)
{
    if (m_externalMethods->RestoreCall(name, pThis, ppParams, pStack))
        return;

    CBotClass* pClass = this;
    while (pClass != nullptr)
    {
        bool ok = CBotFunction::RestoreCall(nIdent, name->GetString(), pThis, ppParams, pStack, pClass);
        if (ok) return;
        pClass = pClass->m_parent;
    }
    assert(false);
}

bool CBotClass::SaveStaticVars(std::ostream &ostr, CBotClass* pClass)
{
    for (auto pv = pClass->GetVar(); pv != nullptr; pv = pv->GetNext())
    {
        if ( !pv->IsStatic() ) continue;
        if (!WriteString(ostr, pv->GetName())) return false;
        if (!pv->Save0State(ostr)) return false;             // common header
        if (!pv->Save1State(ostr, *pClass->GetContext())) return false;                // saves as the child class
    }
    return WriteString(ostr, "");
}

bool CBotClass::RestoreStaticVars(std::istream &istr, CBotClass* pClass, CBotContext& context)
{
    for ( std::string varname; ReadString(istr, varname); )
    {
        if (varname.empty()) return true;
        CBotVarUPtr pv;
        if (!CBotVar::RestoreVar(istr, pv, context)) return false;
        if (pv == nullptr) { assert(false); return false; }
        auto pVar = pClass ? pClass->GetItem(varname) : nullptr;
        if (pVar != nullptr) pVar->Copy(pv.get());
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::CheckCall(CBotProgram* program, CBotDefParam* pParam, CBotToken*& pToken)
{
    std::string  name = pToken->GetString();

    if ( program->GetContext()->CheckCall(name) ) return true;

    for (CBotFunction* pp : m_pMethod)
    {
        if ( pToken->GetString() == pp->GetName() )
        {
            // are their parameters exactly the same?
            if ( pp->CheckParam( pParam ) )
                return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Compile1(CBotToken* &p, CBotCStack* pStack)
{
    if ( !IsOfType(p, ID_PUBLIC) )
    {
        pStack->SetError(CBotErrNoPublic, p);
        return nullptr;
    }

    if ( !IsOfType(p, ID_CLASS) ) return nullptr;

    std::string name = p->GetString();

    // a name of the class is there?
    if (IsOfType(p, TokenTypVar))
    {
        auto pOld = pStack->FindClass(name);
        if ((pOld != nullptr && pOld->m_IsDef) ||          /* public class exists in different program */
            pStack->GetProgram()->ClassExists(name))       /* class exists in this program */
        {
            pStack->SetError(CBotErrRedefClass, p->GetPrev());
            return nullptr;
        }

        CBotClass* pPapa = nullptr;
        if ( IsOfType( p, ID_EXTENDS ) )
        {
            std::string name = p->GetString();
            pPapa = pStack->FindClass(name);
            CBotToken* pp = p;

            if (!IsOfType(p, TokenTypVar) || pPapa == nullptr )
            {
                pStack->SetError(CBotErrNoClassName, pp);
                return nullptr;
            }
        }
        auto context = pStack->GetContext();
        auto classe = (pOld == nullptr) ? context->CreateClass(name, pPapa) : pOld;

        classe->Purge();                            // empty the old definitions
        classe->m_IsDef = false;                    // current definition

        classe->m_pOpenblk = p;

        if ( !IsOfType( p, ID_OPBLK) )
        {
            pStack->SetError(CBotErrOpenBlock, p);
            return nullptr;
        }

        int level = 1;
        while (level > 0 && p != nullptr)
        {
            int type = p->GetType();
            p = p->GetNext();
            if (type == ID_OPBLK) level++;
            if (type == ID_CLBLK) level--;
        }

        if (level > 0) pStack->SetError(CBotErrCloseBlock, classe->m_pOpenblk);

        if (pStack->IsOk()) return classe;
    }
    else
        pStack->SetError(CBotErrNoClassName, p);

    pStack->SetError(CBotErrNoTerminator, p);
    return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
void CBotClass::DefineClasses(std::list<CBotClass*> pClassList, CBotCStack* pStack)
{
    for (CBotClass* pClass : pClassList)
    {
        CBotClass* pParent = pClass->m_parent;
        pClass->m_nbVar = (pParent == nullptr) ? 0 : pParent->m_nbVar;
        CBotToken* p = pClass->m_pOpenblk->GetNext();

        while (pStack->IsOk() && !IsOfType(p, ID_CLBLK))
        {
            pClass->CompileDefItem(p, pStack, false);
        }

        if (!pStack->IsOk()) return;
    }
}

const std::list<CBotFunction*>& CBotClass::GetFunctions() const
{
    return m_pMethod;
}

////////////////////////////////////////////////////////////////////////////////
bool CBotClass::CompileDefItem(CBotToken* &p, CBotCStack* pStack, bool bSecond)
{
    bool    bStatic = false;
    CBotVar::ProtectionLevel mProtect = CBotVar::ProtectionLevel::Public;
    bool    bSynchro = false;

    while (IsOfType(p, ID_SEP)) ;

    CBotTypResult   type( -1 );

    if ( IsOfType(p, ID_SYNCHO) ) bSynchro = true;
    CBotToken*      pBase = p;

    if ( IsOfType(p, ID_STATIC) ) bStatic = true;
    if ( IsOfType(p, ID_PUBLIC) ) mProtect = CBotVar::ProtectionLevel::Public;
    if ( IsOfType(p, ID_PRIVATE) ) mProtect = CBotVar::ProtectionLevel::Private;
    if ( IsOfType(p, ID_PROTECTED) ) mProtect = CBotVar::ProtectionLevel::Protected;
    if ( IsOfType(p, ID_STATIC) ) bStatic = true;

//  CBotClass* pClass = nullptr;
    type = TypeParam(p, pStack);        // type of the result

    if ( type.Eq(-1) )
    {
        pStack->SetError(CBotErrNoType, p);
        return false;
    }

    while (pStack->IsOk())
    {
        CBotTypResult  type2 = CBotTypResult(type);                     // reset type after comma
        CBotToken*     varToken = p;

        std::string pp = p->GetString();
        if ( IsOfType(p, ID_NOT) )
        {
            pp = std::string("~") + p->GetString();
        }

        if (IsOfType(p, TokenTypVar))
        {
            if ( p->GetType() == ID_OPENPAR )
            {
                if ( !bSecond )
                {
                    p = pBase;
                    CBotFunction* f = CBotFunction::Compile1(p, pStack, this);

                    if ( f == nullptr ) return false;

                    m_pMethod.push_back(f);
                }
                else
                {
                    // return a method precompiled in pass 1
                    CBotCStack* pStk = pStack->TokenStack(nullptr, true);
                    CBotDefParam* params = CBotDefParam::Compile(p, pStk );
                    pStack->DeleteNext();
                    auto pfIter = std::find_if(m_pMethod.begin(), m_pMethod.end(), [&pp, &params](CBotFunction* x)
                    {
                        return x->GetName() == pp && x->CheckParam( params );
                    });
                    assert(pfIter != m_pMethod.end());
                    CBotFunction* pf = *pfIter;
                    delete params;

                    CBotCStack* pile = pStack->TokenStack(nullptr, true);

                    // compiles a method
                    p = pBase;
                    CBotFunction* f =
                    CBotFunction::Compile(p, pile, pf/*, false*/);

                    if ( f != nullptr )
                    {
                        f->m_pProg = pStack->GetProgram();
                        f->m_bSynchro = bSynchro;
                    }
                    pStack->DeleteNext();
                }

                return pStack->IsOk();
            }

            // definition of an element
            if (type.Eq(0))
            {
                pStack->SetError(CBotErrNoTerminator, p);
                return false;
            }

            if (pp[0] == '~' || pp == GetName()) // bad variable name
            {
                pStack->SetError(CBotErrNoVar, varToken);
                return false;
            }

            if (!bSecond && CheckVar(pp)) // variable already exists
            {
                pStack->SetError(CBotErrRedefVar, varToken);
                return false;
            }

            CBotInstr* limites = nullptr;
            while ( IsOfType( p, ID_OPBRK ) )   // an array
            {
                CBotInstr* i = nullptr;
                pStack->SetStartError( p->GetStart() );
                if ( p->GetType() != ID_CLBRK )
                {
                    i = CBotExpression::Compile( p, pStack );           // expression for the value
                    if (i == nullptr || pStack->GetType() != CBotTypInt) // must be a number
                    {
                        pStack->SetError(CBotErrBadIndex, p->GetStart());
                    }
                }
                else
                    i = new CBotEmpty();                            // special if not a formula

                type2 = CBotTypResult(CBotTypArrayPointer, type2);

                if (limites == nullptr) limites = i;
                else limites->AddNext3(i);

                if (pStack->IsOk() && IsOfType(p, ID_CLBRK)) continue;
                pStack->SetError(CBotErrCloseIndex, p->GetStart());
                delete limites;
                return false;
            }

            CBotInstr* i = nullptr;
            if ( IsOfType(p, ID_ASS ) )
            {
                pStack->SetStartError(p->GetStart());
                if ( IsOfType(p, ID_SEP) )
                {
                    pStack->SetError(CBotErrNoExpression, p->GetStart());
                    return false;
                }
                if ( type2.Eq(CBotTypArrayPointer) )
                {
                    if ( nullptr == (i = CBotListArray::Compile(p, pStack, type2.GetTypElem())) )
                    {
                        if (pStack->IsOk())
                        {
                            i = CBotTwoOpExpr::Compile(p, pStack);
                            if (i == nullptr || !pStack->GetTypResult().Compare(type2))
                            {
                                pStack->SetError(CBotErrBadType1, p->GetStart());
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    // it has an assignmet to calculate
                    i = CBotTwoOpExpr::Compile(p, pStack);

                    if ( !(type.Eq(CBotTypPointer) && pStack->GetTypResult().Eq(CBotTypNullPointer)) &&
                         !TypesCompatibles( type2, pStack->GetTypResult()) )
                    {
                        pStack->SetError(CBotErrBadType1, p->GetStart());
                        return false;
                    }
                }
                if ( !pStack->IsOk() ) return false;
            }
            else if ( type2.Eq(CBotTypArrayPointer) ) i = new CBotExprLitNull();


            if ( !bSecond )
            {
                CBotVar*    pv = CBotVar::Create(pp, type2);
                pv -> SetStatic( bStatic );
                pv -> SetPrivate( mProtect );

                AddItem( pv );

                pv->m_InitExpr = i;
                pv->m_LimExpr = limites;


                if ( pv->IsStatic() && pv->m_InitExpr != nullptr )
                {
                    CBotStack* pile = CBotStack::AllocateStack(pStack->GetContext()); // independent stack
                    if ( type2.Eq(CBotTypArrayPointer) )
                    {
                        while(pile->IsOk() && !pv->m_InitExpr->Execute(pile, pv));
                    }
                    else
                    {
                        while(pile->IsOk() && !pv->m_InitExpr->Execute(pile)); // evaluates the expression without timer
                        pv->SetVal( pile->GetVar() ) ;
                    }
                    pile->Delete();
                }
            }
            else
            {
                delete i;
                delete limites;
            }

            if ( IsOfType(p, ID_COMMA) ) continue;
            if ( IsOfType(p, ID_SEP) ) break;
        }
        pStack->SetError(CBotErrNoTerminator, p);
    }
    return pStack->IsOk();
}

////////////////////////////////////////////////////////////////////////////////
CBotClass* CBotClass::Compile(CBotToken* &p, CBotCStack* pStack)
{
    if ( !IsOfType(p, ID_PUBLIC) ) return nullptr;
    if ( !IsOfType(p, ID_CLASS) ) return nullptr;

    std::string name = p->GetString();

    // a name for the class is there?
    if (IsOfType(p, TokenTypVar))
    {
        // the class was created by Compile1
        auto pOld = pStack->FindClass(name);

        if ( IsOfType( p, ID_EXTENDS ) )
        {
            // TODO: Not sure how correct is that - I have no idea how the precompilation (Compile1 method) works ~krzys_h
            std::string name = p->GetString();
            auto pPapa = pStack->FindClass(name);
            CBotToken* pp = p;

            if (!IsOfType(p, TokenTypVar) || pPapa == nullptr)
            {
                pStack->SetError(CBotErrNoClassName, pp);
                return nullptr;
            }
            pOld->m_parent = pPapa;
        }
        else
        {
            if (pOld != nullptr)
            {
                pOld->m_parent = nullptr;
            }
        }
        IsOfType( p, ID_OPBLK); // necessarily

        while ( pStack->IsOk() && !IsOfType( p, ID_CLBLK ) )
        {
            pOld->CompileDefItem(p, pStack, true);
        }

        pOld->m_IsDef = true;           // complete definition
        if (pStack->IsOk()) return pOld;
    }
    pStack->SetError(CBotErrNoTerminator, p);
    return nullptr;
}

void CBotClass::Update(CBotVar* var, void* user)
{
    m_rUpdate(var, user);
}

} // namespace CBot
