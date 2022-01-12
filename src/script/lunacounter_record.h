#pragma once
#ifndef DEATHRECORD_H
#define DEATHRECORD_H

#include <cstdint>
#include <cstdio>
#include <string>

struct DeathRecord
{
    void Save(FILE *openfile);
    void Load(FILE *openfile);

    std::string LevelName;
    int32_t Deaths = 0;
};

#endif // DEATHRECORD_H
