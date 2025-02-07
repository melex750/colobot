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

#include "CBot/stdlib/stdlib.h"

#include "CBot/CBotVar/CBotVar.h"

#include "CBot/context/cbot_context.h"

#include "CBot/CBotClass.h"

#include <cmath>
#include <cstdlib>

namespace CBot
{
namespace
{
const float PI = 3.14159265358979323846f;

// Instruction "sin(degrees)".

bool rSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sinf(value*PI/180.0f));
    return true;
}

// Instruction "cos(degrees)".

bool rCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(cosf(value*PI/180.0f));
    return true;
}

// Instruction "tan(degrees)".

bool rTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(tanf(value*PI/180.0f));
    return true;
}

// Instruction "asin(degrees)".

bool raSin(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(asinf(value)*180.0f/PI);
    return true;
}

// Instruction "acos(degrees)".

bool raCos(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(acosf(value)*180.0f/PI);
    return true;
}

// Instruction "atan(degrees)".

bool raTan(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(atanf(value)*180.0f/PI);
    return true;
}

// Instruction "atan2(y,x)".

bool raTan2(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float y = var->GetValFloat();
    var = var->GetNext();
    float x = var->GetValFloat();

    result->SetValFloat(atan2(y, x) * 180.0f / PI);
    return true;
}

// Instruction "sqrt(value)".

bool rSqrt(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(sqrtf(value));
    return true;
}

// Instruction "pow(x, y)".

bool rPow(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   x, y;

    x = var->GetValFloat();
    var = var->GetNext();
    y = var->GetValFloat();
    result->SetValFloat(powf(x, y));
    return true;
}

// Instruction "rand()".

bool rRand(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    result->SetValFloat(static_cast<float>(rand()) / static_cast<float>(RAND_MAX));
    return true;
}

// Instruction "abs()".

bool rAbs(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    switch (result->GetType())
    {
        case CBotTypDouble:
            *result = fabs(var->GetValDouble());
            break;
        case CBotTypFloat:
            *result = fabs(var->GetValFloat());
            break;
        case CBotTypLong:
            *result = labs(var->GetValLong());
            break;
        default:
            *result = abs(var->GetValInt());
            break;
    }

    return true;
}

CBotTypResult cAbs(CBotVar* &var, void* user)
{
    if ( var == nullptr ) return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble ) return CBotTypResult(CBotErrBadNum);

    CBotTypResult returnType(var->GetType());
    var = var->GetNext();
    if ( var != nullptr ) return CBotTypResult(CBotErrOverParam);
    return returnType;
}

// Instruction "floor()"

bool rFloor(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(floor(value));
    return true;
}

// Instruction "ceil()"

bool rCeil(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(ceil(value));
    return true;
}

// Instruction "round()"

bool rRound(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(round(value));
    return true;
}

// Instruction "trunc()"

bool rTrunc(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    float   value;

    value = var->GetValFloat();
    result->SetValFloat(trunc(value));
    return true;
}

// Instruction "isnan()"

CBotTypResult cIsNAN(CBotVar*& var, void* user)
{
    if (var == nullptr)  return CBotTypResult(CBotErrLowParam);

    if (var->GetType() > CBotTypDouble)  return CBotTypResult(CBotErrBadNum);

    var = var->GetNext();
    if (var != nullptr)  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypBoolean);
}

bool rIsNAN(CBotVar* var, CBotVar* result, int& exception, void* user)
{
    bool isnan = false;

    if (var->GetType() == CBotTypFloat)
    {
        if (std::isnan(var->GetValFloat())) isnan = true;
    }
    else if (var->GetType() == CBotTypDouble)
    {
        if (std::isnan(var->GetValDouble())) isnan = true;
    }

    result->SetValInt(isnan);
    return true;
}

CBotTypResult cSizeOf( CBotVar* &pVar, void* pUser )
{
    if ( pVar == nullptr ) return CBotTypResult( CBotErrLowParam );
    if ( pVar->GetType() != CBotTypArrayPointer )
                        return CBotTypResult( CBotErrBadParam );
    return CBotTypResult( CBotTypInt );
}

bool rSizeOf( CBotVar* pVar, CBotVar* pResult, int& ex, void* pUser )
{
    if ( pVar == nullptr ) { ex = CBotErrLowParam; return true; }

    int i = 0;
    pVar = pVar->GetItemList();

    while ( pVar != nullptr )
    {
        i++;
        pVar = pVar->GetNext();
    }

    pResult->SetValInt(i);
    return true;
}

// Compilation of class "point".

