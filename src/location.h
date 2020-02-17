#ifndef LOCATION_H
#define LOCATION_H

//Public Type Location    'Holds location information for objects
struct Location_t
{
//    X As Double
    double X = 0.0;
//    Y As Double
    double Y = 0.0;
//    Height As Double
    double Height = 0.0;
//    Width As Double
    double Width = 0.0;
//    SpeedX As Double
    double SpeedX = 0.0;
//    SpeedY As Double
    double SpeedY = 0.0;
    //! Initial direction [New-added]
    int Direction = 1;
//End Type
};

#endif // LOCATION_H
