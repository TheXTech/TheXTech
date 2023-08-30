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

#include <Logger/logger.h>

#include "sdl_proxy/sdl_stdinc.h"
#include "sdl_proxy/sdl_assert.h"

#include "mememu.h"
#include "globals.h"
#include "global_constants.h"
#include "layers.h"
#include "game_main.h" // GamePaused
#include "main/trees.h" // treeNPCUpdate
#include "npc/npc_queues.h"
#include "main/cheat_code.h"

#include <unordered_map>
#include <functional>
#include <type_traits>
#include <limits>


#if defined(arm) && !defined(__SOFTFP__) && !defined(__VFP_FP__) && !defined(__MAVERICK__)
#   define ARM_BIDI_ENDIAN
//inline void swap_halfes(uint64_t &x)
//{
//    uint64_t y = ((x & 0xFFFFFFFF00000000) >> 32) & 0xFFFFFFFF;
//    x = ((x << 32) & 0xFFFFFFFF00000000) & y;
//}
//#else // Do nothing
//#   define swap_halfes(x)
#endif


#if 0 // Unused yet
SDL_FORCE_INLINE void toX86Endian(double in_d, uint8_t out[8])
{
    auto *in = reinterpret_cast<uint8_t*>(&in_d);

#if defined(THEXTECH_BIG_ENDIAN)
    out[0] = in[7];
    out[1] = in[6];
    out[2] = in[5];
    out[3] = in[4];
    out[4] = in[3];
    out[5] = in[2];
    out[6] = in[1];
    out[7] = in[0];
#elif defined(ARM_BIDI_ENDIAN) // some old devices
    out[4] = in[0];
    out[5] = in[1];
    out[6] = in[2];
    out[7] = in[3];
    out[0] = in[4];
    out[1] = in[5];
    out[2] = in[6];
    out[3] = in[7];
#else // normal little endian
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = in[3];
    out[4] = in[4];
    out[5] = in[5];
    out[6] = in[6];
    out[7] = in[7];
#endif
}

SDL_FORCE_INLINE void fromX86Endian(const uint8_t in[8], double &out_d)
{
    auto *out = reinterpret_cast<uint8_t*>(&out_d);

#if defined(THEXTECH_BIG_ENDIAN)
    out[0] = in[7];
    out[1] = in[6];
    out[2] = in[5];
    out[3] = in[4];
    out[4] = in[3];
    out[5] = in[2];
    out[6] = in[1];
    out[7] = in[0];
#elif defined(ARM_BIDI_ENDIAN) // some old devices
    out[4] = in[0];
    out[5] = in[1];
    out[6] = in[2];
    out[7] = in[3];
    out[0] = in[4];
    out[1] = in[5];
    out[2] = in[6];
    out[3] = in[7];
#else // normal little endian
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
    out[3] = in[3];
    out[4] = in[4];
    out[5] = in[5];
    out[6] = in[6];
    out[7] = in[7];
#endif
}
#endif


template<class T>
T f2i_cast(double in)
{
    if(std::is_same<T, uint64_t>::value)
    {
        if(in < static_cast<double>(std::numeric_limits<uint64_t>::min()) || (in > 2.0 * 0x8000000000000000))
            in = SDL_fmod(in, 2.0 * 0x8000000000000000);
        return static_cast<T>(in);
    }

    if(in < static_cast<double>(std::numeric_limits<int64_t>::min()) || in > static_cast<double>(std::numeric_limits<int64_t>::max()))
        in = SDL_fmod(in, 0x8000000000000000);

    return static_cast<T>(static_cast<int64_t>(in));
}

SDL_FORCE_INLINE void modifyByteX86(double &dst, size_t byte, uint8_t data)
{
    auto *in = reinterpret_cast<uint8_t*>(&dst);
    SDL_assert(byte < 8);

#if defined(THEXTECH_BIG_ENDIAN)
    in[7 - byte] = data;
#elif defined(ARM_BIDI_ENDIAN) // some old devices
    byte += (byte < 4) ? +4 : -4;
    in[byte] = data;
#else // normal little endian
    in[byte] = data;
#endif
}

SDL_FORCE_INLINE void modifyByteX86(float &dst, size_t byte, uint8_t data)
{
    auto *in = reinterpret_cast<uint8_t*>(&dst);
    SDL_assert(byte < 4);

#if defined(THEXTECH_BIG_ENDIAN)
    in[3 - byte] = data;
#else // normal little endian
    in[byte] = data;
#endif
}

SDL_FORCE_INLINE void modifyByteX86(int16_t &dst, size_t byte, uint8_t data)
{
    auto *in = reinterpret_cast<uint8_t*>(&dst);
    SDL_assert(byte < 2);

#if defined(THEXTECH_BIG_ENDIAN)
    in[1 - byte] = data;
#else // normal little endian
    in[byte] = data;
#endif
}


SDL_FORCE_INLINE uint8_t getByteX86(const double &src, size_t byte)
{
    const auto *in = reinterpret_cast<const uint8_t*>(&src);
    SDL_assert(byte < 8);
#if defined(THEXTECH_BIG_ENDIAN)
    return in[7 - byte];
#elif defined(ARM_BIDI_ENDIAN) // some old devices
    byte += (byte < 4) ? +4 : -4;
    return in[byte];
#else // normal little endian
    return in[byte];
#endif
}

SDL_FORCE_INLINE uint8_t getByteX86(const float &src, size_t byte)
{
    const auto *in = reinterpret_cast<const uint8_t*>(&src);
    SDL_assert(byte < 4);
#if defined(THEXTECH_BIG_ENDIAN)
    return in[3 - byte];
#else // normal little endian
    return in[byte];
#endif
}

SDL_FORCE_INLINE uint8_t getByteX86(const int16_t &src, size_t byte)
{
    const auto *in = reinterpret_cast<const uint8_t*>(&src);
    SDL_assert(byte < 2);
#if defined(THEXTECH_BIG_ENDIAN)
    return in[1 - byte];
#else // normal little endian
    return in[byte];
#endif
}


/*----------------------------------------------*
 *          Write memory value                  *
 *----------------------------------------------*/

SDL_FORCE_INLINE void memToValue(double &target, double value, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        target =static_cast<double>(f2i_cast<uint8_t>(value));
        break;
    case FT_WORD:
        target =static_cast<double>(static_cast<int16_t>(f2i_cast<uint16_t>(value)));
        break;
    case FT_DWORD:
        target =static_cast<double>(f2i_cast<int32_t>(value));
        break;
    case FT_FLOAT:
        target =static_cast<double>(static_cast<float>(value));
        break;
    case FT_DFLOAT:
        target = value;
        break;
    default:
        break; // Don't change
    }
}

SDL_FORCE_INLINE void memToValue(float &target, double value, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        target = static_cast<float>(f2i_cast<uint8_t>(value));
        break;
    case FT_WORD:
        target = static_cast<float>(static_cast<int16_t>(f2i_cast<uint16_t>(value)));
        break;
    case FT_DWORD:
        target = static_cast<float>(f2i_cast<int32_t>(value));
        break;
    case FT_FLOAT:
        target = static_cast<float>(value);
        break;
    case FT_DFLOAT:
        target = static_cast<double>(static_cast<float>(value));
        break;
    default: //Don't change
        break;
    }
}

SDL_FORCE_INLINE void memToValue(int &target, double value, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        target = static_cast<int32_t>(f2i_cast<uint8_t>(value));
        break;
    case FT_WORD:
        target = static_cast<int32_t>(static_cast<int16_t>(f2i_cast<uint16_t>(value)));
        break;
    case FT_DWORD:
    case FT_DFLOAT:
        target = static_cast<int32_t>(value);
        break;
    case FT_FLOAT:
        target = static_cast<int32_t>(f2i_cast<float>(value));
        break;
//    case FT_DFLOAT: // United with FT_DWORD
//        target = static_cast<int32_t>(value);
//        break;
    default: //Don't change
        break;
    }
}

SDL_FORCE_INLINE void memToValue(short &target, double value, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        target = static_cast<int16_t>(f2i_cast<uint8_t>(value));
        break;
    case FT_WORD:
        target = static_cast<int16_t>(static_cast<int16_t>(f2i_cast<uint16_t>(value)));
        break;
    case FT_DWORD:
    case FT_DFLOAT:
        target = static_cast<int16_t>(value);
        break;
    case FT_FLOAT:
        target = static_cast<int16_t>(f2i_cast<float>(value));
        break;
//    case FT_DFLOAT: // United with FT_DWORD
//        target = static_cast<int32_t>(value);
//        break;
    default: //Don't change
        break;
    }
}

SDL_FORCE_INLINE void memToValue(bool &target, double value, FIELDTYPE ftype)
{
    UNUSED(ftype);
    target = (value != 0.0);
}


/*----------------------------------------------*
 *           Read memory value                  *
 *----------------------------------------------*/

SDL_FORCE_INLINE double valueToMem(const double &source, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        return static_cast<double>(static_cast<uint8_t>(source));
    case FT_WORD:
        return static_cast<double>(static_cast<int16_t>(static_cast<uint16_t>(source)));
    case FT_DWORD:
        return static_cast<double>(static_cast<int32_t>(source));
    case FT_FLOAT:
        return static_cast<double>(static_cast<float>(source));
    default:
    case FT_DFLOAT:
        return source;
    }
}

