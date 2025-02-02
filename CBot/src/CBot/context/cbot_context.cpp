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

#include "CBot/context/cbot_context.h"

#include "CBot/CBotClass.h"

#include <algorithm>
#include <cassert>

namespace CBot
{

CBotContextSPtr CBotContext::CreateGlobalContext()
{
    auto newContext = std::shared_ptr<CBotContext>(new CBotContext);
    InitErrorConstants(*newContext);
    InitFileIOLibrary(*newContext);
    InitStringFunctions(*newContext);
    InitMathLibrary(*newContext);

    return newContext;
}

CBotContextSPtr CBotContext::Create(const CBotContextSPtr& outer)
{
    auto newContext = std::shared_ptr<CBotContext>(new CBotContext(outer));
    InitErrorConstants(*newContext);
    InitStringFunctions(*newContext);
    InitMathLibrary(*newContext);

    return newContext;
}

CBotContext::CBotContext() :
    std::enable_shared_from_this<CBotContext>(),
    m_outerContext(nullptr),
    m_globalData(std::make_shared<CBotContext::GlobalData>())
{
}

CBotContext::CBotContext(const CBotContextSPtr& outer) :
    std::enable_shared_from_this<CBotContext>(),
    m_outerContext(outer),
    m_globalData(outer->m_globalData)
{
}

CBotContext::~CBotContext()
{
}

void CBotContext::ClearInstanceList()
{
    m_globalData->m_instances.clear();
    m_globalData->m_next = 1;
}

int CBotContext::DeclareInstance(CBotVar* var)
{
    int id = m_globalData->m_next++;
    m_globalData->m_instances[id] = var;
    return id;
}

void CBotContext::DeclareInstance(int id, CBotVar* var)
{
    assert(m_globalData->m_next == 1);
    m_globalData->m_instances[id] = var;
}

CBotClass* CBotContext::FindClass(const std::string& name) const
{
    auto it = m_classList.find(name);
    if (it != m_classList.end()) return it->second.get();
    return !m_outerContext ? nullptr : m_outerContext->FindClass(name);
}

CBotClass* CBotContext::CreateClass(const std::string& name, CBotClass* parent, bool intrinsic)
{
    if (FindClass(name) != nullptr) return nullptr;
    auto& newClass = m_classList[name];
    newClass.reset(CBotClass::Create(name, parent, shared_from_this(), intrinsic));
    return newClass.get();
}

void CBotContext::FreeLock(CBotProgram* prog) const
{
    for (auto p = this; p != nullptr; p = p->m_outerContext.get())
    {
        for (auto& item : p->m_classList) item.second->FreeLock(prog);
    }
}

bool CBotContext::WriteStaticState(std::ostream& ostr) const
{
    for ( auto& p : m_classList )
    {
        if (!WriteWord(ostr, 1)) return false;
        // save the name of the class
        if (!WriteString(ostr, p.second->GetName())) return false;

        if (!CBotClass::SaveStaticVars(ostr, p.second.get())) return false;
    }
    if (!WriteWord(ostr, 0)) return false; // no more classes
    if (!WriteWord(ostr, 0)) return false; // a flag to possibly save other data
    return true;
}

bool CBotContext::ReadStaticState(std::istream& istr, CBotContext& context)
{
    unsigned short w;
    while (true)
    {
        if (!ReadWord(istr, w)) return false;
        if ( w == 0 ) break; // no more classes
        if ( w != 1 )
        {
            assert(false);
            return false;
        }
        std::string className;
        if (!ReadString(istr, className)) return false;
        CBotClass* pClass = nullptr;
        auto it = context.m_classList.find(className);
        pClass = it != context.m_classList.end() ? it->second.get() : nullptr;
        if (!CBotClass::RestoreStaticVars(istr, pClass, context)) return false;
    }
    if (!ReadWord(istr, w)) return false;
    if (w != 0) return false; // currently unused flag should read 0
    return true;
}

CBotVar* CBotContext::FindInstance(int pos) const
{
    auto it = m_globalData->m_instances.find(pos);
    if (it != m_globalData->m_instances.end()) return it->second;
    return nullptr;
}

int CBotContext::FindInstance(CBotVar* var) const
{
    for (const auto& item : m_globalData->m_instances)
        if (item.second == var) return item.first;
    return -1;
}

bool CBotContext::IsDefinedConstant(const std::string& name) const
{
    if (!m_listConstant.IsDefinedConstant(name))
        return m_outerContext && m_outerContext->IsDefinedConstant(name);
    return true;
}

const CBotVarUPtr& CBotContext::GetDefinedConstant(const std::string& name)
{
    if (!m_outerContext || m_listConstant.IsDefinedConstant(name))
        return m_listConstant.GetDefinedConstant(name);
    return m_outerContext->GetDefinedConstant(name);
}

void CBotContext::SetFileAccessHandler(std::unique_ptr<CBotFileAccessHandler> fileHandler)
{
    if (m_globalData->m_fileHandler) return; // already set

    m_globalData->m_fileHandler = std::move(fileHandler);
}

CBotFileAccessHandler* CBotContext::GetFileAccessHandler() const
{
    return m_globalData->m_fileHandler.get();
}

const std::list<CBotFunction*>& CBotContext::GetPublicFunctions() const
{
    return m_functions;
}

void CBotContext::AddPublicFunction(CBotFunction* func)
{
    m_functions.push_back(func);
}

void CBotContext::RemovePublicFunction(CBotFunction* func)
{
    auto it = std::find_if(m_functions.begin(), m_functions.end(), [&func](auto x) { return x == func; });
    if (it != m_functions.end()) m_functions.erase(it);
}

} // namespace CBot
