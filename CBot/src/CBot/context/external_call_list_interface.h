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

#include "CBot/CBotExternalCall.h"
#include "CBot/CBotTypResult.h"

#include <utility>

namespace CBot
{

class CBotExternalCallListInterface
{
protected:
    CBotExternalCallListInterface()
    {
    }

    ~CBotExternalCallListInterface()
    {
    }

public:
    /**
     * \brief Add a function that can be called from CBOT
     *
     * To define an external function, proceed as follows:
     *
     * 1. Define a function for compilation
     *
     * This function should take a list of function arguments (types only, no values!) and a user-defined void* pointer (can be passed in Compile()) as parameters, and return CBotTypResult.
     *
     * Usually, name of this function is prefixed with "c".
     *
     * The function should iterate through the provided parameter list and verify that they match.<br>
     * If they don't, then return CBotTypResult with an appropariate error code (see ::CBotError).<br>
     * If they do, return CBotTypResult with the function's return type
     *
     * \code
     * CBotTypResult cMessage(CBotVar* &var, void* user)
     * {
     *     if (var == nullptr) return CBotTypResult(CBotErrLowParam); // Not enough parameters
     *     if (var->GetType() != CBotTypString) return CBotTypResult(CBotErrBadString); // String expected
     *
     *     var = var->GetNext(); // Get the next parameter
     *     if (var != nullptr) return CBotTypResult(CBotErrOverParam); // Too many parameters
     *
     *     return CBotTypResult(CBotTypFloat); // This function returns float (it may depend on parameters given!)
     * }
     * \endcode
     *
     * 2. Define a function for execution
     *
     * This function should take:
     * * a list of parameters
     * * pointer to a result variable (a variable of type given at compilation time will be provided)
     * * pointer to an exception variable
     * * user-defined pointer (can be passed in Run())
     *
     * This function returns true if execution of this function is finished, or false to suspend the program and call this function again on next Run() cycle.
     *
     * Usually, execution functions are prefixed with "r".
     *
     * \code
     * bool rMessage(CBotVar* var, CBotVar* result, int& exception, void* user)
     * {
     *     std::string message = var->GetValString();
     *     std::cout << message << std::endl;
     *
     *     result->SetValInt(0); set the return value
     *     return true; // Execution finished
     * }
     * \endcode
     *
     * 3. Call AddFunction() to register the function in the list
     *
     * \code
     * AddFunction("message", rMessage, cMessage);
     * \endcode
     *
     * For more sophisticated examples, see the Colobot source code, mainly the src/script/scriptfunc.cpp file
     *
     * \param name Name of the function
     * \param rExec Execution function
     * \param rCompile Compilation function
     * \return false if there was an error, otherwise true
     */
    template<typename R = DefaultRuntimeFunc, typename C = DefaultCompileFunc>
    bool AddFunction(const std::string& name, R rExec, C cCompile)
    {
        return m_externalCallList.AddFunction(name, rExec, cCompile);
    }

    template<typename... Args>
    CBotTypResult CompileCall(Args&&... args)
    {
        return m_externalCallList.CompileCall(std::forward<Args>(args)...);
    }

    bool CheckCall(const std::string& name)
    {
        return m_externalCallList.CheckCall(name);
    }

    template<typename... Args>
    int DoCall(Args&&... args)
    {
        return m_externalCallList.DoCall(std::forward<Args>(args)...);
    }

    template<typename... Args>
    bool RestoreCall(Args&&... args)
    {
        return m_externalCallList.RestoreCall(std::forward<Args>(args)...);
    }

    void SetUserPtr(void* pUser)
    {
        return m_externalCallList.SetUserPtr(pUser);
    }

private:
    CBotExternalCallList m_externalCallList;
};

} // namespace CBot