SDL_FORCE_INLINE double valueToMem(const float &source, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        return static_cast<double>(static_cast<uint8_t>(source));
    case FT_WORD:
        return static_cast<double>(static_cast<int16_t>(static_cast<uint16_t>(source)));
    case FT_DWORD:
        return static_cast<double>(static_cast<int32_t>(source));
    default:
    case FT_FLOAT:
    case FT_DFLOAT:
        return static_cast<double>(source);
    }
}

SDL_FORCE_INLINE double valueToMem(const int &source, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        return static_cast<double>(static_cast<uint8_t>(source));
    case FT_WORD:
        return static_cast<double>(static_cast<int16_t>(source));
    default:
    case FT_DWORD:
        return static_cast<double>(source);
    case FT_FLOAT:
        return static_cast<double>(static_cast<float>(source));
    case FT_DFLOAT:
        return static_cast<double>(source);
    }
}

SDL_FORCE_INLINE double valueToMem(const short &source, FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        return static_cast<double>(static_cast<uint8_t>(source));
    case FT_WORD:
        return static_cast<double>(static_cast<int16_t>(source));
    default:
    case FT_DWORD:
        return static_cast<double>(source);
    case FT_FLOAT:
        return static_cast<double>(static_cast<float>(source));
    case FT_DFLOAT:
        return static_cast<double>(source);
    }
}

SDL_FORCE_INLINE double valueToMem(const bool &source, FIELDTYPE ftype)
{
    UNUSED(ftype);
    return source ? 0xFFFF : 0;
}


/*!
 * \brief Convert field type into string
 * \param ftype Field type
 * \return Human-readable string
 */
static const char *FieldtypeToStr(FIELDTYPE ftype)
{
    switch(ftype)
    {
    case FT_BYTE:
        return "Uint8";
    case FT_WORD:
        return "SInt16";
    case FT_DWORD:
        return "SInt32";
    case FT_FLOAT:
        return "Float";
    case FT_DFLOAT:
        return "Double";
    default:
        return "<Invalid>";
    }
}



/*!
 * \brief Global memory emulator
 */
class SMBXMemoryEmulator
{
    std::unordered_map<size_t, double *> m_df;
    std::unordered_map<size_t, float *>  m_ff;
    std::unordered_map<size_t, short *>  m_i16f;
    std::unordered_map<size_t, int *>    m_i32f;
    std::unordered_map<size_t, bool *>   m_bf;
    std::unordered_map<size_t, std::string *>   m_sf;

    typedef std::function<double(FIELDTYPE)> Getter;
    typedef std::function<void(double,FIELDTYPE)> Setter;
    std::unordered_map<size_t, std::pair<Getter, Setter>> m_lff;

    typedef std::function<std::string()> StrGetter;
    typedef std::function<void(const std::string&)> StrSetter;
    std::unordered_map<size_t, std::pair<StrGetter, StrSetter>> m_sff;

    enum ValueType
    {
        VT_UNKNOWN = 0,
        VT_DOUBLE,
        VT_FLOAT,
        VT_INT16,
        VT_INT32,
        VT_BOOL,
        VT_STRING,
        VT_LAMBDA,
        VT_STRLAMBDA
    };

    std::unordered_map<int, ValueType> m_type;

    void insert(size_t address, short *field)
    {
        m_i16f.insert({address, field});
        m_type.insert({address, VT_INT16});
    }

    void insert(size_t address, int *field)
    {
        m_i32f.insert({address, field});
        m_type.insert({address, VT_INT32});
    }

    void insert(size_t address, double *field)
    {
        m_df.insert({address, field});
        m_type.insert({address, VT_DOUBLE});
    }

    void insert(size_t address, float *field)
    {
        m_ff.insert({address, field});
        m_type.insert({address, VT_FLOAT});
    }

    void insert(size_t address, bool *field)
    {
        m_bf.insert({address, field});
        m_type.insert({address, VT_BOOL});
    }

    void insert(size_t address, std::string *field)
    {
        m_sf.insert({address, field});
        m_type.insert({address, VT_STRING});
    }

    void insert(size_t address, Getter g, Setter s)
    {
        m_lff.insert({address, {g, s}});
        m_type.insert({address, VT_LAMBDA});
    }

    void insert(size_t address, StrGetter g, StrSetter s)
    {
        m_sff.insert({address, {g, s}});
        m_type.insert({address, VT_STRLAMBDA});
    }

public:
    SMBXMemoryEmulator() noexcept
    {
        buildTable();
    }

    void buildTable()
    {
        insert(0x00B2504C, &TakeScreen);
        insert(0x00B250D6, &numLocked);
        insert(0x00B250E2, // Pause menu visible
            [](FIELDTYPE ftype)->double
            {
                bool tmp = (GamePaused != PauseCode::None);
                return valueToMem(tmp, ftype);
            },
            [](double in, FIELDTYPE ftype)->void
            {
                // FIXME: Verify this, if it needs to be written, try to work around this
                pLogWarning("Attempt to write the read-only field at 0x00B250E2 "
                            "(GamePaused) with value %g as %s", in, FieldtypeToStr(ftype));
            }
        );
        insert(0x00B25134, &LevelEditor);

        insert(0x00B251E0, &numStars); // HUD star count
        insert(0x00B25700, &numWater);

        insert(0x00B25724, &StartLevel);
        insert(0x00B25728, &NoMap);
        insert(0x00B2572A, &RestartLevel);

        // should all be read-only
        insert(0x00B257A4, &numTiles);
        insert(0x00B257A6, &numScenes);
        insert(0x00B258E0, &numWorldPaths);
        insert(0x00B258E2, &numWarps);
        insert(0x00B25956, &numBlock);
        insert(0x00B25958, &numBackground);
        insert(0x00B2595A, &numNPCs); // NPC count
        insert(0x00B2595E, &numPlayers); // Player Count
        insert(0x00B25960, &numWorldLevels);
        insert(0x00B25980, &numWorldMusic);

        insert(0x00B2C5A8, &Coins); // HUD coins count
        insert(0x00B2C5AC, &Lives); // HUD lives count

        insert(0x00B2C624, &WorldName);

        insert(0x00B2C62C, &PSwitchTime); // P-Switch Timer
        insert(0x00B2C62E, &PSwitchStop); // Stopwatch Timer
        insert(0x00B2C630, &PSwitchPlayer); // P-Switch/Stopwatch Player

        insert(0x00B2C684, &FrameSkip);

        insert(0x00B2C87C, &Physics.NPCPSwitch); // P-Switch/Stopwatch Length

        insert(0x00B2C880, &MenuCursor); // Current menu choice
        insert(0x00B2C882, &MenuMode); // Current menu mode

        // insert(0x00B2C884, ???}; // Key Released!!!
        // insert(0x00B2C894, &BlocksSorted); // removed by block quadtree
        insert(0x00B2C894,
            [](FIELDTYPE ftype)->double
            {
                bool ret = true;
                return valueToMem(ret, ftype);
            },
            [](double in, FIELDTYPE ftype)->void
            {
                pLogWarning("Attempt to write the read-only field at 0x00B2C894 "
                            "(BlocksSorted) with value %g as %s", in, FieldtypeToStr(ftype));
            }
        );

        insert(0x00B2C896, &SingleCoop);
        insert(0x00B2C898,
            []()->std::string
            {
                return cheats_get();
            },
            [](const std::string &in)->void
            {
                cheats_setBuffer(in);
            }
        );

        insert(0x00B2C89C, &GameOutro);
        insert(0x00B2C8A0, &CreditChop);
        insert(0x00B2C8A4, &EndCredits);

        insert(0x00B2C8A6, &curStars);
        insert(0x00B2C8A8, &maxStars);

        insert(0x00B2C8AA, &ShadowMode);
        insert(0x00B2C8AC, &MultiHop);
        insert(0x00B2C8AE, &SuperSpeed);
        insert(0x00B2C8B0, &WalkAnywhere);
        insert(0x00B2C8B2, &FlyForever);
        insert(0x00B2C8B4, &FreezeNPCs);
        insert(0x00B2C8B6, &CaptainN);
        insert(0x00B2C8B8, &FlameThrower);
        insert(0x00B2C8BA, &CoinMode);
        insert(0x00B2C8BE, &MaxFPS);
        insert(0x00B2C8C0, &GodMode);
        insert(0x00B2C8C2, &GrabAll);

        insert(0x00B2C8C4, &Cheater);

        insert(0x00B2C8E4, &Score); // HUD points count
        insert(0x00B2C906, &MaxWorldStars); // Max stars at episode

        insert(0x00B2C908, &Debugger);

        insert(0x00B2D6B8, &PlayerCharacter);
        insert(0x00B2D6BA, &PlayerCharacter2);

        insert(0x00B2D6BC, &SharedCursor.X); // Mouse cursor X
        insert(0x00B2D6C4, &SharedCursor.Y); // Mouse cursor Y
        insert(0x00B2D6CC, &SharedCursor.Primary);
        insert(0x00B2D6D0, &MenuMouseRelease);
        insert(0x00B2D6D2, &SharedCursor.Move);
        insert(0x00B2D710, &numEvents);
        insert(0x00B2D734, &noSound);
    }

