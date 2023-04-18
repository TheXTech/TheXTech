/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2023 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#ifndef STD_PICTURE_LOAD_BASE_H
#define STD_PICTURE_LOAD_BASE_H

#define PICTURE_LOAD_NORMAL

#include <cstdint>
#include <vector>

/*!
 * \brief Generic image loading store.
 *
 * If needed somehing unusual, please define alternative structure instead of this
 */
struct StdPictureLoad
{
    //! Is this a lazy-loaded texture?
    bool lazyLoaded = false;

    //! Original compressed data of the front image
    std::vector<char> raw;
    //! Original compressed data of the mask image (if presented)
    std::vector<char> rawMask;
    //! Was mask restored from the PNG at default graphics?
    bool isMaskPng = false;


#ifdef THEXTECH_BUILD_GL_MODERN
    //! Original vertex shader source (if presented) -- CURRENTLY USED ONLY FOR PARTICLE SYSTEMS
    std::vector<char> particleVertexShaderSource;
    //! Original fragment shader source (if presented)
    std::vector<char> fragmentShaderSource;

    //! Shader uniform variables registered
    std::vector<std::string> registeredUniforms;
    //! Most recent values for shader uniform variables
    std::vector<UniformValue_t> finalUniformState;
#endif


    // Transparent color for BMP and JPEG
    bool     colorKey = false;
    uint8_t  keyRgb[3] = {0 /*R*/, 0 /*G*/, 0 /*B*/};

    /*!
     * \brief Can a picture be reloaded from this load struct?
     */
    inline bool canLoad() const
    {
        return lazyLoaded;
    }
};

#endif // STD_PICTURE_LOAD_BASE_H
