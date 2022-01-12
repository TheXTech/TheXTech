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
    DeathCounter();

    void init();

    bool TryLoadStats();
    void UpdateDeaths(bool write_save);
    void AddDeath(const std::string &, int amount);
    void TrySave();
    void Draw();
    void Recount();
    void ClearRecords();

    void PrintDebug();

private:
    void InitStatsFile(FILE *openfile);
    void WriteHeader(FILE *openfile);
    void WriteRecords(FILE *statsfile);
    void ReadRecords(FILE *openfile);
    void Save(FILE *openfile);

    // Members
public:
    bool mStatFileOK;
    bool mEnabled;

    int mCurTotalDeaths;
    int mCurLevelDeaths;

    std::list<DeathRecord> mDeathRecords;

    std::string counterFile;
};

extern DeathCounter	gDeathCounter;

#endif // DEATHCOUNTER_H
