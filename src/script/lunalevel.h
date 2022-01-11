#pragma once
#ifndef LUNALEVEL_H
#define LUNALEVEL_H

#include <string>
#include "lunadefs.h"

namespace LevelF
{

double GetBoundary(int section, int which_boundary_UDLR);
void GetBoundary(LunaRect* rectangle, int section);
void PushSectionBoundary(int section, int which_boundary_UDLR, double push_val);
void SetSectionBounds(int section, double left_bound, double top_bound, double right_bound, double bot_bound);

std::string GetName();

}

#endif // LUNALEVEL_H
