/*
 * TheXTech - A platform game engine ported from old source code for VB6
 *
 * Copyright (c) 2009-2011 Andrew Spinks, original VB6 code
 * Copyright (c) 2020-2021 Vitaly Novichkov <admin@wohlnet.ru>
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

#include "mememu.h"
#include "globals.h"
#include "global_constants.h"

#include <unordered_map>

/*!
 * \brief Global memory emulator
 */
class SMBXMemoryEmulator
{
    std::unordered_map<int, double *> m_df;
    std::unordered_map<int, float *>  m_ff;
    std::unordered_map<int, int *>    m_if;
    std::unordered_map<int, bool *>   m_bf;
    std::unordered_map<int, std::string *>   m_sf;

public:
    SMBXMemoryEmulator()
    {
        buildTable();
    }

    void buildTable()
    {
        m_bf.clear();
        m_bf.insert({0xB25728, &NoMap});
        m_bf.insert({0xB2572A, &RestartLevel});
        m_bf.insert({0x00B2D6D2, &MenuMouseMove});
        m_bf.insert({0x00B2D6D0, &MenuMouseRelease});
        m_bf.insert({0x00B2D6CC, &MenuMouseDown});
        m_bf.insert({0x00B250E2, &GamePaused}); // Pause menu visible

        // m_bf.insert({0x00B2C884, ???}); // Key Released!!!

        m_df.clear();
        m_df.insert({0x00B2D6BC, &MenuMouseX}); // Mouse cursor X
        m_df.insert({0x00B2D6C4, &MenuMouseY}); // Mouse cursor Y

        m_if.clear();
        m_if.insert({0x00B2595E, &numPlayers}); // Player Count
        m_if.insert({0x00B2595A, &numNPCs}); // NPC count

        m_if.insert({0xB2C906, &numStars}); // HUD star count

        m_if.insert({0x00B251E0, &maxStars}); // HUD star count
        m_if.insert({0x00B2C5A8, &Coins}); // HUD coins count
        m_if.insert({0x00B2C880, &MenuCursor}); // Current menu choice
        m_if.insert({0x00B2C8E4, &Score}); // HUD points count

        m_if.insert({0x00B2C62C, &PSwitchTime}); // P-Switch Timer
        m_if.insert({0x00B2C62E, &PSwitchStop}); // Stopwatch Timer
        m_if.insert({0x00B2C630, &PSwitchPlayer}); // P-Switch/Stopwatch Player
        m_if.insert({0x00B2C87C, &Physics.NPCPSwitch}); // P-Switch/Stopwatch Length

        m_ff.clear();
        m_ff.insert({0x00B2C5AC, &Lives}); // HUD lives count

        m_sf.clear();
        m_sf.insert({0xB2C624, &WorldName});
        m_sf.insert({0xB25724, &StartLevel});
    }

    double getAny(int address)
    {
        auto dres = m_df.find(address);
        if(dres != m_df.end())
            return *dres->second;

        auto fres = m_df.find(address);
        if(fres != m_df.end())
            return (double) * fres->second;

        auto ires = m_if.find(address);
        if(ires != m_if.end())
            return (double) * ires->second;

        auto bres = m_bf.find(address);
        if(bres != m_bf.end())
            return *bres->second ? 0xffff : 0000;

        pLogWarning("MemEmu: Unknown <any> address to read: %x", address);
        return 0.0;
    }