    double getValue(size_t address, FIELDTYPE ftype)
    {
        if(ftype == FT_INVALID)
        {
            pLogWarning("MemEmu: Requested value of invalid type: <Global> 0x%x", address);
            return 0.0;
        }

        auto ft = m_type.find(address);
        if(ft == m_type.end())
        {
            pLogWarning("MemEmu: Unknown %s address to read: <Global> 0x%x", FieldtypeToStr(ftype), address);
            return 0.0;
        }

        switch(ft->second)
        {
        case VT_DOUBLE:
        {
            auto dres = m_df.find(address);
            if(dres != m_df.end())
            {
                if(ftype != FT_DFLOAT)
                    pLogWarning("MemEmu: Read type missmatched at 0x%x (Double expected, %s actually)", address, FieldtypeToStr(ftype));

                return valueToMem(*dres->second, ftype);
            }
            break;
        }

        case VT_FLOAT:
        {
            auto fres = m_ff.find(address);
            if(fres != m_ff.end())
            {
                if(ftype != FT_FLOAT)
                    pLogWarning("MemEmu: Read type missmatched at 0x%x (Float expected, %s actually)", address, FieldtypeToStr(ftype));

                return valueToMem(*fres->second, ftype);
            }
            break;
        }

        case VT_INT32:
        {
            auto ires = m_i32f.find(address);
            if(ires != m_i32f.end())
            {
                if(ftype != FT_DWORD && ftype != FT_WORD)
                    pLogWarning("MemEmu: Read type missmatched at 0x%x (SInt16 or SInt32 expected, %s actually)", address, FieldtypeToStr(ftype));

                return valueToMem(*ires->second, ftype);
            }
            break;
        }

        case VT_INT16:
        {
            auto ires = m_i16f.find(address);
            if(ires != m_i16f.end())
            {
                if(ftype != FT_WORD)
                    pLogWarning("MemEmu: Read type missmatched at 0x%x (SInt16 expected, %s actually)", address, FieldtypeToStr(ftype));

                return valueToMem(*ires->second, ftype);
            }
            break;
        }

        case VT_BOOL:
        {
            auto bres = m_bf.find(address);
            if(bres != m_bf.end())
            {
                if(ftype != FT_WORD && ftype != FT_BYTE)
                    pLogWarning("MemEmu: Read type missmatched at 0x%x (Sint16 or Uint8 as boolean expected, %s actually)", address, FieldtypeToStr(ftype));
                return *bres->second ? 0xffff : 0x0000;
            }
            break;
        }

        case VT_LAMBDA:
        {
            auto lfres = m_lff.find(address);
            if(lfres != m_lff.end())
            {
                auto &l = lfres->second;
                return l.first(ftype);
            }
            break;
        }

        default:
            break;
        }

        return 0.0;
    }

    void setValue(size_t address, double value, FIELDTYPE ftype)
    {
        if(ftype == FT_INVALID)
        {
            pLogWarning("MemEmu: Passed value of invalid type: <Global> 0x%x", address);
            return;
        }

        auto ft = m_type.find(address);
        if(ft == m_type.end())
        {
            pLogWarning("MemEmu: Unknown %s address to write: 0x%x", FieldtypeToStr(ftype), address);
            return;
        }

        switch(ft->second)
        {
        case VT_DOUBLE:
        {
            auto dres = m_df.find(address);
            if(dres != m_df.end())
            {
                if(ftype != FT_DFLOAT)
                    pLogWarning("MemEmu: Write type missmatched at 0x%x (Double expected, %s actually)", address, FieldtypeToStr(ftype));

                memToValue(*dres->second, value, ftype);
                return;
            }
            break;
        }

        case VT_FLOAT:
        {
            auto fres = m_ff.find(address);
            if(fres != m_ff.end())
            {
                if(ftype != FT_FLOAT)
                    pLogWarning("MemEmu: Write type missmatched at 0x%x (Float expected, %s actually)", address, FieldtypeToStr(ftype));

                memToValue(*fres->second, value, ftype);
                return;
            }
            break;
        }

        case VT_INT32:
        {
            auto ires = m_i32f.find(address);
            if(ires != m_i32f.end())
            {
                if(ftype != FT_DWORD && ftype != FT_WORD)
                    pLogWarning("MemEmu: Write type missmatched at 0x%x (SInt16 or SInt32 expected, %s actually)", address, FieldtypeToStr(ftype));

                memToValue(*ires->second, value, ftype);
                return;
            }
            break;
        }

        case VT_INT16:
        {
            auto ires = m_i16f.find(address);
            if(ires != m_i16f.end())
            {
                if(ftype != FT_WORD)
                    pLogWarning("MemEmu: Write type missmatched at 0x%x (SInt16 expected, %s actually)", address, FieldtypeToStr(ftype));

                memToValue(*ires->second, value, ftype);
                return;
            }
            break;
        }

        case VT_BOOL:
        {
            auto bres = m_bf.find(address);
            if(bres != m_bf.end())
            {
                if(ftype != FT_WORD && ftype != FT_BYTE)
                    pLogWarning("MemEmu: Write type missmatched at 0x%x (Sint16 or Uint8 as boolean expected, %s actually)", address, FieldtypeToStr(ftype));
                *bres->second = (value != 0.0);
                return;
            }
            break;
        }

        case VT_LAMBDA:
        {
            auto lfres = m_lff.find(address);
            if(lfres != m_lff.end())
            {
                auto &l = lfres->second;
                l.second(value, ftype);
            }
            break;
        }

        default:
            break;
        }
    }
};

/*!
 * \brief Per-Object memory emulator
 */
template<class T, char const *objName, size_t maxAddr>
class SMBXObjectMemoryEmulator
{
protected:

    typedef std::function<double(const T&,FIELDTYPE)> Getter;
    typedef std::function<void(T&,double,FIELDTYPE)> Setter;

    // typedef std::function<std::string(const T&)> StrGetter;
    // typedef std::function<void(T&,const std::string&)> StrSetter;

    enum ValueType
    {
        VT_UNKNOWN = 0,
        VT_DOUBLE,
        VT_FLOAT,
        VT_INT16,
        VT_INT32,
        VT_BOOL,
        VT_STRING,
        VT_BYTE_HACK,
        VT_LAMBDA,
    };

    struct Value
    {
        //! Type of field
        ValueType type = VT_UNKNOWN;
        //! Base type for byte hacking mode
        ValueType baseType = VT_UNKNOWN;
        //! Byte offset
        int offset = 0;
        //! Base address of real value
        int baseAddress = 0;

        //! Double-type field pointer
        double      T::* field_d = nullptr;
        //! Float-type field pointer
        float       T::* field_f = nullptr;
        //! Int-type field pointer
        short       T::* field_i16 = nullptr;
        //! Int-type field pointer
        int         T::* field_i32 = nullptr;
        //! Boolean type field pointer
        bool        T::* field_b = nullptr;
        //! String-type field pointer
        std::string T::* field_s = nullptr;
        //! Lambda-type field pointer
        std::pair<Getter, Setter> field_lf;
    };

    //! Basic map of addresses
    Value m_type[maxAddr];
    //! Byte map of addresses
    Value m_byte[maxAddr];

    void insert(size_t address, short T::*field)
    {
        Value v;

        // Normal field
        v.field_i16 = field;
        v.type = VT_INT16;
        v.baseType = VT_INT16;
        v.offset = 0; //-V1048
        v.baseAddress = address;
        m_type[address] = v;

        // Byte hack fields
        v.type = VT_BYTE_HACK;
        for(int i = 0; i < 2; ++i)
        {
            v.offset = i;
            m_byte[address + i] = v;
            if(i > 0)
                m_type[address + i] = v;
        }
    }

    void insert(size_t address, int T::*field)
    {
        Value v;

        // Normal field
        v.field_i32 = field;
        v.type = VT_INT32;
        v.baseType = VT_INT32;
        v.offset = 0; //-V1048
        v.baseAddress = address;
        m_type[address] = v;

        // Byte hack fields
        v.type = VT_BYTE_HACK;
        for(int i = 0; i < 2; ++i)
        {
            v.offset = i;
            m_byte[address + i] = v;
            if(i > 0)
                m_type[address + i] = v;
        }
    }

    void insert(size_t address, double T::*field)
    {
        Value v;

        // Normal field
        v.field_d = field;
        v.type = VT_DOUBLE;
        v.baseType = VT_DOUBLE;
        v.offset = 0;
        v.baseAddress = address;
        m_type[address] = v;

        // Byte hack fields
        v.type = VT_BYTE_HACK;
        for(int i = 0; i < 8; ++i)
        {
            v.offset = i;
            m_byte[address + i] = v;
            if(i > 0)
                m_type[address + i] = v;
        }
    }

