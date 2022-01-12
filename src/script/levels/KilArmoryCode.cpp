#include "KilArmoryCode.h"
#include "globals.h"
#include "../lunaplayer.h"
#include "../lunainput.h"
#include "../lunarender.h"
#include "../renderop_effect.h"
#include "../autocode_manager.h"

void KilArmoryCode()
{
    Player_t *demo = PlayerF::Get(1);

    if(demo)
    {
        // Section 20(19) glow effect code
        if(gFrames > 60 && demo->Section == 19)
        {
            int intensity = (int)(sin((float)(gFrames) / 22) * 35) + 60;
            intensity <<= 16;
            RenderEffectOp *op = new RenderEffectOp(RNDEFF_ScreenGlow, BLEND_Additive, intensity, 100);
            op->m_FramesLeft = 1;
            Renderer::Get().AddOp(op);
        }

        // Section 1(0) glow effect code
        if(gFrames > 60 && demo->Section == 0 && gAutoMan.GetVar("GOTSANGRE") == 0)
        {
            int intensity = (int)(sin((float)(gFrames) / 10) * 45) + 48;
            intensity <<= 16;
            RenderEffectOp *op = new RenderEffectOp(RNDEFF_ScreenGlow, BLEND_Additive, intensity, 100);
            op->m_FramesLeft = 1;
            Renderer::Get().AddOp(op);
        }
    }
}