    void setAny(int address, double value)
    {
        auto dres = m_df.find(address);
        if(dres != m_df.end())
        {
            *dres->second = value;
            return;
        }

        auto fres = m_df.find(address);
        if(fres != m_df.end())
        {
            *dres->second = (float)value;
            return;
        }

        auto ires = m_if.find(address);
        if(ires != m_if.end())
        {
            *dres->second = (int)value;
            return;
        }

        auto bres = m_bf.find(address);
        if(bres != m_bf.end())
        {
            *dres->second = (value != 0.0);
            return;
        }

        pLogWarning("MemEmu: Unknown <any> address to write: %x", address);
    }


//    double getDouble(int address)
//    {
//        auto res = m_df.find(address);
//        if(res == m_df.end())
//        {
//            pLogWarning("MemEmu: Unknown double address to read: %x", address);
//            return 0.0;
//        }
//        return *res->second;
//    }

//    void setDouble(int address, double value)
//    {
//        auto res = m_df.find(address);
//        if(res == m_df.end())
//        {
//            pLogWarning("MemEmu: Unknown double address to write: %x", address);
//            return;
//        }
//        *res->second = value;
//    }



//    float getFloat(int address)
//    {
//        auto res = m_ff.find(address);
//        if(res == m_ff.end())
//        {
//            pLogWarning("MemEmu: Unknown float address to read: %x", address);
//            return 0.0;
//        }
//        return *res->second;
//    }

//    void setFloat(int address, float value)
//    {
//        auto res = m_ff.find(address);
//        if(res == m_ff.end())
//        {
//            pLogWarning("MemEmu: Unknown float address to write: %x", address);
//            return;
//        }
//        *res->second = value;
//    }



//    int getInt(int address)
//    {
//        auto res = m_if.find(address);
//        if(res == m_if.end())
//        {
//            pLogWarning("MemEmu: Unknown int address to read: %x", address);
//            return 0.0;
//        }
//        return *res->second;
//    }

//    void setInt(int address, double value)
//    {
//        auto res = m_if.find(address);
//        if(res == m_if.end())
//        {
//            pLogWarning("MemEmu: Unknown int address to write: %x", address);
//            return;
//        }
//        *res->second = value;
//    }

//    bool getBool(int address)
//    {
//        auto res = m_bf.find(address);
//        if(res == m_bf.end())
//        {
//            pLogWarning("MemEmu: Unknown bool address to read: %x", address);
//            return 0.0;
//        }
//        return *res->second;
//    }

//    void setBool(int address, double value)
//    {
//        auto res = m_bf.find(address);
//        if(res == m_bf.end())
//        {
//            pLogWarning("MemEmu: Unknown bool address to write: %x", address);
//            return;
//        }
//        *res->second = value;
//    }

//    std::string getString(int address)
//    {
//        auto res = m_sf.find(address);
//        if(res == m_sf.end())
//        {
//            pLogWarning("MemEmu: Unknown string address to read: %x", address);
//            return std::string();
//        }
//        return *res->second;
//    }

//    void setString(int address, const std::string &value)
//    {
//        auto res = m_sf.find(address);
//        if(res == m_sf.end())
//        {
//            pLogWarning("MemEmu: Unknown string address to write: %x", address);
//            return;
//        }
//        *res->second = value;
//    }
};

/*!
 * \brief Per-Object memory emulator
 */
template<class T, char const *objName>
class SMBXObjectMemoryEmulator
{
protected:
    std::unordered_map<int, double T::*> m_df;
    std::unordered_map<int, float T::*>  m_ff;
    std::unordered_map<int, int T::*>    m_if;
    std::unordered_map<int, bool T::*>   m_bf;
    std::unordered_map<int, std::string T::*>   m_sf;

public:
    SMBXObjectMemoryEmulator()
    {}

    virtual double getAny(T *obj, int address)
    {
        auto dres = m_df.find(address);
        if(dres != m_df.end())
            return obj->*(dres->second);

        auto fres = m_df.find(address);
        if(fres != m_df.end())
            return (double)(obj->*(fres->second));

        auto ires = m_if.find(address);
        if(ires != m_if.end())
            return (double)(obj->*(ires->second));

        auto bres = m_bf.find(address);
        if(bres != m_bf.end())
            return obj->*(bres->second) ? 0xffff : 0000;

        pLogWarning("MemEmu: Unknown %s::<any> address to read: %x", objName, address);
        return 0.0;
    }