    void insert(size_t address, float T::*field)
    {
        Value v;

        // Normal field
        v.field_f = field;
        v.type = VT_FLOAT;
        v.baseType = VT_FLOAT;
        v.offset = 0;
        v.baseAddress = address;
        m_type[address] = v;

        // Byte hack fields
        v.type = VT_BYTE_HACK;
        for(int i = 0; i < 4; ++i)
        {
            v.offset = i;
            m_byte[address + i] = v;
            if(i > 0)
                m_type[address + i] = v;
        }
    }

    void insert(size_t address, bool T::*field)
    {
        Value v;

        // Normal field
        v.field_b = field;
        v.type = VT_BOOL;
        v.baseType = VT_BOOL;
        v.offset = 0;
        v.baseAddress = address;
        m_type[address] = v;
    }

    void insert(size_t address, std::string T::*field)
    {
        Value v;

        // Normal field
        v.field_s = field; // -V820 // False positive, it's a pointer, not a std::string
        v.type = VT_STRING;
        v.baseType = VT_STRING;
        v.offset = 0;
        v.baseAddress = address;
        m_type[address] = v;
    }

    void insert(size_t address, Getter g, Setter s)
    {
        Value v;

        // Normal field
        v.field_lf = {g, s};
        v.type = VT_LAMBDA;
        v.baseType = VT_LAMBDA;
        v.offset = 0;
        v.baseAddress = address;
        m_type[address] = v;
    }

public:
    SMBXObjectMemoryEmulator() noexcept
    {}

    virtual double getValue(T *obj, size_t address, FIELDTYPE ftype)
    {
        Value *t = nullptr;

        if(address >= maxAddr)
        {
            pLogWarning("MemEmu: Requested value of out-of-range address: %s 0x%x", objName, address);
            return 0.0;
        }

        if(ftype == FT_INVALID)
        {
            pLogWarning("MemEmu: Requested value of invalid type: %s 0x%x", objName, address);
            return 0.0;
        }

        if(ftype == FT_BYTE) // byte hacking
        {
            t = &m_byte[address];
            if(t->type == VT_UNKNOWN)
                t = &m_type[address];
        }
        else
            t = &m_type[address];

        if(t->type == VT_UNKNOWN)
        {
            pLogWarning("MemEmu: Unknown %s::%s address to read: 0x%x", objName, FieldtypeToStr(ftype), address);
            return 0.0;
        }

        switch(t->type)
        {
        case VT_DOUBLE:
        {
            SDL_assert(t->field_d);
            if(ftype != FT_DFLOAT)
                pLogWarning("MemEmu: Read type missmatched at %s 0x%x (Double expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            return valueToMem(obj->*(t->field_d), ftype);
        }

        case VT_FLOAT:
        {
            SDL_assert(t->field_f);
            if(ftype != FT_FLOAT)
                pLogWarning("MemEmu: Read type missmatched at %s 0x%x (Float expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            return valueToMem(obj->*(t->field_f), ftype);
        }

        case VT_INT32:
        {
            SDL_assert(t->field_i32);
            if(ftype != FT_DWORD && ftype != FT_WORD)
                pLogWarning("MemEmu: Read type missmatched at %s 0x%x (SInt16 or SInt32 expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            return valueToMem(obj->*(t->field_i32), ftype);
        }

        case VT_INT16:
        {
            SDL_assert(t->field_i16);
            if(ftype != FT_WORD)
                pLogWarning("MemEmu: Read type missmatched at %s 0x%x (SInt16 expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            return valueToMem(obj->*(t->field_i16), ftype);
        }

        case VT_BOOL:
        {
            SDL_assert(t->field_b);
            if(ftype != FT_WORD && ftype != FT_BYTE)
                pLogWarning("MemEmu: Read type missmatched at %s 0x%x (Sint16 or Uint8 as boolean expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            return valueToMem(obj->*(t->field_b), ftype);
        }

        case VT_LAMBDA:
        {
            return t->field_lf.first(*obj, ftype);
        }

        case VT_BYTE_HACK:
        {
            switch(t->baseType)
            {
            case VT_DOUBLE:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_DOUBLE && bt.field_d);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Read type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                return (double)getByteX86(obj->*(bt.field_d), t->offset);
            }

            case VT_FLOAT:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_FLOAT && bt.field_f);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Read type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                return (double)getByteX86(obj->*(bt.field_f), t->offset);
            }

            case VT_INT16:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_INT16 && bt.field_i16);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Read type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                int16_t s = static_cast<int16_t>(obj->*(bt.field_i16));
                return (double)getByteX86(s, t->offset);
            }

            case VT_INT32:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_INT32 && bt.field_i32);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Read type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                int16_t s = static_cast<int16_t>(obj->*(bt.field_i32));
                return (double)getByteX86(s, t->offset);
            }

            default:
                break;
            }

            break;
        }

        default:
            break;
        }

        return 0.0;
    }

    virtual void setValue(T *obj, size_t address, double value, FIELDTYPE ftype)
    {
        Value *t = nullptr;

        if(address >= maxAddr)
        {
            pLogWarning("MemEmu: Requested value of out-of-range address: %s 0x%x", objName, address);
            return;
        }

        if(ftype == FT_INVALID)
        {
            pLogWarning("MemEmu: Passed value of invalid type: %s 0x%x", objName, address);
            return;
        }

        if(ftype == FT_BYTE) // byte hacking
        {
            t = &m_byte[address];
            if(t->type == VT_UNKNOWN)
                t = &m_type[address];
        }
        else
            t = &m_type[address];

        if(t->type == VT_UNKNOWN)
        {
            pLogWarning("MemEmu: Unknown %s::%s address to write: 0x%x", objName, FieldtypeToStr(ftype), address);
            return;
        }

        switch(t->type)
        {
        case VT_DOUBLE:
        {
            SDL_assert(t->field_d);
            if(ftype != FT_DFLOAT)
                pLogWarning("MemEmu: Write type missmatched at %s 0x%x (Double expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            memToValue(obj->*(t->field_d), value, ftype);
            return;
        }

        case VT_FLOAT:
        {
            SDL_assert(t->field_f);
            if(ftype != FT_FLOAT)
                pLogWarning("MemEmu: Write type missmatched at %s 0x%x (Float expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            memToValue(obj->*(t->field_f), value, ftype);
            return;
        }

        case VT_INT32:
        {
            SDL_assert(t->field_i32);
            if(ftype != FT_DWORD && ftype != FT_WORD)
                pLogWarning("MemEmu: Write type missmatched at %s 0x%x (SInt16 or SInt32 expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            memToValue(obj->*(t->field_i32), value, ftype);
            return;
        }

        case VT_INT16:
        {
            SDL_assert(t->field_i16);
            if(ftype != FT_WORD)
                pLogWarning("MemEmu: Write type missmatched at %s 0x%x (SInt16 expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            memToValue(obj->*(t->field_i16), value, ftype);
            return;
        }

        case VT_BOOL:
        {
            SDL_assert(t->field_b);
            if(ftype != FT_WORD && ftype != FT_BYTE)
                pLogWarning("MemEmu: Write type missmatched at %s 0x%x (Sint16 or Uint8 as boolean expected, %s actually)", objName, address, FieldtypeToStr(ftype));
            memToValue(obj->*(t->field_b), value, ftype);
            return;
        }

        case VT_LAMBDA:
        {
            t->field_lf.second(*obj, value, ftype);
            return;
        }

        case VT_BYTE_HACK:
        {
            switch(t->baseType)
            {
            case VT_DOUBLE:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_DOUBLE && bt.field_d);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Write type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                modifyByteX86(obj->*(bt.field_d), t->offset, f2i_cast<uint8_t>(value));
                return;
            }

            case VT_FLOAT:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_FLOAT && bt.field_f);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Write type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                modifyByteX86(obj->*(bt.field_f), t->offset, f2i_cast<uint8_t>(value));
                return;
            }

            case VT_INT16:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_INT16 && bt.field_i16);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Write type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                int16_t s = static_cast<int16_t>(obj->*(bt.field_i16));
                modifyByteX86(s, t->offset, f2i_cast<uint8_t>(value));
                obj->*(bt.field_i16) = static_cast<int>(s);
                return;
            }

            case VT_INT32:
            {
                auto &bt = m_type[t->baseAddress];
                SDL_assert(bt.type == VT_INT32 && bt.field_i32);
                if(ftype != FT_BYTE)
                    pLogWarning("MemEmu: Write type missmatched at %s 0x%x (byte expected, %s actually)", objName, address, FieldtypeToStr(ftype));
                int16_t s = static_cast<int16_t>(obj->*(bt.field_i32));
                modifyByteX86(s, t->offset, f2i_cast<uint8_t>(value));
                obj->*(bt.field_i32) = static_cast<int>(s);
                return;
            }

            default:
                break;
            }

            break;
        }

        default:
            break;
        }
    }
};

static constexpr char location_t_name[] = "Location_t";
typedef SMBXObjectMemoryEmulator<Location_t, location_t_name, 0x31> LocationParent;
class LocationMemory final : public LocationParent
{
public:
    LocationMemory() noexcept : LocationParent()
    {
        buildTable();
    }

