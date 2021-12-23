/*
 * This file is part of the Colobot: Gold Edition source code
 * Copyright (C) 2001-2021, Daniel Roux, EPSITEC SA & TerranovaTeam
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

/**
 * \file graphics/core/vertex.h
 * \brief Vertex structs
 */

#pragma once


#include "graphics/core/color.h"
#include "graphics/core/type.h"

#include "math/vector.h"

#include <sstream>
#include <cstdint>

#include <glm/glm.hpp>

// Graphics module namespace
namespace Gfx
{

enum VertexType
{
    VERTEX_TYPE_NORMAL,
    VERTEX_TYPE_TEX2,
    VERTEX_TYPE_COL,
};

/**
 * \struct Vertex
 * \brief Vertex of a primitive
 *
 * This structure was created as analog to DirectX's D3DVERTEX.
 *
 * It contains:
 *  - vertex coordinates (x,y,z) as Math::Vector,
 *  - normal coordinates (nx,ny,nz) as Math::Vector
 *  - texture coordinates (u,v) as glm::vec2.
 */
struct Vertex
{
    static constexpr VertexType VERTEX_TYPE = VERTEX_TYPE_NORMAL;

    Math::Vector coord = Math::Vector();
    Math::Vector normal = Math::Vector();
    glm::vec2 texCoord = { 0, 0 };
};

/**
 * \struct VertexCol
 * \brief Colored vertex
 *
 * It contains:
 *  - vertex coordinates (x,y,z) as Math::Vector,
 *  - RGBA color as Color
 */
struct VertexCol
{
    static constexpr VertexType VERTEX_TYPE = VERTEX_TYPE_COL;

    Math::Vector coord = Math::Vector();
    Color color = Color();
};

/**
 * \struct VertexTex2
 * \brief Vertex with secondary texture coordinates
 *
 * In addition to fields from Vector, it contains
 * secondary texture coordinates (u2, v2) as glm::vec2
 */
struct VertexTex2
{
    static constexpr VertexType VERTEX_TYPE = VERTEX_TYPE_TEX2;

    Math::Vector coord = Math::Vector();
    Math::Vector normal = Math::Vector();
    glm::vec2 texCoord = { 0, 0 };
    glm::vec2 texCoord2 = { 0, 0 };

    //! Sets the fields from Vertex with texCoord2 = (0,0)
    void FromVertex(const Vertex &v)
    {
        coord = v.coord;
        normal = v.normal;
        texCoord = v.texCoord;
        texCoord2 = { 0, 0 };
    }
};

/**
 * \struct Vertex2D
 * \brief 2D vertex for interface rendering, contains UV and color
 */
struct Vertex2D
{
    glm::vec2 position = { 0.0f, 0.0f };
    glm::vec2 uv = { 0.0f, 0.0f };
    glm::u8vec4 color = { 255, 255, 255, 255 };
};

/**
 * \struct Vertex3D
 * \brief 3D vertex for 3D rendering, contains UV, color and normal
 */
struct Vertex3D
{
    glm::vec3 position = { 0.0f, 0.0f, 0.0f };
    glm::u8vec4 color = { 255, 255, 255, 255 };
    glm::vec2 uv = { 0.0f, 0.0f };
    glm::vec2 uv2 = { 0.0f, 0.0f };
    glm::vec3 normal = { 0.0f, 0.0f, 1.0f };

    Vertex3D() = default;

    Vertex3D(const Vertex& vertex)
        : position(vertex.coord)
        , uv(vertex.texCoord)
        , normal(vertex.normal)
    {

    }

    Vertex3D(const VertexTex2& vertex)
        : position(vertex.coord)
        , uv(vertex.texCoord)
        , uv2(vertex.texCoord2)
        , normal(vertex.normal)
    {

    }

    operator Vertex() const
    {
        return Vertex{ position, normal, uv };
    }

    operator VertexTex2() const
    {
        return VertexTex2{ position, normal, uv, uv2 };
    }
};

} // namespace Gfx