    virtual void setAny(T *obj, int address, double value)
    {
        auto dres = m_df.find(address);
        if(dres != m_df.end())
        {
            obj->*(dres->second) = value;
            return;
        }

        auto fres = m_df.find(address);
        if(fres != m_df.end())
        {
            obj->*(fres->second) = (float)value;
            return;
        }

        auto ires = m_if.find(address);
        if(ires != m_if.end())
        {
            obj->*(ires->second) = (int)value;
            return;
        }

        auto bres = m_bf.find(address);
        if(bres != m_bf.end())
        {
            obj->*(bres->second) = (value != 0.0);
            return;
        }

        pLogWarning("MemEmu: Unknown %s::<any> address to write: %x", objName, address);
    }

//    double getDouble(T *obj, int address)
//    {
//        auto res = m_df.find(address);
//        if(res == m_df.end())
//        {
//            pLogWarning("MemEmu: Unknown %s::double address to read: %x", objName, address);
//            return 0.0;
//        }
//        return obj->*(res->second);
//    }

//    void setDouble(T *obj, int address, double value)
//    {
//        auto res = m_df.find(address);
//        if(res == m_df.end())
//        {
//            pLogWarning("MemEmu: Unknown %s::double address to write: %x", objName, address);
//            return;
//        }
//        obj->*(res->second) = value;
//    }



//    float getFloat(T *obj, int address)
//    {
//        auto res = m_ff.find(address);
//        if(res == m_ff.end())
//        {
//            pLogWarning("MemEmu: Unknown %s::float address to read: %x", objName, address);
//            return 0.0;
//        }
//        return obj->*(res->second);
//    }

//    void setFloat(T *obj, int address, float value)
//    {
//        auto res = m_ff.find(address);
//        if(res == m_ff.end())
//        {
//            pLogWarning("MemEmu: Unknown %s::float address to write: %x", objName, address);
//            return;
//        }
//        obj->*(res->second) = value;
//    }



//    int getInt(T *obj, int address)
//    {
//        auto res = m_if.find(address);
//        if(res == m_if.end())
//        {
//            pLogWarning("MemEmu: Unknown %s::int address to read: %x", objName, address);
//            return 0.0;
//        }
//        return obj->*(res->second);
//    }

//    void setInt(T *obj, int address, double value)
//    {
//        auto res = m_if.find(address);
//        if(res == m_if.end())
//        {
//            pLogWarning("MemEmu: Unknown %s::int address to write: %x", objName, address);
//            return;
//        }
//        obj->*(res->second) = value;
//    }

//    bool getBool(T *obj, int address)
//    {
//        auto res = m_bf.find(address);
//        if(res == m_bf.end())
//        {
//            pLogWarning("MemEmu: Unknown objName::bool address to read: %x", objName, address);
//            return 0.0;
//        }
//        return obj->*(res->second);
//    }

//    void setBool(T *obj, int address, double value)
//    {
//        auto res = m_bf.find(address);
//        if(res == m_bf.end())
//        {
//            pLogWarning("MemEmu: Unknown objName::bool address to write: %x", objName, address);
//            return;
//        }
//        obj->*(res->second) = value;
//    }

//    std::string getString(T *obj, int address)
//    {
//        auto res = m_sf.find(address);
//        if(res == m_sf.end())
//        {
//            pLogWarning("MemEmu: Unknown objName::string address to read: %x", objName, address);
//            return std::string();
//        }
//        return obj->*(res->second);
//    }

//    void setString(T *obj, int address, const std::string &value)
//    {
//        auto res = m_sf.find(address);
//        if(res == m_sf.end())
//        {
//            pLogWarning("MemEmu: Unknown objName::string address to write: %x", objName, address);
//            return;
//        }
//        obj->*(res->second) = value;
//    }
};

static const char location_t_name[] = "Location_t";
class LocationMemory final : public SMBXObjectMemoryEmulator<Location_t, location_t_name>
{
public:
    LocationMemory() : SMBXObjectMemoryEmulator<Location_t, location_t_name>()
    {
        buildTable();
    }

    void buildTable()
    {
        m_df.clear();
        m_df.insert({0x00, &Location_t::X});
        m_df.insert({0x08, &Location_t::Y});
        m_df.insert({0x10, &Location_t::Height});
        m_df.insert({0x18, &Location_t::Width});
        m_df.insert({0x20, &Location_t::SpeedX});
        m_df.insert({0x28, &Location_t::SpeedY});
    }
};


static const char controls_t_name[] = "Controls_t";
class ControlsMemory final : public SMBXObjectMemoryEmulator<Controls_t, controls_t_name>
{
public:
    ControlsMemory() : SMBXObjectMemoryEmulator<Controls_t, controls_t_name>()
    {
        buildTable();
    }

    void buildTable()
    {
        m_bf.clear();
        m_bf.insert({0x00, &Controls_t::Up});
        m_bf.insert({0x02, &Controls_t::Down});
        m_bf.insert({0x04, &Controls_t::Left});
        m_bf.insert({0x06, &Controls_t::Right});
        m_bf.insert({0x08, &Controls_t::Jump});
        m_bf.insert({0x0A, &Controls_t::AltJump});
        m_bf.insert({0x0C, &Controls_t::Run});
        m_bf.insert({0x0E, &Controls_t::AltRun});
        m_bf.insert({0x10, &Controls_t::Drop});
        m_bf.insert({0x12, &Controls_t::Start});
    }
};

static ControlsMemory s_conMem;
static LocationMemory s_locMem;