    void buildTable()
    {
        insert(0x00, &Location_t::X);
        insert(0x08, &Location_t::Y);
        insert(0x10, &Location_t::Height);
        insert(0x18, &Location_t::Width);
        insert(0x20, &Location_t::SpeedX);
        insert(0x28, &Location_t::SpeedY);
    }
};


static constexpr char controls_t_name[] = "Controls_t";
typedef SMBXObjectMemoryEmulator<Controls_t, controls_t_name, 0x16> ControlsParent;
class ControlsMemory final : public ControlsParent
{
public:
    ControlsMemory() noexcept :  ControlsParent()
    {
        buildTable();
    }

    void buildTable()
    {
        insert(0x00, &Controls_t::Up);
        insert(0x02, &Controls_t::Down);
        insert(0x04, &Controls_t::Left);
        insert(0x06, &Controls_t::Right);
        insert(0x08, &Controls_t::Jump);
        insert(0x0A, &Controls_t::AltJump);
        insert(0x0C, &Controls_t::Run);
        insert(0x0E, &Controls_t::AltRun);
        insert(0x10, &Controls_t::Drop);
        insert(0x12, &Controls_t::Start);
    }
};

static ControlsMemory s_conMem;
static LocationMemory s_locMem;


static constexpr char playere_t_name[] = "Player_t";
typedef SMBXObjectMemoryEmulator<Player_t, playere_t_name, 0x186> PlayerParent;
class PlayerMemory final : public PlayerParent
{
public:
    PlayerMemory() noexcept : PlayerParent()
    {
        buildTable();
    }

    void buildTable()
    {
        insert(0x00000000, &Player_t::DoubleJump);
        insert(0x00000002, &Player_t::FlySparks);
        insert(0x00000004, &Player_t::Driving);
        insert(0x00000006, &Player_t::Quicksand);
        insert(0x00000008, &Player_t::Bombs);
        insert(0x0000000a, &Player_t::Slippy);

        insert(0x0000000c, &Player_t::Fairy);
        insert(0x0000000e, &Player_t::FairyCD);
        insert(0x00000010, &Player_t::FairyTime);
        insert(0x00000012, &Player_t::HasKey);
        insert(0x00000014, &Player_t::SwordPoke);
        insert(0x00000016, &Player_t::Hearts);

        insert(0x00000018, &Player_t::CanFloat);
        insert(0x0000001a, &Player_t::FloatRelease);
        insert(0x0000001c, &Player_t::FloatTime);
        insert(0x00000020, &Player_t::FloatSpeed);
        insert(0x00000024, &Player_t::FloatDir);
        insert(0x00000026, &Player_t::GrabTime);
        insert(0x00000028, &Player_t::GrabSpeed);
        insert(0x0000002c, &Player_t::VineNPC);

        insert(0x00000034, &Player_t::Wet);
        insert(0x00000036, &Player_t::WetFrame);
        insert(0x00000038, &Player_t::SwimCount);
        insert(0x0000003a, &Player_t::NoGravity);
        insert(0x0000003c, &Player_t::Slide);
        insert(0x0000003e, &Player_t::SlideKill);
        insert(0x00000040, &Player_t::Vine);
        insert(0x00000042, &Player_t::NoShellKick);
        insert(0x00000044, &Player_t::ShellSurf);
        insert(0x00000046, &Player_t::StateNPC);
        insert(0x00000048, &Player_t::Slope);
        insert(0x0000004a, &Player_t::Stoned);
        insert(0x0000004c, &Player_t::StonedCD);
        insert(0x0000004e, &Player_t::StonedTime);
        insert(0x00000050, &Player_t::SpinJump);
        insert(0x00000052, &Player_t::SpinFrame);
        insert(0x00000054, &Player_t::SpinFireDir);
        insert(0x00000056, &Player_t::Multiplier);
        insert(0x00000058, &Player_t::SlideCounter);
        insert(0x0000005a, &Player_t::ShowWarp);
        insert(0x0000005c, &Player_t::GroundPound);
        insert(0x0000005e, &Player_t::GroundPound2);
        insert(0x00000060, &Player_t::CanPound);
        insert(0x00000062, &Player_t::ForceHold);
        insert(0x00000064, &Player_t::YoshiYellow);
        insert(0x00000066, &Player_t::YoshiBlue);
        insert(0x00000068, &Player_t::YoshiRed);
        insert(0x0000006a, &Player_t::YoshiWingsFrame);
        insert(0x0000006c, &Player_t::YoshiWingsFrameCount);
        insert(0x0000006e, &Player_t::YoshiTX);
        insert(0x00000070, &Player_t::YoshiTY);
        insert(0x00000072, &Player_t::YoshiTFrame);
        insert(0x00000074, &Player_t::YoshiTFrameCount);
        insert(0x00000076, &Player_t::YoshiBX);
        insert(0x00000078, &Player_t::YoshiBY);
        insert(0x0000007a, &Player_t::YoshiBFrame);
        insert(0x0000007c, &Player_t::YoshiBFrameCount);
        //Location_t YoshiTongue; (Between 0x80 and 0xB0)
        insert(0x000000b0, &Player_t::YoshiTongueX);
        insert(0x000000b4, &Player_t::YoshiTongueLength);
        insert(0x000000b6, &Player_t::YoshiTonugeBool);
        insert(0x000000b8, &Player_t::YoshiNPC);
        insert(0x000000ba, &Player_t::YoshiPlayer);
        insert(0x000000bc, &Player_t::Dismount);
        insert(0x000000be, &Player_t::NoPlayerCol);
        //Location_t Location; (Between 0xC0 and 0xF0)
        insert(0x000000f0, &Player_t::Character);
        //Controls_t Controls; (Between 0xF2 and 0x105)
        insert(0x00000106, &Player_t::Direction);
        insert(0x00000108, &Player_t::Mount);
        insert(0x0000010a, &Player_t::MountType);
        insert(0x0000010c, &Player_t::MountSpecial);
        insert(0x0000010e, &Player_t::MountOffsetY);
        insert(0x00000110, &Player_t::MountFrame);
        insert(0x00000112, &Player_t::State);
        insert(0x00000114, &Player_t::Frame);
        insert(0x00000118, &Player_t::FrameCount);
        insert(0x0000011c, &Player_t::Jump);
        insert(0x0000011e, &Player_t::CanJump);
        insert(0x00000120, &Player_t::CanAltJump);
        insert(0x00000122, &Player_t::Effect);
        insert(0x00000124, &Player_t::Effect2);
        insert(0x0000012c, &Player_t::DuckRelease);
        insert(0x0000012e, &Player_t::Duck);
        insert(0x00000130, &Player_t::DropRelease);
        insert(0x00000132, &Player_t::StandUp);
        insert(0x00000134, &Player_t::StandUp2);
        insert(0x00000136, &Player_t::Bumped);
        insert(0x00000138, &Player_t::Bumped2);
        insert(0x0000013c, &Player_t::Dead);
        insert(0x0000013e, &Player_t::TimeToLive);
        insert(0x00000140, &Player_t::Immune);
        insert(0x00000142, &Player_t::Immune2);
        insert(0x00000144, &Player_t::ForceHitSpot3);

        // handlers below
        // insert(0x00000146, &Player_t::Pinched1);
        // insert(0x00000148, &Player_t::Pinched2);
        // insert(0x0000014a, &Player_t::Pinched3);
        // insert(0x0000014c, &Player_t::Pinched4);
        // insert(0x0000014e, &Player_t::NPCPinched);

        insert(0x00000150, &Player_t::m2Speed);
        insert(0x00000154, &Player_t::HoldingNPC);
        insert(0x00000156, &Player_t::CanGrabNPCs);
        insert(0x00000158, &Player_t::HeldBonus);
        insert(0x0000015a, &Player_t::Section);
        insert(0x0000015c, &Player_t::WarpCD);
        insert(0x0000015e, &Player_t::Warp);
        insert(0x00000160, &Player_t::FireBallCD);
        insert(0x00000162, &Player_t::FireBallCD2);
        insert(0x00000164, &Player_t::TailCount);
        insert(0x00000168, &Player_t::RunCount);
        insert(0x0000016c, &Player_t::CanFly);
        insert(0x0000016e, &Player_t::CanFly2);
        insert(0x00000170, &Player_t::FlyCount);
        insert(0x00000172, &Player_t::RunRelease);
        insert(0x00000174, &Player_t::JumpRelease);
        insert(0x00000176, &Player_t::StandingOnNPC);
        insert(0x00000178, &Player_t::StandingOnTempNPC);
        insert(0x0000017a, &Player_t::UnStart);
        insert(0x0000017c, &Player_t::mountBump);
        insert(0x00000180, &Player_t::SpeedFixY);
    }

