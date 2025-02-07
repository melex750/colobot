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

#include "CBot/context/external_call_list_interface.h"
#include "CBot/context/list_constant_interface.h"

#include "CBot/stdlib/stdlib_public.h"

#include <list>
#include <memory>
#include <unordered_map>

namespace CBot
{
class CBotClass;
class CBotContext;
class CBotFunction;
class CBotProgram;
class CBotVar;

using CBotContextSPtr = std::shared_ptr<CBot::CBotContext>;
using CBotFileAccessHandlerUPtr = std::unique_ptr<CBot::CBotFileAccessHandler>;

class CBotContext :
    public std::enable_shared_from_this<CBotContext>,
    public CBotExternalCallListInterface,
    public CBotListConstantInterface
{
private:
    CBotContext();

    CBotContext(const CBotContextSPtr& outer);

    CBotContext(const CBotContext&) = delete;
    CBotContext& operator=(const CBotContext&) = delete;

public:
    [[nodiscard]] static CBotContextSPtr CreateGlobalContext();

    [[nodiscard]] static CBotContextSPtr Create(const CBotContextSPtr& outer);

    /**
     * \brief Destructor
     */
    ~CBotContext();

    /**
     * \brief Generate unique identifier for functions, etc.
     */
    long GetNewUniqueID()
    {
        return m_globalData->m_nextUniqueID++;
    }

    CBotClass* FindClass(const std::string& name) const;

    CBotClass* CreateClass(const std::string& name, CBotClass* parent, bool intrinsic = false);

    void FreeLock(CBotProgram* prog) const;

    bool WriteStaticState(std::ostream& ostr) const;

    static bool ReadStaticState(std::istream& istr, CBotContext& context);

    int FindInstance(CBotVar* var) const;
    CBotVar* FindInstance(int id) const;
    void ClearInstanceList();
    int DeclareInstance(CBotVar* var);
    void DeclareInstance(int id, CBotVar* var);

    bool IsDefinedConstant(const std::string& name) const override;

    const CBotVarUPtr& GetDefinedConstant(const std::string& name) override;

    CBotFileAccessHandler* GetFileAccessHandler() const;

    void SetFileAccessHandler(CBotFileAccessHandlerUPtr fileHandler);

    const std::list<CBotFunction*>& GetPublicFunctions() const;

    void AddPublicFunction(CBotFunction* func);

    void RemovePublicFunction(CBotFunction* func);

private:
    CBotContextSPtr m_outerContext;

    std::list<CBotFunction*> m_functions;

    struct GlobalData
    {
        long m_nextUniqueID = 10000;
        CBotFileAccessHandlerUPtr m_fileHandler;
        std::unordered_map<int, CBotVar*> m_instances;
        int m_next = 1;
    };
    std::shared_ptr<CBotContext::GlobalData> m_globalData;

    std::unordered_map<std::string, std::unique_ptr<CBotClass>> m_classList;
};

} // namespace CBot