static const char playere_t_name[] = "Player_t";
typedef SMBXObjectMemoryEmulator<Player_t, playere_t_name> PlayerParent;
class PlayerMemory final : public PlayerParent
{
public:
    PlayerMemory() : PlayerParent()
    {
        buildTable();
    }

    void buildTable()
    {
        m_bf.clear();
        m_df.clear();
        m_if.clear();
        m_ff.clear();
        m_sf.clear();

        m_bf.insert({0x00000000, &Player_t::DoubleJump});
        m_bf.insert({0x00000002, &Player_t::FlySparks});
        m_bf.insert({0x00000004, &Player_t::Driving});
        m_if.insert({0x00000006, &Player_t::Quicksand});
        m_if.insert({0x00000008, &Player_t::Bombs});
        m_bf.insert({0x0000000a, &Player_t::Slippy});

        m_bf.insert({0x0000000c, &Player_t::Fairy});
        m_if.insert({0x0000000e, &Player_t::FairyCD});
        m_if.insert({0x00000010, &Player_t::FairyTime});
        m_bf.insert({0x00000012, &Player_t::HasKey});
        m_if.insert({0x00000014, &Player_t::SwordPoke});
        m_if.insert({0x00000016, &Player_t::Hearts});

        m_bf.insert({0x00000018, &Player_t::CanFloat});
        m_bf.insert({0x0000001a, &Player_t::FloatRelease});
        m_if.insert({0x0000001c, &Player_t::FloatTime});
        m_ff.insert({0x00000020, &Player_t::FloatSpeed});
        m_if.insert({0x00000024, &Player_t::FloatDir});
        m_if.insert({0x00000026, &Player_t::GrabTime});
        m_ff.insert({0x00000028, &Player_t::GrabSpeed});
        m_df.insert({0x0000002c, &Player_t::VineNPC});

        m_if.insert({0x00000034, &Player_t::Wet});
        m_bf.insert({0x00000036, &Player_t::WetFrame});
        m_if.insert({0x00000038, &Player_t::SwimCount});
        m_if.insert({0x0000003a, &Player_t::NoGravity});
        m_bf.insert({0x0000003c, &Player_t::Slide});
        m_bf.insert({0x0000003e, &Player_t::SlideKill});
        m_if.insert({0x00000040, &Player_t::Vine});
        m_if.insert({0x00000042, &Player_t::NoShellKick});
        m_bf.insert({0x00000044, &Player_t::ShellSurf});
        m_if.insert({0x00000046, &Player_t::StateNPC});
        m_if.insert({0x00000048, &Player_t::Slope});
        m_bf.insert({0x0000004a, &Player_t::Stoned});
        m_if.insert({0x0000004c, &Player_t::StonedCD});
        m_if.insert({0x0000004e, &Player_t::StonedTime});
        m_bf.insert({0x00000050, &Player_t::SpinJump});
        m_if.insert({0x00000052, &Player_t::SpinFrame});
        m_if.insert({0x00000054, &Player_t::SpinFireDir});
        m_if.insert({0x00000056, &Player_t::Multiplier});
        m_if.insert({0x00000058, &Player_t::SlideCounter});
        m_if.insert({0x0000005a, &Player_t::ShowWarp});
        m_bf.insert({0x0000005c, &Player_t::GroundPound});
        m_bf.insert({0x0000005e, &Player_t::GroundPound2});
        m_bf.insert({0x00000060, &Player_t::CanPound});
        m_if.insert({0x00000062, &Player_t::ForceHold});
        m_bf.insert({0x00000064, &Player_t::YoshiYellow});
        m_bf.insert({0x00000066, &Player_t::YoshiBlue});
        m_bf.insert({0x00000068, &Player_t::YoshiRed});
        m_if.insert({0x0000006a, &Player_t::YoshiWingsFrame});
        m_if.insert({0x0000006c, &Player_t::YoshiWingsFrameCount});
        m_if.insert({0x0000006e, &Player_t::YoshiTX});
        m_if.insert({0x00000070, &Player_t::YoshiTY});
        m_if.insert({0x00000072, &Player_t::YoshiTFrame});
        m_if.insert({0x00000074, &Player_t::YoshiTFrameCount});
        m_if.insert({0x00000076, &Player_t::YoshiBX});
        m_if.insert({0x00000078, &Player_t::YoshiBY});
        m_if.insert({0x0000007a, &Player_t::YoshiBFrame});
        m_if.insert({0x0000007c, &Player_t::YoshiBFrameCount});
        //Location_t YoshiTongue; (Between 0x80 and 0xB0)
        m_ff.insert({0x000000b0, &Player_t::YoshiTongueX});
        m_if.insert({0x000000b4, &Player_t::YoshiTongueLength});
        m_bf.insert({0x000000b6, &Player_t::YoshiTonugeBool});
        m_if.insert({0x000000b8, &Player_t::YoshiNPC});
        m_if.insert({0x000000ba, &Player_t::YoshiPlayer});
        m_if.insert({0x000000bc, &Player_t::Dismount});
        m_if.insert({0x000000be, &Player_t::NoPlayerCol});
        //Location_t Location; (Between 0xC0 and 0xF0)
        m_if.insert({0x000000f0, &Player_t::Character});
        //Controls_t Controls; (Between 0xF2 and 0x105)
        m_if.insert({0x00000106, &Player_t::Direction});
        m_if.insert({0x00000108, &Player_t::Mount});
        m_if.insert({0x0000010a, &Player_t::MountType});
        m_if.insert({0x0000010c, &Player_t::MountSpecial});
        m_if.insert({0x0000010e, &Player_t::MountOffsetY});
        m_if.insert({0x00000110, &Player_t::MountFrame});
        m_if.insert({0x00000112, &Player_t::State});
        m_if.insert({0x00000114, &Player_t::Frame});
        m_if.insert({0x00000118, &Player_t::FrameCount});
        m_if.insert({0x0000011c, &Player_t::Jump});
        m_bf.insert({0x0000011e, &Player_t::CanJump});
        m_bf.insert({0x00000120, &Player_t::CanAltJump});
        m_if.insert({0x00000122, &Player_t::Effect});
        m_df.insert({0x00000124, &Player_t::Effect2});
        m_bf.insert({0x0000012c, &Player_t::DuckRelease});
        m_bf.insert({0x0000012e, &Player_t::Duck});
        m_bf.insert({0x00000130, &Player_t::DropRelease});
        m_bf.insert({0x00000132, &Player_t::StandUp});
        m_bf.insert({0x00000134, &Player_t::StandUp2});
        m_bf.insert({0x00000136, &Player_t::Bumped});
        m_ff.insert({0x00000138, &Player_t::Bumped2});
        m_bf.insert({0x0000013c, &Player_t::Dead});
        m_if.insert({0x0000013e, &Player_t::TimeToLive});
        m_if.insert({0x00000140, &Player_t::Immune});
        m_bf.insert({0x00000142, &Player_t::Immune2});
        m_bf.insert({0x00000144, &Player_t::ForceHitSpot3});
        m_if.insert({0x00000146, &Player_t::Pinched1});
        m_if.insert({0x00000148, &Player_t::Pinched2});
        m_if.insert({0x0000014a, &Player_t::Pinched3});
        m_if.insert({0x0000014c, &Player_t::Pinched4});
        m_if.insert({0x0000014e, &Player_t::NPCPinched});
        m_ff.insert({0x00000150, &Player_t::m2Speed});
        m_if.insert({0x00000154, &Player_t::HoldingNPC});
        m_bf.insert({0x00000156, &Player_t::CanGrabNPCs});
        m_if.insert({0x00000158, &Player_t::HeldBonus});
        m_if.insert({0x0000015a, &Player_t::Section});
        m_if.insert({0x0000015c, &Player_t::WarpCD});
        m_if.insert({0x0000015e, &Player_t::Warp});
        m_if.insert({0x00000160, &Player_t::FireBallCD});
        m_if.insert({0x00000162, &Player_t::FireBallCD2});
        m_if.insert({0x00000164, &Player_t::TailCount});
        m_ff.insert({0x00000168, &Player_t::RunCount});
        m_bf.insert({0x0000016c, &Player_t::CanFly});
        m_bf.insert({0x0000016e, &Player_t::CanFly2});
        m_if.insert({0x00000170, &Player_t::FlyCount});
        m_bf.insert({0x00000172, &Player_t::RunRelease});
        m_bf.insert({0x00000174, &Player_t::JumpRelease});
        m_if.insert({0x00000176, &Player_t::StandingOnNPC});
        m_if.insert({0x00000178, &Player_t::StandingOnTempNPC});
        m_bf.insert({0x0000017a, &Player_t::UnStart});
        m_ff.insert({0x0000017c, &Player_t::mountBump});
        m_ff.insert({0x00000180, &Player_t::SpeedFixY});
    }