    double getValue(Player_t *obj, size_t address, FIELDTYPE ftype) override
    {
        if(address >= 0x80 && address < 0xB0) // YoshiTongue
            return s_locMem.getValue(&obj->YoshiTongue, address - 0x80, ftype);
        else if(address >= 0xC0 && address < 0xF0) // Location
            return s_locMem.getValue(&obj->Location, address - 0xC0, ftype);
        else if(address >= 0xF2 && address < 0x106) // Controls
            return s_conMem.getValue(&obj->Controls, address - 0xF2, ftype);
        else if(address >= 0x146 && address < 0x150) // Pinched
        {
            switch(address)
            {
            case 0x146: // Pinched1
                return valueToMem((int)obj->Pinched.Bottom1, ftype);
            case 0x148: // Pinched2
                return valueToMem((int)obj->Pinched.Left2, ftype);
            case 0x14a: // Pinched3
                return valueToMem((int)obj->Pinched.Top3, ftype);
            case 0x14c: // Pinched4
                return valueToMem((int)obj->Pinched.Right4, ftype);
            case 0x14e: // NPCPinched
                return valueToMem((int)obj->Pinched.Moving, ftype);
            default:
                pLogWarning("MemEmu: Attempt to read player address 0x%x (invalid byte hacking)", address);
                break;
            }
        }
        return PlayerParent::getValue(obj, address, ftype);
    }

    void setValue(Player_t *obj, size_t address, double value, FIELDTYPE ftype) override
    {
        if(address >= 0x80 && address < 0xB0) // YoshiTongue
        {
            s_locMem.setValue(&obj->YoshiTongue, address - 0x80, value, ftype);
            return;
        }
        else if(address >= 0xC0 && address < 0xF0) // Location
        {
            s_locMem.setValue(&obj->Location, address - 0xC0, value, ftype);
            return;
        }
        else if(address >= 0xF2 && address < 0x106) // Controls
        {
            s_conMem.setValue(&obj->Controls, address - 0xF2, value, ftype);
            return;
        }
        else if(address >= 0x146 && address < 0x150) // Pinched
        {
            int in = 0;

            memToValue(in, value, ftype);

            // clamp to range
            if(in < 0)
                in = 0;
            else if(in > 3)
                in = 3;

            switch(address)
            {
            case 0x146: // Pinched1
                obj->Pinched.Bottom1 = in;
                return;
            case 0x148: // Pinched2
                obj->Pinched.Left2 = in;
                return;
            case 0x14a: // Pinched3
                obj->Pinched.Top3 = in;
                return;
            case 0x14c: // Pinched4
                obj->Pinched.Right4 = in;
                return;
            case 0x14e: // NPCPinched
                obj->Pinched.Moving = in;

                obj->Pinched.MovingLR = (bool)in;
                obj->Pinched.MovingUD = (bool)in;

                return;
            default:
                pLogWarning("MemEmu: Attempt to set player address 0x%x to %d (invalid byte hacking)", address, in);
                break;
            }
        }

        PlayerParent::setValue(obj, address, value, ftype);
    }
};

static constexpr char npc_t_name[] = "NPC_t";
typedef SMBXObjectMemoryEmulator<NPC_t, npc_t_name, 0x160> NpcParent;
class NPCMemory final : public NpcParent
{
public:
    NPCMemory() noexcept : NpcParent()
    {
        buildTable();
    }

    void buildTable()
    {
        // Note: strings that became indices have been blocked out temporarily
        // (mememu does not support setting strings yet)

        // insert(0x00000000, &NPC_t::AttLayer);
        insert(0x00000004, &NPC_t::Quicksand);
        insert(0x00000006, // RespawnDelay
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.RespawnDelay, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.RespawnDelay, in, ftype);

                if(n.RespawnDelay)
                    NPCQueues::RespawnDelay.insert(n);
                else
                    NPCQueues::RespawnDelay.erase(n);
            }
        );
        insert(0x00000008, // Bouce
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Bouce, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.Bouce;
                memToValue(temp, in, ftype);
                n.Bouce = temp;
            }
        );

        // handler below
        // insert(0x0000000a, &NPC_t::Pinched1);
        // insert(0x0000000c, &NPC_t::Pinched2);
        // insert(0x0000000e, &NPC_t::Pinched3);
        // insert(0x00000010, &NPC_t::Pinched4);
        // insert(0x00000012, &NPC_t::MovingPinched);

        // insert(0x00000014, &NPC_t::NetTimeout); // unused since SMBX64, now removed
        insert(0x00000018, // RealSpeedX
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.RealSpeedX, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.RealSpeedX, in, ftype);
                NPCQueues::Unchecked.push_back(n);
            }
        );
        insert(0x0000001c, &NPC_t::Wet);
        // insert(0x0000001e, &NPC_t::Settings); // unused since SMBX64, now removed
        insert(0x00000020, // NoLavaSplash
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.NoLavaSplash, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.NoLavaSplash;
                memToValue(temp, in, ftype);
                n.NoLavaSplash = temp;
            }
        );
        insert(0x00000022, &NPC_t::Slope);
        insert(0x00000024, // Multiplier
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Multiplier, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.Multiplier, in, ftype);
                NPCQueues::Unchecked.push_back(n);
            }
        );
        insert(0x00000026, // TailCD
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.TailCD, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.TailCD, in, ftype);
                NPCQueues::Unchecked.push_back(n);
            }
        );
        insert(0x00000028, &NPC_t::Shadow);
        // insert(0x0000002c, &NPC_t::TriggerActivate);
        // insert(0x00000030, &NPC_t::TriggerDeath);
        // insert(0x00000034, &NPC_t::TriggerTalk);
        // insert(0x00000038, &NPC_t::TriggerLast);
        // insert(0x0000003c, &NPC_t::Layer);
        insert(0x00000040, &NPC_t::Hidden);
        insert(0x00000042, // Legacy
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Legacy, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.Legacy;
                memToValue(temp, in, ftype);
                n.Legacy = temp;
            }
        );
        insert(0x00000044, // Chat
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Chat, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.Chat;
                memToValue(temp, in, ftype);
                n.Chat = temp;
            }
        );
        insert(0x00000046, &NPC_t::Inert);
        insert(0x00000048, &NPC_t::Stuck);
        insert(0x0000004a, // DefaultStuck
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.DefaultStuck, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.DefaultStuck;
                memToValue(temp, in, ftype);
                n.DefaultStuck = temp;
            }
        );
        // insert(0x0000004c, &NPC_t::Text);
        insert(0x00000050, &NPC_t::oldAddBelt);
        // insert(0x00000054, &NPC_t::PinchCount); // unused since SMBX64, now removed
        // insert(0x00000056, &NPC_t::Pinched); // unused since SMBX64, now removed
        // insert(0x00000058, &NPC_t::PinchedDirection); // unused since SMBX64, now removed
        insert(0x0000005c, &NPC_t::BeltSpeed);
        insert(0x00000060, &NPC_t::standingOnPlayer);
        insert(0x00000062, &NPC_t::standingOnPlayerY);
        insert(0x00000064, // Generator
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Generator, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.Generator, in, ftype);

                if(n.Generator)
                    NPCQueues::Active.insert(n);
                else if(!NPCQueues::check_active(n))
                    NPCQueues::Active.erase(n);
            }
        );
        insert(0x00000068, &NPC_t::GeneratorTimeMax);
        insert(0x0000006c, &NPC_t::GeneratorTime);
        insert(0x00000070, &NPC_t::GeneratorDirection);
        insert(0x00000072, &NPC_t::GeneratorEffect);
        insert(0x00000074, // GeneratorActive
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.GeneratorActive, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.GeneratorActive;
                memToValue(temp, in, ftype);
                n.GeneratorActive = temp;
            }
        );
        insert(0x00000076, // playerTemp
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.playerTemp, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool prev = n.playerTemp;

                bool temp = n.playerTemp;
                memToValue(temp, in, ftype);
                n.playerTemp = temp;

                if(!prev && n.playerTemp)
                    NPCQueues::PlayerTemp.push_back(n);
            }
        );
        // insert(0x00000078, &NPC_t::Location); // between 0x78 and 0xA8
        // insert(0x000000a8, &NPC_t::DefaultLocation); // between 0xA8 and 0xD8
        insert(0x000000d8, &NPC_t::DefaultDirection);
        insert(0x000000dc, &NPC_t::DefaultType);
        insert(0x000000de, &NPC_t::DefaultSpecial);
        insert(0x000000e0, &NPC_t::DefaultSpecial2);
        insert(0x000000e2, // Type
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Type, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.Type, in, ftype);

                // may have switched to/from an always-active type
                if(NPCQueues::check_active(n))
                    NPCQueues::Active.insert(n);
                else
                    NPCQueues::Active.erase(n);
            }
        );
        insert(0x000000e4, &NPC_t::Frame);
        insert(0x000000e8, &NPC_t::FrameCount);
        insert(0x000000ec, &NPC_t::Direction);
        insert(0x000000f0, &NPC_t::Special);
        insert(0x000000f8, &NPC_t::Special2);
        insert(0x00000100, &NPC_t::Special3);
        insert(0x00000108, &NPC_t::Special4);
        insert(0x00000110, &NPC_t::Special5);
        insert(0x00000118, &NPC_t::Special6);
        insert(0x00000120, // TurnAround
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.TurnAround, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.TurnAround;
                memToValue(temp, in, ftype);
                n.TurnAround = temp;
            }
        );
        insert(0x00000122, // Killed
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Killed, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool prev = (n.Killed != 0);

                memToValue(n.Killed, in, ftype);

                if(!prev && n.Killed != 0)
                    NPCQueues::Killed.push_back(n);
            }
        );
        insert(0x00000124, // Active
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Active, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.Active, in, ftype);

                if(n.Active)
                    NPCQueues::Active.insert(n);
                else if(!NPCQueues::check_active(n))
                {
                    NPCQueues::Active.erase(n);
                    NPCQueues::Unchecked.push_back(n);
                }
            }
        );
        // insert(0x00000126, &NPC_t::Reset);
        insert(0x0000012a, &NPC_t::TimeLeft);
        insert(0x0000012c, &NPC_t::HoldingPlayer);
        insert(0x0000012e, &NPC_t::CantHurt);
        insert(0x00000130, &NPC_t::CantHurtPlayer);
        insert(0x00000132, &NPC_t::BattleOwner);
        insert(0x00000134, &NPC_t::WallDeath);
        insert(0x00000136, &NPC_t::Projectile);
        insert(0x00000138, &NPC_t::Effect);
        insert(0x0000013c, &NPC_t::Effect2);
        insert(0x00000144, &NPC_t::Effect3);
        insert(0x00000146, &NPC_t::Section);
        insert(0x00000148, &NPC_t::Damage);
        insert(0x0000014c, // JustActivated
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.JustActivated, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.JustActivated, in, ftype);

                if(n.JustActivated)
                    NPCQueues::Active.insert(n);
                else if(!NPCQueues::check_active(n))
                    NPCQueues::Active.erase(n);
            }
        );
        insert(0x0000014e, &NPC_t::Block);
        insert(0x00000150, &NPC_t::tempBlock);
        insert(0x00000152, // onWall
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.onWall, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.onWall;
                memToValue(temp, in, ftype);
                n.onWall = temp;
            }
        );
        insert(0x00000154, // TurnBackWipe
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.TurnBackWipe, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                bool temp = n.TurnBackWipe;
                memToValue(temp, in, ftype);
                n.TurnBackWipe = temp;
            }
        );
        insert(0x00000156, // Immune
            [](const NPC_t& n, FIELDTYPE ftype)->double
            {
                return valueToMem(n.Immune, ftype);
            },
            [](NPC_t& n, double in, FIELDTYPE ftype)->void
            {
                memToValue(n.Immune, in, ftype);
                NPCQueues::Unchecked.push_back(n);
            }
        );
    }

    double getValue(NPC_t *obj, size_t address, FIELDTYPE ftype) override
    {
        if(address >= 0x78 && address < 0xA8) // Location
            return s_locMem.getValue(&obj->Location, address - 0x78, ftype);
        else if(address >= 0xA8 && address < 0xD8) // DefaultLocation
            return s_locMem.getValue(&obj->DefaultLocation, address - 0xA8, ftype);
        else if(address == 0x126)
            return obj->Reset[1] ? 0xFFFF : 0;
        else if(address == 0x128)
            return obj->Reset[2] ? 0xFFFF : 0;
        else if(address >= 0x0A && address < 0x14) // Pinched
        {
            switch(address)
            {
            case 0x0A: // Pinched1
                return valueToMem((short)obj->Pinched.Bottom1, ftype);
            case 0x0C: // Pinched2
                return valueToMem((short)obj->Pinched.Left2, ftype);
            case 0x0E: // Pinched3
                return valueToMem((short)obj->Pinched.Top3, ftype);
            case 0x10: // Pinched4
                return valueToMem((short)obj->Pinched.Right4, ftype);
            case 0x12: // MovingPinched
                return valueToMem((short)obj->Pinched.Moving, ftype);
            default:
                pLogWarning("MemEmu: Attempt to read NPC address 0x%x (invalid byte hacking)", address);
                break;
            }
        }
        return NpcParent::getValue(obj, address, ftype);
    }

    void setValue(NPC_t *obj, size_t address, double value, FIELDTYPE ftype) override
    {
        if(address >= 0x78 && address < 0xA8) // Location
        {
            s_locMem.setValue(&obj->Location, address - 0x78, value, ftype);

            NPCQueues::Unchecked.push_back(obj);
            treeNPCUpdate(obj);
            return;
        }
        else if(address >= 0xA8 && address < 0xD8) // DefaultLocation
        {
            s_locMem.setValue(&obj->DefaultLocation, address - 0xA8, value, ftype);
            return;
        }
        else if(address == 0x126)
        {
            obj->Reset[1] = value != 0;

            if(value == 0)
                NPCQueues::NoReset.push_back(obj);

            return;
        }
        else if(address == 0x128)
        {
            obj->Reset[2] = value != 0;

            if(value == 0)
                NPCQueues::NoReset.push_back(obj);

            return;
        }
        else if(address >= 0x0A && address < 0x14) // Pinched
        {
            int in = 0;

            memToValue(in, value, ftype);

            // clamp to range
            if(in < 0)
                in = 0;
            else if(in > 3)
                in = 3;

            switch(address)
            {
            case 0x0A: // Pinched1
                obj->Pinched.Bottom1 = in;
                return;
            case 0x0C: // Pinched2
                obj->Pinched.Left2 = in;
                return;
            case 0x0E: // Pinched3
                obj->Pinched.Top3 = in;
                return;
            case 0x10: // Pinched4
                obj->Pinched.Right4 = in;
                return;
            case 0x12: // MovingPinched
                obj->Pinched.Moving = in;
                return;
            default:
                pLogWarning("MemEmu: Attempt to set NPC address 0x%x to %d (invalid byte hacking)", address, in);
                return;
            }
        }

        NpcParent::setValue(obj, address, value, ftype);

#if 0 // Layer sync hook, but setting it is not yet supported.
        if(address == 0x3C)
        {
            int index = obj - &NPC[0];
            if(index >= -128 && index <= maxNPCs)
                syncLayers_NPC(index);
        }
#endif
    }
};

