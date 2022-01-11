#include "lunalevel.h"
#include "globals.h"


void LevelF::PushSectionBoundary(int section, int which_boundary_UDLR, double push_val)
{
    Location_t &boundarray = level[section];

    switch(which_boundary_UDLR)
    {
    case 0:     // U
        boundarray.Y += push_val;
        break;

    case 1:     // D
        boundarray.Height += push_val;
        break;

    case 2:     // L
        boundarray.X += push_val;
        break;

    case 3:     // R
        boundarray.Width += push_val;
        break;
    }
}

void LevelF::SetSectionBounds(int section, double left_bound, double top_bound, double right_bound, double bot_bound)
{
    Location_t &boundarray = level[section];
    boundarray.X = left_bound;
    boundarray.Y = top_bound;
    boundarray.Height = bot_bound;
    boundarray.Width = right_bound;
}

double LevelF::GetBoundary(int section, int which_boundary_UDLR)
{
    Location_t &boundarray = level[section];

    switch(which_boundary_UDLR)
    {
    case 0:     // U
        return boundarray.Y;
        break;

    case 1:     // D
        return boundarray.Height;
        break;

    case 2:     // L
        return boundarray.X;
        break;

    case 3:     // R
        return boundarray.Width;
        break;
    }
    return 0;
}

void LevelF::GetBoundary(LunaRect *rect, int section)
{
    if(section >= 0 && section < numSections)
    {
        Location_t &boundarray = level[section];
        rect->top = (int)boundarray.Y;
        rect->bottom = (int)boundarray.Height;
        rect->left = (int)boundarray.X;
        rect->right = (int)boundarray.Width;
    }
}

std::string LevelF::GetName()
{
    return LevelName;
}
