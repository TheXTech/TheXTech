/*
 * Moondust, a free game engine for platform game making
 * Copyright (c) 2014-2022 Vitaly Novichkov <admin@wohlnet.ru>
 *
 * This software is licensed under a dual license system (MIT or GPL version 3 or later).
 * This means you are free to choose with which of both licenses (MIT or GPL version 3 or later)
 * you want to use this software.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You can see text of MIT license in the LICENSE.mit file you can see in Engine folder,
 * or see https://mit-license.org/.
 *
 * You can see text of GPLv3 license in the LICENSE.gpl3 file you can see in Engine folder,
 * or see <http://www.gnu.org/licenses/>.
 */

#ifndef PGE_FADER_H
#define PGE_FADER_H

class PGE_Fader
{
public:
    PGE_Fader();
    PGE_Fader(const PGE_Fader &f);
    ~PGE_Fader();

    PGE_Fader &operator=(const PGE_Fader &f);

    void setFade(int speed, double target, double step);
    void setRatio(double ratio);
    bool tickFader(double ticks);
    const double &fadeRatio();
    bool isFading();
    bool isFull();
    void setFull();
    bool isNull();
    void setNull();

    int ticksLeft();
private:
    void fadeStep();

    double cur_ratio = 0.0;
    double target_ratio = 0.0;
    double fade_step = 0.0;
    int fadeSpeed = 0;
    double manual_ticks = 0.0;
};

#endif // PGE_FADER_H