static SMBXMemoryEmulator   s_emu;
static PlayerMemory         s_emuPlayer;
static NPCMemory            s_emuNPC;

template<typename T, class D>
SDL_FORCE_INLINE void opAdd(D &mem, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(addr, ftype);
    T res = static_cast<T>(o1) + static_cast<T>(o2);
    mem.setValue(addr, static_cast<double>(res), ftype);
}

template<typename T, class D>
SDL_FORCE_INLINE void opSub(D &mem, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(addr, ftype);
    T res = static_cast<T>(o1) - static_cast<T>(o2);
    mem.setValue(addr, static_cast<double>(res), ftype);
}

template<typename T, class D>
SDL_FORCE_INLINE void opMul(D &mem, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(addr, ftype);
    T res = static_cast<T>(o1) * static_cast<T>(o2);
    mem.setValue(addr, static_cast<double>(res), ftype);
}

template<typename T, class D>
SDL_FORCE_INLINE void opDiv(D &mem, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(addr, ftype);
    T res = static_cast<T>(o1) / static_cast<T>(o2);
    mem.setValue(addr, static_cast<double>(res), ftype);
}

template<typename T, class D>
SDL_FORCE_INLINE void opXor(D &mem, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(addr, ftype);
    T res = static_cast<T>(o1) ^ static_cast<T>(o2);
    mem.setValue(addr, static_cast<double>(res), ftype);
}


void MemAssign(size_t address, double value, OPTYPE operation, FIELDTYPE ftype)
{
    if(address < GM_BASE || address > GM_END)
    {
        pLogWarning("MemEmu: MemAssign Requested value of out-of-range global address: 0x%x", address);
        return;
    }

    if(ftype == FT_INVALID)
        return;

    if(operation == OP_Div && value == 0)
        return;

    switch(operation)
    {
    case OP_Assign:
        s_emu.setValue(address, value, ftype);
        break;

    case OP_Add:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opAdd<uint8_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opAdd<int16_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opAdd<int32_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opAdd<float>(s_emu, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opAdd<double>(s_emu, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Add
    break;

    case OP_Sub:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opSub<uint8_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opSub<int16_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opSub<int32_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opSub<float>(s_emu, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opSub<double>(s_emu, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Sub
    break;

    case OP_Mult:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opMul<uint8_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opMul<int16_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opMul<int32_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opMul<float>(s_emu, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opMul<double>(s_emu, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Mult
    break;

    case OP_Div:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opDiv<uint8_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opDiv<int16_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opDiv<int32_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opDiv<float>(s_emu, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opDiv<double>(s_emu, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Div
    break;

    case OP_XOR:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opXor<uint8_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opXor<int16_t>(s_emu, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opXor<int32_t>(s_emu, address, value, ftype);
            break;
        }
        default:
            break;
        }
    }//OP XOR
    break;

    default:
        break;
    }// switch on op
}

bool CheckMem(size_t address, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
    if(address < GM_BASE || address > GM_END)
    {
        pLogWarning("MemEmu: CheckMem Requested value of out-of-range global address: 0x%x", address);
        return false;
    }

    double cur = s_emu.getValue(address, ftype);

    switch(ctype)
    {
    case CMPT_EQUALS:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) == static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) == static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) == static_cast<int32_t>(value);
        case FT_FLOAT:
            return fEqual(static_cast<float>(cur), static_cast<float>(value));
        case FT_DFLOAT:
            return fEqual(cur, value);
        default:
            return false;
        }

    case CMPT_GREATER:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) > static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) > static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) > static_cast<int32_t>(value);
        case FT_FLOAT:
            return static_cast<float>(cur) > static_cast<float>(value);
        case FT_DFLOAT:
            return cur > value;
        default:
            return false;
        }

    case CMPT_LESS:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) < static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) < static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) < static_cast<int32_t>(value);
        case FT_FLOAT:
            return static_cast<float>(cur) < static_cast<float>(value);
        case FT_DFLOAT:
            return cur < value;
        default:
            return false;
        }

    case CMPT_NOTEQ:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) != static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) != static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) != static_cast<int32_t>(value);
        case FT_FLOAT:
            return !fEqual(static_cast<float>(cur), static_cast<float>(value));
        case FT_DFLOAT:
            return !fEqual(cur, value);
        default:
            return false;
        }
    }

    return false;
}

