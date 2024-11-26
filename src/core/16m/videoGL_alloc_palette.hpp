extern "C"
{
void removePaletteFromTexture( gl_texture_data *tex );

u8*
vramBlock_examineSpecial( s_vramBlock *mb, u8 *addr, u32 size, u8 align );

u16* vramGetBank(u16 *addr);

u32
vramBlock_allocateSpecial( s_vramBlock *mb, u8 *addr, u32 size );
};

//---------------------------------------------------------------------------------
// glColorTableEXT loads a 15-bit color
//  format palette into palette memory,
//  and sets it to the currently bound texture
//---------------------------------------------------------------------------------
void glColorTableEXT_alloc_only( int, int, u16 width, int, int ) {
//---------------------------------------------------------------------------------
    if( glGlob->activeTexture ) {
        gl_texture_data *texture = (gl_texture_data*)DynamicArrayGet( &glGlob->texturePtrs, glGlob->activeTexture );
        gl_palette_data *palette;
        if( texture->palIndex ) // Remove prior palette if exists
            removePaletteFromTexture( texture );

        // Exit if no color table or color count is 0 (helpful in emptying the palette for the active texture)
        if( !width )
            return;

        // Allocate new palette block based on the texture's format
        u32 colFormat = (( texture->texFormat >> 26 ) & 0x7 );

        u32 colFormatVal = (( colFormat == GL_RGB4 || ( colFormat == GL_NOTEXTURE && width <= 4 )) ? 3 : 4 );
        u8* checkAddr = vramBlock_examineSpecial( glGlob->vramBlocks[ 1 ], (u8*)VRAM_E, width << 1, colFormatVal );

        if( checkAddr ) {
            // Calculate the address, logical and actual, of where the palette will go
            u16* baseBank = vramGetBank( (u16*)checkAddr );
            u32 addr = ( (u32)checkAddr - (u32)baseBank );
            u8 offset = 0;

            if( baseBank == VRAM_F )
                offset = ( VRAM_F_CR >> 3 ) & 3;
            else if( baseBank == VRAM_G )
                offset = ( VRAM_G_CR >> 3 ) & 3;
            addr += (( offset & 0x1 ) * 0x4000 ) + (( offset & 0x2 ) * 0x8000 );

            addr >>= colFormatVal;
            if( colFormatVal == 3 && addr >= 0x2000 ) {
                // palette location not good because 4 color mode cannot extend past 64K texture palette space
                GFX_PAL_FORMAT = glGlob->activePalette = 0;
                return;
            }

            palette = (gl_palette_data*)malloc( sizeof( gl_palette_data ));
            palette->palIndex = vramBlock_allocateSpecial( glGlob->vramBlocks[ 1 ], checkAddr, width << 1 );
            palette->vramAddr = checkAddr;
            palette->addr = addr;

            palette->connectCount = 1;
            palette->palSize = width << 1;

            // copy straight to VRAM, and assign a palette name
            // u32 tempVRAM = VRAM_EFG_CR;
            // u16 *startBank = vramGetBank( (u16*)palette->vramAddr );
            // u16 *endBank = vramGetBank( (u16*)((char*)palette->vramAddr + ( width << 1 ) - 1));
            // do {
            //     if( startBank == VRAM_E ) {
            //         vramSetBankE( VRAM_E_LCD );
            //         startBank += 0x8000;
            //     } else if( startBank == VRAM_F ) {
            //         vramSetBankF( VRAM_F_LCD );
            //         startBank += 0x2000;
            //     } else if( startBank == VRAM_G ) {
            //         vramSetBankG( VRAM_G_LCD );
            //         startBank += 0x2000;
            //     }
            // } while ( startBank <= endBank );

            // swiCopy( table, palette->vramAddr, width | COPY_MODE_HWORD );
            // vramRestoreBanks_EFG( tempVRAM );

            if( glGlob->deallocPalSize )
                texture->palIndex = (u32)DynamicArrayGet( &glGlob->deallocPal, glGlob->deallocPalSize-- );
            else
                texture->palIndex = glGlob->palCount++;
            DynamicArraySet( &glGlob->palettePtrs, texture->palIndex, (void*)palette );

            GFX_PAL_FORMAT = palette->addr;
            glGlob->activePalette = texture->palIndex;
        } else
            GFX_PAL_FORMAT = glGlob->activePalette = texture->palIndex;
    }
}
