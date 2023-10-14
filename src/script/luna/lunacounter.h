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
#ifndef DEATHCOUNTER_H
#define DEATHCOUNTER_H

#include <cstdio>
#include <string>
#include <list>

#include "lunacounter_record.h"

#define DEATHCT_FNAME "demos.dmo"

struct DeathCounter
{
    DeathCounter() noexcept;
    ~DeathCounter() noexcept;

    void init();
    void quit();

    bool TryLoadStats();
    void UpdateDeaths(bool write_save);
    void AddDeath(const std::string &, int amount);
    void TrySave();
    void Draw(int screenZ);
    void Recount();
    void ClearRecords();

    void PrintDebug() const;

    struct CachedPrint
    {
        // Cache meta-data for counter
        void syncCache(int curLevel, int total);
        int counterLevel = -1;
        int counterTotal = -1;
        int counterPixLen = 0;
        int font = -1;
        std::string counterOut;

        // Cache meta-data for title
        void syncCache(const std::string &title);
        int titlePixLen = 0;
        intptr_t titlePointer = 0;
        size_t titleSize = std::string::npos;
    } m_print;

private:
    FILE* m_openFile = nullptr;

    friend struct DeathRecord;
    static void InitStatsFile(FILE *openfile);
    static void WriteHeader(FILE *openfile);
    void WriteRecords(FILE *statsfile);
    void ReadRecords(FILE *openfile);
    void Save(FILE *openfile);

    // Members
public:
    bool mStatFileOK = false;
    bool mEnabled = false;

    int mCurTotalDeaths = 0;
    int mCurLevelDeaths = 0;

    std::list<DeathRecord> mDeathRecords;

    std::string counterFile;
};

extern DeathCounter	gDeathCounter;

#endif // DEATHCOUNTER_H