    double getAny(Player_t *obj, int address) override
    {
        if(address >= 0x80 && address < 0xB0) // YoshiTongue
            return s_locMem.getAny(&obj->YoshiTongue, address - 0x80);
        else if(address >= 0xC0 && address < 0xF0) // Location
            return s_locMem.getAny(&obj->Location, address - 0xC0);
        else if(address >= 0xF2 && address < 0x106) // Controls
            return s_conMem.getAny(&obj->Controls, address - 0xF2);
        return PlayerParent::getAny(obj, address);
    }

    void setAny(Player_t *obj, int address, double value) override
    {
        if(address >= 0x80 && address < 0xB0) // YoshiTongue
        {
            s_locMem.setAny(&obj->YoshiTongue, address - 0x80, value);
            return;
        }
        else if(address >= 0xC0 && address < 0xF0) // Location
        {
            s_locMem.setAny(&obj->Location, address - 0xC0, value);
            return;
        }
        else if(address >= 0xF2 && address < 0x106) // Controls
        {
            s_conMem.setAny(&obj->Controls, address - 0xF2, value);
            return;
        }

        PlayerParent::setAny(obj, address, value);
    }
};

static const char npc_t_name[] = "NPC_t";
typedef SMBXObjectMemoryEmulator<NPC_t, npc_t_name> NpcParent;
class NPCMemory final : public NpcParent
{
public:
    NPCMemory() : NpcParent()
    {
        buildTable();
    }

