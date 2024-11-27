extern "C"
{

#ifdef __BLOCKS__

typedef struct s_vramBlock
{
    uint8_t *startAddr, *endAddr;
    struct s_SingleBlock *firstBlock;
    struct s_SingleBlock *firstEmpty;
    struct s_SingleBlock *firstAlloc;

    struct s_SingleBlock *lastExamined;
    uint8_t *lastExaminedAddr;
    uint32_t lastExaminedSize;

    DynamicArray blockPtrs;
    DynamicArray deallocBlocks;

    uint32_t blockCount;
    uint32_t deallocCount;
} s_vramBlock;

typedef struct gl_texture_data
{
    void *vramAddr;       // Address to the texture loaded into VRAM
    uint32_t texIndex;    // The index in the Memory Block
    uint32_t texIndexExt; // The secondary index in the memory block (for GL_COMPRESSED)
    int palIndex;         // The palette index
    uint32_t texFormat;   // Specifications of how the texture is displayed
    uint32_t texSize;     // The size (in blocks) of the texture
} gl_texture_data;

typedef struct gl_palette_data
{
    void *vramAddr;         // Address to the palette loaded into VRAM
    uint32_t palIndex;      // The index in the memory block
    uint16_t addr;          // The offset address for texture palettes in VRAM
    uint16_t palSize;       // The length of the palette
    uint32_t connectCount;  // The number of textures currently using this palette
} gl_palette_data;

// This struct holds hidden globals for videoGL. It is initialized by glInit().
typedef struct gl_hidden_globals
{
    // VRAM blocks management
    // ----------------------

    s_vramBlock *vramBlocksTex; // One for textures
    s_vramBlock *vramBlocksPal; // One for palettes
    int vramLockTex; // Holds the current lock state of the VRAM banks
    int vramLockPal; // Holds the current lock state of the VRAM banks

    // Texture/palette manamenent
    // --------------------------

    // Arrays of texture and palettes. The index to access a texture is the same
    // as the name of that texture. The value of each array component is a
    // pointer to a texture or palette struct. When a texture/palette is
    // generated, the pointer is allocated. When it is freed, the pointer is set
    // deallocated and set to NULL, and the texture name (the array index) is
    // added to the deallocTex or deallocPal array to be reused when required.
    //
    // Note: Getting the activeTexture or activePalette from the arrays will
    // always succeed. glBindTexure() can only set activeTexture and
    // activePalette to an element that exists.
    DynamicArray texturePtrs;
    DynamicArray palettePtrs;

    // Array of names that have been deleted and are ready to be reused. They
    // are just a list of indices to the arrays texturePtrs and palettePtrs that
    // we can reuse.
    DynamicArray deallocTex;
    DynamicArray deallocPal;

    // Number of names available in the list of reusable names
    uint32_t deallocTexSize;
    uint32_t deallocPalSize;

    // Current number of allocated names. It's also the next name that will be
    // used (if there are no reusable names).
    int texCount;
    int palCount;

    // State not related to dynamic memory management
    // ----------------------------------------------

    int activeTexture; // The current active texture name
    int activePalette; // The current active palette name
    u32 clearColor; // Holds the current state of the clear color register
    GL_MATRIX_MODE_ENUM matrixMode; // Holds the current Matrix Mode

    uint8_t isActive; // Has glInit() been called before?
}
gl_hidden_globals;

// This is the actual data of the globals for videoGL.
extern gl_hidden_globals glGlob;

void removePaletteFromTexture( gl_texture_data *tex );

u8*
vramBlock_examineSpecial( s_vramBlock *mb, u8 *addr, u32 size, u8 align );

u16* vramGetBank(u16 *addr);

u32
vramBlock_allocateSpecial( s_vramBlock *mb, u8 *addr, u32 size );
};

