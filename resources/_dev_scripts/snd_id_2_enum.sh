#!/bin/bash

ARR="SFX_Jump SFX_Stomp SFX_BlockHit SFX_BlockSmashed SFX_PlayerShrink SFX_PlayerGrow SFX_Mushroom SFX_PlayerDied SFX_ShellHit SFX_Skid SFX_DropItem SFX_GotItem SFX_Camera SFX_Coin SFX_1up SFX_Lava SFX_Warp SFX_Fireball SFX_CardRouletteClear SFX_BossBeat SFX_DungeonClear SFX_Bullet SFX_Grab SFX_Spring SFX_HammerToss SFX_Slide SFX_NewPath SFX_LevelSelect SFX_Do SFX_Pause SFX_Key SFX_PSwitch SFX_Tail SFX_Raccoon SFX_Boot SFX_Smash SFX_Twomp SFX_BirdoSpit SFX_BirdoHit SFX_CrystalBallExit SFX_BirdoBeat SFX_BigFireball SFX_Fireworks SFX_BowserKilled SFX_GameBeat SFX_Door SFX_Message SFX_Yoshi SFX_YoshiHurt SFX_YoshiTongue SFX_YoshiEgg SFX_GotStar SFX_ZeldaKill SFX_PlayerDied2 SFX_YoshiSwallow SFX_SonicRing SFX_DryBones SFX_Checkpoint SFX_DraginCoin SFX_TapeExit SFX_Blaarg SFX_WartBubbles SFX_WartKilled SFX_SMBlockHit SFX_SMKilled SFX_SMHurt SFX_SMGlass SFX_SMBossHit SFX_SMCry SFX_SMExplosion SFX_Climbing SFX_Swim SFX_Grab2 SFX_Saw SFX_Throw SFX_PlayerHit SFX_ZeldaStab SFX_ZeldaHurt SFX_ZeldaHeart SFX_ZeldaDied SFX_ZeldaRupee SFX_ZeldaFire SFX_ZeldaItem SFX_ZeldaKey SFX_ZeldaShield SFX_ZeldaDash SFX_ZeldaFairy SFX_ZeldaGrass SFX_ZeldaHit SFX_ZeldaSwordBeam SFX_Bubble SFX_PSwitchTimeout SFX_SwooperFlap SFX_Iceball SFX_Freeze SFX_Icebreak SFX_PlayerHammer SFX_SproutVine"

count=1
for q in $ARR; do
    echo "$q - $count"
    find . -type f -name "*.cpp" -exec ./snd_id_sed.sh {} $count $q \;
    count=$((count + 1))
done
