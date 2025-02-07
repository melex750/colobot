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

#include "CBot/CBotVar/CBotVar.h"

namespace CBot
{

/**
 * \brief CBotVar subclass for managing classes (::CBotTypClass, ::CBotTypIntrinsic)
 *
 * \nosubgrouping
 */
class CBotVarClass : public CBotVar
{
public:
    /**
     * \brief Constructor. Do not call directly, use CBotVar::Create()
     */
    CBotVarClass(const CBotToken& name, const CBotTypResult& type);
    /**
     * \brief Destructor. Do not call directly, use CBotVar::Destroy()
     */
    ~CBotVarClass();

    void Copy(CBotVar* pSrc, bool bName = true) override;

    void SetClass(CBotClass* pClass) override;
    CBotClass* GetClass() override;

    CBotVar* GetItem(const std::string& name) override;
    CBotVar* GetItemRef(int nIdent) override;
    CBotVar* GetItem(int n, bool bExtend) override;
    CBotVar* GetItemList() override;
    std::string GetValString() const override;

    bool Save1State(std::ostream &ostr, CBotContext& context) override;

    void Update() override;
    void SetUserPointer(std::unique_ptr<CBotUserPointer> user) override;
    const std::unique_ptr<CBotUserPointer>& GetUserPointer() override;

    CBotVarSPtr GetPointer() override;

    bool Eq(CBotVar* left, CBotVar* right) override;
    bool Ne(CBotVar* left, CBotVar* right) override;

    void ConstructorSet() override;

private:
    void CallDestructor(); //TODO

private:
    //! Class definition
    CBotClass* m_pClass;
    //! Class members
    CBotVar* m_pVar;
    //! Weak pointer to self
    std::weak_ptr<CBotVar> m_weakPtr;
    //! Set after constructor is called, allows destructor to be called
    bool m_bConstructor;

    std::unique_ptr<CBotUserPointer> m_userPtr;

    friend class CBotVar;
    friend class CBotVarPointer;
};

} // namespace CBot