    void buildTable()
    {
        m_bf.clear();
        m_df.clear();
        m_if.clear();
        m_ff.clear();
        m_sf.clear();

        m_sf.insert({0x00000000, &NPC_t::AttLayer});
        m_if.insert({0x00000004, &NPC_t::Quicksand});
        m_if.insert({0x00000006, &NPC_t::RespawnDelay});
        m_bf.insert({0x00000008, &NPC_t::Bouce});
        m_if.insert({0x0000000a, &NPC_t::Pinched1});
        m_if.insert({0x0000000c, &NPC_t::Pinched2});
        m_if.insert({0x0000000e, &NPC_t::Pinched3});
        m_if.insert({0x00000010, &NPC_t::Pinched4});
        m_if.insert({0x00000012, &NPC_t::MovingPinched});
        m_if.insert({0x00000014, &NPC_t::NetTimeout});
        m_ff.insert({0x00000018, &NPC_t::RealSpeedX});
        m_if.insert({0x0000001c, &NPC_t::Wet});
        m_if.insert({0x0000001e, &NPC_t::Settings});
        m_bf.insert({0x00000020, &NPC_t::NoLavaSplash});
        m_if.insert({0x00000022, &NPC_t::Slope});
        m_if.insert({0x00000024, &NPC_t::Multiplier});
        m_if.insert({0x00000026, &NPC_t::TailCD});
        m_bf.insert({0x00000028, &NPC_t::Shadow});
        m_sf.insert({0x0000002c, &NPC_t::TriggerActivate});
        m_sf.insert({0x00000030, &NPC_t::TriggerDeath});
        m_sf.insert({0x00000034, &NPC_t::TriggerTalk});
        m_sf.insert({0x00000038, &NPC_t::TriggerLast});
        m_sf.insert({0x0000003c, &NPC_t::Layer});
        m_bf.insert({0x00000040, &NPC_t::Hidden});
        m_bf.insert({0x00000042, &NPC_t::Legacy});
        m_bf.insert({0x00000044, &NPC_t::Chat});
        m_bf.insert({0x00000046, &NPC_t::Inert});
        m_bf.insert({0x00000048, &NPC_t::Stuck});
        m_bf.insert({0x0000004a, &NPC_t::DefaultStuck});
        m_sf.insert({0x0000004c, &NPC_t::Text});
        m_ff.insert({0x00000050, &NPC_t::oldAddBelt});
        m_if.insert({0x00000054, &NPC_t::PinchCount});
        m_bf.insert({0x00000056, &NPC_t::Pinched});
        m_if.insert({0x00000058, &NPC_t::PinchedDirection});
        m_ff.insert({0x0000005c, &NPC_t::BeltSpeed});
        m_if.insert({0x00000060, &NPC_t::standingOnPlayer});
        m_if.insert({0x00000062, &NPC_t::standingOnPlayerY});
        m_bf.insert({0x00000064, &NPC_t::Generator});
        m_ff.insert({0x00000068, &NPC_t::GeneratorTimeMax});
        m_ff.insert({0x0000006c, &NPC_t::GeneratorTime});
        m_if.insert({0x00000070, &NPC_t::GeneratorDirection});
        m_if.insert({0x00000072, &NPC_t::GeneratorEffect});
        m_bf.insert({0x00000074, &NPC_t::GeneratorActive});
        m_bf.insert({0x00000076, &NPC_t::playerTemp});
        // m_bf.insert({0x00000078, &NPC_t::Location}); // between 0x78 and 0xA8
        // m_bf.insert({0x000000a8, &NPC_t::DefaultLocation}); // between 0xA8 and 0xD8
        m_ff.insert({0x000000d8, &NPC_t::DefaultDirection});
        m_if.insert({0x000000dc, &NPC_t::DefaultType});
        m_if.insert({0x000000de, &NPC_t::DefaultSpecial});
        m_if.insert({0x000000e0, &NPC_t::DefaultSpecial2});
        m_if.insert({0x000000e2, &NPC_t::Type});
        m_if.insert({0x000000e4, &NPC_t::Frame});
        m_ff.insert({0x000000e8, &NPC_t::FrameCount});
        m_ff.insert({0x000000ec, &NPC_t::Direction});
        m_df.insert({0x000000f0, &NPC_t::Special});
        m_df.insert({0x000000f8, &NPC_t::Special2});
        m_df.insert({0x00000100, &NPC_t::Special3});
        m_df.insert({0x00000108, &NPC_t::Special4});
        m_df.insert({0x00000110, &NPC_t::Special5});
        m_df.insert({0x00000118, &NPC_t::Special6});
        m_bf.insert({0x00000120, &NPC_t::TurnAround});
        m_if.insert({0x00000122, &NPC_t::Killed});
        m_bf.insert({0x00000124, &NPC_t::Active});
        // m_bf.insert({0x00000126, &NPC_t::Reset});
        m_if.insert({0x0000012a, &NPC_t::TimeLeft});
        m_if.insert({0x0000012c, &NPC_t::HoldingPlayer});
        m_if.insert({0x0000012e, &NPC_t::CantHurt});
        m_if.insert({0x00000130, &NPC_t::CantHurtPlayer});
        m_if.insert({0x00000132, &NPC_t::BattleOwner});
        m_if.insert({0x00000134, &NPC_t::WallDeath});
        m_bf.insert({0x00000136, &NPC_t::Projectile});
        m_if.insert({0x00000138, &NPC_t::Effect});
        m_df.insert({0x0000013c, &NPC_t::Effect2});
        m_if.insert({0x00000144, &NPC_t::Effect3});
        m_if.insert({0x00000146, &NPC_t::Section});
        m_ff.insert({0x00000148, &NPC_t::Damage});
        m_if.insert({0x0000014c, &NPC_t::JustActivated});
        m_if.insert({0x0000014e, &NPC_t::Block});
        m_if.insert({0x00000150, &NPC_t::tempBlock});
        m_bf.insert({0x00000152, &NPC_t::onWall});
        m_bf.insert({0x00000154, &NPC_t::TurnBackWipe});
        m_if.insert({0x00000156, &NPC_t::Immune});
    }