double GetMem(size_t addr, FIELDTYPE ftype)
{
    if(addr < GM_BASE || addr > GM_END)
    {
        pLogWarning("MemEmu: GetMem Requested value of out-of-range global address: 0x%x", addr);
        return 0.0;
    }

    double cur = s_emu.getValue(addr, ftype);

    switch(ftype)
    {
    case FT_BYTE:
        return static_cast<double>(static_cast<uint8_t>(cur));
    case FT_WORD:
        return static_cast<double>(static_cast<int16_t>(cur));
    case FT_DWORD:
        return static_cast<double>(static_cast<int32_t>(cur));
    case FT_FLOAT:
        return static_cast<double>(static_cast<float>(cur));
    default:
    case FT_DFLOAT:
        return cur;
    }
}


template<typename T, class D, class U>
SDL_FORCE_INLINE void opAdd(D &mem, U *obj, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(obj, addr, ftype);
    T res = static_cast<T>(o1) + static_cast<T>(o2);
    mem.setValue(obj, addr, static_cast<double>(res), ftype);
}

template<typename T, class D, class U>
SDL_FORCE_INLINE void opSub(D &mem, U *obj, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(obj, addr, ftype);
    T res = static_cast<T>(o1) - static_cast<T>(o2);
    mem.setValue(obj, addr, static_cast<double>(res), ftype);
}

template<typename T, class D, class U>
SDL_FORCE_INLINE void opMul(D &mem, U *obj, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(obj, addr, ftype);
    T res = static_cast<T>(o1) * static_cast<T>(o2);
    mem.setValue(obj, addr, static_cast<double>(res), ftype);
}

template<typename T, class D, class U>
SDL_FORCE_INLINE void opDiv(D &mem, U *obj, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(obj, addr, ftype);
    T res = static_cast<T>(o1) / static_cast<T>(o2);
    mem.setValue(obj, addr, static_cast<double>(res), ftype);
}

template<typename T, class D, class U>
SDL_FORCE_INLINE void opXor(D &mem, U *obj, size_t addr, double o2, FIELDTYPE ftype)
{
    double o1 = mem.getValue(obj, addr, ftype);
    T res = static_cast<T>(o1) ^ static_cast<T>(o2);
    mem.setValue(obj, addr, static_cast<double>(res), ftype);
}

template<class T, class U>
static void MemAssignType(T &mem, U *obj, size_t address, double value, OPTYPE operation, FIELDTYPE ftype)
{
    if(ftype == FT_INVALID)
        return;

    if(operation == OP_Div && value == 0)
        return;

    switch(operation)
    {
    case OP_Assign:
        mem.setValue(obj, address, value, ftype);
        break;

    case OP_Add:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opAdd<uint8_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opAdd<int16_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opAdd<int32_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opAdd<float>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opAdd<double>(mem, obj, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Add
    break;

    case OP_Sub:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opSub<uint8_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opSub<int16_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opSub<int32_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opSub<float>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opSub<double>(mem, obj, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Sub
    break;

    case OP_Mult:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opMul<uint8_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opMul<int16_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opMul<int32_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opMul<float>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opMul<double>(mem, obj, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Mult
    break;

    case OP_Div:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opDiv<uint8_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opDiv<int16_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opDiv<int32_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_FLOAT:
        {
            opDiv<float>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DFLOAT:
            opDiv<double>(mem, obj, address, value, ftype);
            break;
        default:
            break;
        }
    }//OP Div
    break;

    case OP_XOR:
    {
        switch(ftype)
        {
        case FT_BYTE:
        {
            opXor<uint8_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_WORD:
        {
            opXor<int16_t>(mem, obj, address, value, ftype);
            break;
        }
        case FT_DWORD:
        {
            opXor<int32_t>(mem, obj, address, value, ftype);
            break;
        }
        default:
            break;
        }
    }//OP XOR
    break;

    default:
        break;
    }// switch on op
}

template<class T, class U>
static bool ChecmMemType(T &mem, U *obj, size_t offset, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
    double cur = mem.getValue(obj, offset, ftype);

    switch(ctype)
    {
    case CMPT_EQUALS:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) == static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) == static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) == static_cast<int32_t>(value);
        case FT_FLOAT:
            return fEqual(static_cast<float>(cur), static_cast<float>(value));
        case FT_DFLOAT:
            return fEqual(cur, value);
        default:
            return false;
        }

    case CMPT_GREATER:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) > static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) > static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) > static_cast<int32_t>(value);
        case FT_FLOAT:
            return static_cast<float>(cur) > static_cast<float>(value);
        case FT_DFLOAT:
            return cur > value;
        default:
            return false;
        }

    case CMPT_LESS:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) < static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) < static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) < static_cast<int32_t>(value);
        case FT_FLOAT:
            return static_cast<float>(cur) < static_cast<float>(value);
        case FT_DFLOAT:
            return cur < value;
        default:
            return false;
        }

    case CMPT_NOTEQ:
        switch(ftype)
        {
        case FT_BYTE:
            return static_cast<uint8_t>(cur) != static_cast<uint8_t>(value);
        case FT_WORD:
            return static_cast<int16_t>(cur) != static_cast<int16_t>(value);
        case FT_DWORD:
            return static_cast<int32_t>(cur) != static_cast<int32_t>(value);
        case FT_FLOAT:
            return !fEqual(static_cast<float>(cur), static_cast<float>(value));
        case FT_DFLOAT:
            return !fEqual(cur, value);
        default:
            return false;
        }
    }

    return false;
}


// #define DEBUG_MEMEMU_TRACE

void MemAssign(Player_t *obj, size_t address, double value, OPTYPE operation, FIELDTYPE ftype)
{
#ifdef DEBUG_MEMEMU_TRACE
    D_pLogDebug("Player mem TRACE: Write 0x%08X, %g, op-%d, ft-%d", address, value, (int)operation, (int)ftype);
#endif
    MemAssignType(s_emuPlayer, obj, address, value, operation, ftype);
}

bool CheckMem(Player_t *obj, size_t offset, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
#ifdef DEBUG_MEMEMU_TRACE
    D_pLogDebug("Player mem TRACE: Compare 0x%08X, %g, cp-%d, ft-%d", offset, value, (int)ctype, (int)ftype);
#endif
    return ChecmMemType(s_emuPlayer, obj, offset, value, ctype, ftype);
}

double GetMem(Player_t *obj, size_t offset, FIELDTYPE ftype)
{
#ifdef DEBUG_MEMEMU_TRACE
    double value = s_emuPlayer.getValue(obj, offset, ftype);
    D_pLogDebug("Player mem TRACE: Read 0x%08X, %g, ft-%d", offset, value, (int)ftype);
    return value;
#else
    return s_emuPlayer.getValue(obj, offset, ftype);
#endif
}


void MemAssign(NPC_t *obj, size_t address, double value, OPTYPE operation, FIELDTYPE ftype)
{
#ifdef DEBUG_MEMEMU_TRACE
    D_pLogDebug("NPC mem TRACE: Write 0x%08X, %g, op-%d, ft-%d", address, value, (int)operation, (int)ftype);
#endif
    MemAssignType(s_emuNPC, obj, address, value, operation, ftype);
}

bool CheckMem(NPC_t *obj, size_t offset, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
#ifdef DEBUG_MEMEMU_TRACE
    D_pLogDebug("NPC mem TRACE: Compare 0x%08X, %g, cp-%d, ft-%d", offset, value, (int)ctype, (int)ftype);
#endif
    return ChecmMemType(s_emuNPC, obj, offset, value, ctype, ftype);
}

double GetMem(NPC_t *obj, size_t offset, FIELDTYPE ftype)
{
#ifdef DEBUG_MEMEMU_TRACE
    double value = s_emuNPC.getValue(obj, offset, ftype);
    D_pLogDebug("NPC mem TRACE: Read 0x%08X, %g, ft-%d", offset, value, (int)ftype);
    return value;
#else
    return s_emuNPC.getValue(obj, offset, ftype);
#endif
}