CBotTypResult cPointConstructor(CBotVar* pThis, CBotVar* &var)
{
    if ( var == nullptr )  return CBotTypResult(0);  // ok if no parameter

    if ( var->GetType() == CBotTypClass )  // takes a single point parameter
    {
        if ( !var->IsElemOfClass("point") )  return CBotTypResult(CBotErrBadParam);
        var = var->GetNext();
        if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);
        return CBotTypResult(CBotTypVoid);  // this function returns void
    }

    // First parameter (x):
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Second parameter (y):
    if ( var == nullptr )  return CBotTypResult(CBotErrLowParam);
    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();

    // Third parameter (z):
    if ( var == nullptr )  // only 2 parameters?
    {
        return CBotTypResult(CBotTypVoid);  // this function returns void
    }

    if ( var->GetType() > CBotTypDouble )  return CBotTypResult(CBotErrBadNum);
    var = var->GetNext();
    if ( var != nullptr )  return CBotTypResult(CBotErrOverParam);

    return CBotTypResult(CBotTypVoid);  // this function returns void
}

//Execution of the class "point".

bool rPointConstructor(CBotVar* pThis, CBotVar* var, CBotVar* pResult, int& Exception, void* user)
{
    if ( var == nullptr )  return true;  // constructor with no parameters is ok

    if ( var->GetType() == CBotTypClass )
    {
        pThis->Copy(var, false);
        return true;  // no interruption
    }

    pThis->GetItem("x")->SetValFloat( var->GetValFloat() );

    var = var->GetNext();
    pThis->GetItem("y")->SetValFloat( var->GetValFloat() );

    var = var->GetNext();
    if ( var == nullptr ) return true;  // ok with only two parameters

    pThis->GetItem("z")->SetValFloat( var->GetValFloat() );

    return true;  // no interruption
}

} // namespace

void InitErrorConstants(CBotContext& context)
{
    context.AddConstant<int>("CBotErrZeroDiv",    CBotErrZeroDiv);    // division by zero
    context.AddConstant<int>("CBotErrNotInit",    CBotErrNotInit);    // uninitialized variable
    context.AddConstant<int>("CBotErrBadThrow",   CBotErrBadThrow);   // throw a negative value
    context.AddConstant<int>("CBotErrNoRetVal",   CBotErrNoRetVal);   // function did not return results
    context.AddConstant<int>("CBotErrNoRun",      CBotErrNoRun);      // active Run () without a function // TODO: Is this actually a runtime error?
    context.AddConstant<int>("CBotErrUndefFunc",  CBotErrUndefFunc);  // Calling a function that no longer exists
    context.AddConstant<int>("CBotErrNotClass",   CBotErrNotClass);   // Class no longer exists
    context.AddConstant<int>("CBotErrNull",       CBotErrNull);       // Attempted to use a null pointer
    context.AddConstant<int>("CBotErrNan",        CBotErrNan);        // Can't do operations on nan
    context.AddConstant<int>("CBotErrOutArray",   CBotErrOutArray);   // Attempted access out of bounds of an array
    context.AddConstant<int>("CBotErrStackOver",  CBotErrStackOver);  // Stack overflow
    context.AddConstant<int>("CBotErrDeletedPtr", CBotErrDeletedPtr); // Attempted to use deleted object

    // TODO: Check the other CBotError runtime codes to see if they should be included here too...
}

void InitMathLibrary(CBotContext& context)
{
    context.AddConstant<float>("PI", PI);

    context.AddFunction("sin",   rSin,   cOneFloat);
    context.AddFunction("cos",   rCos,   cOneFloat);
    context.AddFunction("tan",   rTan,   cOneFloat);
    context.AddFunction("asin",  raSin,  cOneFloat);
    context.AddFunction("acos",  raCos,  cOneFloat);
    context.AddFunction("atan",  raTan,  cOneFloat);
    context.AddFunction("atan2", raTan2, cTwoFloat);
    context.AddFunction("sqrt",  rSqrt,  cOneFloat);
    context.AddFunction("pow",   rPow,   cTwoFloat);
    context.AddFunction("rand",  rRand,  cNull);
    context.AddFunction("abs",   rAbs,   cAbs);
    context.AddFunction("floor", rFloor, cOneFloat);
    context.AddFunction("ceil",  rCeil,  cOneFloat);
    context.AddFunction("round", rRound, cOneFloat);
    context.AddFunction("trunc", rTrunc, cOneFloat);
    context.AddFunction("isnan", rIsNAN, cIsNAN);

    context.AddFunction("sizeof", rSizeOf, cSizeOf);

    auto pnt = context.FindClass("point");
    if (pnt == nullptr)
    {
        pnt = context.CreateClass("point", nullptr, true);  // intrinsic class
        pnt->AddItem("x", CBotTypFloat);
        pnt->AddItem("y", CBotTypFloat);
        pnt->AddItem("z", CBotTypFloat);
        pnt->AddFunction("point", rPointConstructor, cPointConstructor);
    }
}

} // namespace CBot