    double getAny(NPC_t *obj, int address) override
    {
        if(address >= 0x78 && address < 0xA8) // YoshiTongue
            return s_locMem.getAny(&obj->Location, address - 0x78);
        else if(address >= 0xA8 && address < 0xD8) // Location
            return s_locMem.getAny(&obj->DefaultLocation, address - 0xA8);
        else if(address == 0x126)
            return obj->Reset[1] ? 0xFFFF : 0;
        else if(address == 0x128)
            return obj->Reset[2] ? 0xFFFF : 0;

        return NpcParent::getAny(obj, address);
    }

    void setAny(NPC_t *obj, int address, double value) override
    {
        if(address >= 0x78 && address < 0xA8) // YoshiTongue
        {
            s_locMem.setAny(&obj->Location, address - 0x78, value);
            return;
        }
        else if(address >= 0xA8 && address < 0xD8) // Location
        {
            s_locMem.setAny(&obj->DefaultLocation, address - 0xA8, value);
            return;
        }
        else if(address == 0x126)
        {
            obj->Reset[1] = value != 0;
        }
        else if(address == 0x128)
        {
            obj->Reset[2] = value != 0;
        }

        NpcParent::setAny(obj, address, value);
    }
};

static SMBXMemoryEmulator   s_emu;
static PlayerMemory         s_emuPlayer;
static NPCMemory            s_emuNPC;