// Load a 15-bit color format palette into palette memory, and set it to the
// currently bound texture.
int glColorTableEXT_alloc_only(int target, int empty1, uint16_t width, int empty2, int empty3)
{
    (void)target;
    (void)empty1;
    (void)empty2;
    (void)empty3;

    // We can only load a palette if there is an active texture
    if (!glGlob.activeTexture)
        return 0;

    gl_texture_data *texture = (gl_texture_data*)DynamicArrayGet(&glGlob.texturePtrs, glGlob.activeTexture);

    if (texture->palIndex) // Remove prior palette if exists
        removePaletteFromTexture(texture);

    // Exit if no color table or color count is 0 (helpful in emptying the
    // palette for the active texture). This isn't considered an error.
    if ((width == 0) )
        return 1;

    // Allocate new palette block based on the texture's format
    uint32_t colFormat = (texture->texFormat >> 26) & 0x7;

    uint32_t colFormatVal =
        ((colFormat == GL_RGB4 || (colFormat == GL_NOTEXTURE && width <= 4)) ? 3 : 4);
    uint8_t *checkAddr = vramBlock_examineSpecial(glGlob.vramBlocksPal,
        (uint8_t *)VRAM_E, width << 1, colFormatVal);

    if (checkAddr == NULL)
    {
        // Failed to find enough space for the palette
        sassert(texture->palIndex == 0, "glColorTableEXT didn't clear palette");
        GFX_PAL_FORMAT = glGlob.activePalette = 0;
        return 0;
    }

    // Calculate the address, logical and actual, of where the palette will go
    uint16_t *baseBank = vramGetBank((uint16_t *)checkAddr);
    uint32_t addr = ((uint32_t)checkAddr - (uint32_t)baseBank);
    uint8_t offset = 0;

    if (baseBank == VRAM_F)
        offset = (VRAM_F_CR >> 3) & 3;
    else if (baseBank == VRAM_G)
        offset = (VRAM_G_CR >> 3) & 3;
    addr += ((offset & 0x1) * 0x4000) + ((offset & 0x2) * 0x8000);

    addr >>= colFormatVal;
    if (colFormatVal == 3 && addr >= 0x2000)
    {
        // Palette location not good because 4 color mode cannot extend
        // past 64K texture palette space
        GFX_PAL_FORMAT = glGlob.activePalette = 0;
        return 0;
    }

    gl_palette_data *palette = (gl_palette_data *)malloc(sizeof(gl_palette_data));
    if (palette == NULL)
        return 0;

    // Get a new palette name (either reused or new)
    if (glGlob.deallocPalSize)
    {
        uint32_t palIndex = (uint32_t)DynamicArrayGet(&glGlob.deallocPal,
                                                      glGlob.deallocPalSize - 1);

        if (!DynamicArraySet(&glGlob.palettePtrs, palIndex, palette))
        {
            free(palette);
            return 0;
        }

        texture->palIndex = palIndex;
        glGlob.deallocPalSize--;
    }
    else
    {
        uint32_t palIndex = glGlob.palCount;

        if (!DynamicArraySet(&glGlob.palettePtrs, palIndex, palette))
        {
            free(palette);
            return 0;
        }

        texture->palIndex = palIndex;
        glGlob.palCount++;
    }

    // Lock the free space we have found
    palette->palIndex = vramBlock_allocateSpecial(glGlob.vramBlocksPal, checkAddr, width << 1);
    sassert(palette->palIndex != 0, "Failed to lock free palette VRAM");

    palette->vramAddr = checkAddr;
    palette->addr = addr;

    palette->connectCount = 1;
    palette->palSize = width << 1;

    // Copy straight to VRAM, and assign a palette name
    // uint32_t tempVRAM = VRAM_EFG_CR;
    // uint16_t *startBank = vramGetBank((uint16_t *)palette->vramAddr);
    // uint16_t *endBank = vramGetBank((uint16_t *)((char *)palette->vramAddr + (width << 1) - 1));
    // do
    // {
    //     if (startBank == VRAM_E)
    //     {
    //         vramSetBankE(VRAM_E_LCD);
    //         startBank += 0x8000;
    //     }
    //     else if (startBank == VRAM_F)
    //     {
    //         vramSetBankF(VRAM_F_LCD);
    //         startBank += 0x2000;
    //     }
    //     else if (startBank == VRAM_G)
    //     {
    //         vramSetBankG(VRAM_G_LCD);
    //         startBank += 0x2000;
    //     }
    // } while (startBank <= endBank);

    // swiCopy(table, palette->vramAddr, width | COPY_MODE_HWORD);
    // vramRestoreBanks_EFG(tempVRAM);

    GFX_PAL_FORMAT = palette->addr;
    glGlob.activePalette = texture->palIndex;

    return 1;
}

#else // #ifdef __BLOCKSDS__

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

#endif
