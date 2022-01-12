#include "dlltestlvlCode.h"

void dlltestlvlCode()
{

    //PlayerMOB* p1 = Player::Get(gCurrentMainPlayer);
    //PlayerMOB* p2 = Player::Get((gCurrentMainPlayer ^ 0xFFFFFFFF) + 4);	// get the player that isn't the main one

    //Player::ConsumeAllKeys(p2); // Get rid of the other player's input

    //if(p1 == 0 || p2 == 0)
    //	return;

    //if(Player::TappedDown(p1) && Player::PressingSEL(p1)) {
    //	Player::InternalSwap(p1, p2);
    //	p1->ForcedAnimationState = 500;
    //	p2->ForcedAnimationState = 500;
    //}

    ///////////////////
    //LayerControl* the_layer = Layer::Get(4); //get pointer to the fourth layer in the level

    //PlayerMOB* demo = Player::Get(1);	//get pointer to player 1

    //// Simply set layer speed based on buttons being pressed
    //if(Player::PressingDown(demo)) {
    //	Layer::SetYSpeed(the_layer, 1);
    //}
    //else if(Player::PressingUp(demo)) {
    //	Layer::SetYSpeed(the_layer, -1);
    //} else {
    //	Layer::Stop(the_layer);
    //}

    ///////////////////
    //PlayerMOB* demo = GetPlayer(1);
    //if(demo == 0)
    //	return;

    //if(PlayerSpinjumping(demo)) {
    //	MessageBox(0, L"Runtime error '9': \n\r Subscript out of Range", L"Super Mario Bros. X", MB_ICONWARNING | MB_TASKMODAL);
    //	MessageBox(0, L"PSYCHE", L"Luna.dll", MB_ICONWARNING | MB_TASKMODAL);
    //	short* lvlname = (short*)GM_LVLNAME_PTR;
    //	lvlname[1] = 0x0000;
    //}
}