void MemAssign(int address, double value, OPTYPE operation, FIELDTYPE ftype)
{
    if(ftype == FT_INVALID)
        return;

    if(operation == OP_Div && value == 0)
        return;

    switch(operation)
    {
    case OP_Assign:
    {
        switch(ftype)
        {
        case FT_BYTE:
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            s_emu.setAny(address, value);
            break;
        default:
            break;
        }
    }//OP Assign
    break;

    case OP_Add:
    {
        switch(ftype)
        {
        case FT_BYTE:
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            s_emu.setAny(address, s_emu.getAny(address) + value);
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
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            s_emu.setAny(address, s_emu.getAny(address) - value);
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
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            s_emu.setAny(address, s_emu.getAny(address) * value);
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
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            s_emu.setAny(address, s_emu.getAny(address) / value);
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
        case FT_WORD:
        case FT_DWORD:
            s_emu.setAny(address, (double)((int64_t)s_emu.getAny(address) ^ (int64_t)value));
            break;
        default:
            break;
        }
    }//OP XOR
    break;

    default:
        break;
    }// switch on op
}

bool CheckMem(int address, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
    UNUSED(ftype);
    switch(ctype)
    {
    case CMPT_EQUALS:
        return fEqual(s_emu.getAny(address), value);
    case CMPT_GREATER:
        return s_emu.getAny(address) > value;
    case CMPT_LESS:
        return s_emu.getAny(address) < value;
    case CMPT_NOTEQ:
        return !fEqual(s_emu.getAny(address), value);
    }

    return false;
}

double GetMem(int addr, FIELDTYPE ftype)
{
    UNUSED(ftype);
    return s_emu.getAny(addr);
}




template<class T, class U>
static void MemAssignType(T &mem, U *obj, int address, double value, OPTYPE operation, FIELDTYPE ftype)
{
    if(ftype == FT_INVALID)
        return;

    if(operation == OP_Div && value == 0)
        return;

    switch(operation)
    {
    case OP_Assign:
    {
        switch(ftype)
        {
        case FT_BYTE:
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            mem.setAny(obj, address, value);
            break;
        default:
            break;
        }
    }//OP Assign
    break;

    case OP_Add:
    {
        switch(ftype)
        {
        case FT_BYTE:
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            mem.setAny(obj, address, mem.getAny(obj, address) + value);
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
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            mem.setAny(obj, address, mem.getAny(obj, address) - value);
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
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            mem.setAny(obj, address, mem.getAny(obj, address) * value);
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
        case FT_WORD:
        case FT_DWORD:
        case FT_FLOAT:
        case FT_DFLOAT:
            mem.setAny(obj, address, mem.getAny(obj, address) / value);
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
        case FT_WORD:
        case FT_DWORD:
            mem.setAny(obj, address, (double)((int64_t)mem.getAny(obj, address) ^ (int64_t)value));
            break;
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
static bool ChecmMemType(T &mem, U *obj, int offset, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
    UNUSED(ftype);
    switch(ctype)
    {
    case CMPT_EQUALS:
        return fEqual(mem.getAny(obj, offset), value);
    case CMPT_GREATER:
        return mem.getAny(obj, offset) > value;
    case CMPT_LESS:
        return mem.getAny(obj, offset) < value;
    case CMPT_NOTEQ:
        return !fEqual(mem.getAny(obj, offset), value);
    }
    return false;
}


void MemAssign(Player_t *obj, int address, double value, OPTYPE operation, FIELDTYPE ftype)
{
    MemAssignType(s_emuPlayer, obj, address, value, operation, ftype);
}

bool CheckMem(Player_t *obj, int offset, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
    return ChecmMemType(s_emuPlayer, obj, offset, value, ctype, ftype);
}

double GetMem(Player_t *obj, int offset, FIELDTYPE ftype)
{
    UNUSED(ftype);
    return s_emuPlayer.getAny(obj, offset);
}


void MemAssign(NPC_t *obj, int address, double value, OPTYPE operation, FIELDTYPE ftype)
{
    MemAssignType(s_emuNPC, obj, address, value, operation, ftype);
}

bool CheckMem(NPC_t *obj, int offset, double value, COMPARETYPE ctype, FIELDTYPE ftype)
{
    return ChecmMemType(s_emuNPC, obj, offset, value, ctype, ftype);
}

double GetMem(NPC_t *obj, int offset, FIELDTYPE ftype)
{
    UNUSED(ftype);
    return s_emuNPC.getAny(obj, offset);
}

