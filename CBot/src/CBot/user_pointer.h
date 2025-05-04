/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2025, Daniel Roux, EPSITEC SA & TerranovaTeam
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

namespace CBot
{

enum class PtrState
{
    UnInit,
    Alive,
    Dead
};

class CBotVarUserPointer
{
public:
    PtrState GetState() const;
    template<typename T>
    T* GetPointerAs() { return static_cast<T*>(m_p); }
    void Set(void* p);
    void Kill();

    CBotVarUserPointer() = default;
    CBotVarUserPointer(const CBotVarUserPointer&) = default;
    CBotVarUserPointer& operator=(const CBotVarUserPointer&) = default;
private:
    PtrState m_state = PtrState::UnInit;
    void* m_p = nullptr;
};

} // namespace CBot
