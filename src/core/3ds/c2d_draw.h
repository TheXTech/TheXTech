const C2D_ImageTint shadowTint =
{C2D_Color32(0,0,0,255), 1.,
C2D_Color32(0,0,0,255), 1.,
C2D_Color32(0,0,0,255), 1.,
C2D_Color32(0,0,0,255), 1.};

inline bool C2D_DrawImage_Custom(C2D_Image img,
    float x, float y, uint16_t w, uint16_t h,
    float src_x, float src_y, float src_w, float src_h,
    unsigned int flip,
    float r, float g, float b, float a)
{
    const Tex3DS_SubTexture* old_subtex = img.subtex;
    // assuming not rotated (it isn't in the textures generated for TheXTech for 3DS)
    float scale_x = (old_subtex->right - old_subtex->left)/old_subtex->width;
    float scale_y = (old_subtex->bottom - old_subtex->top)/old_subtex->height;
    const Tex3DS_SubTexture new_subtex = {
        w,
        h,
        old_subtex->left + src_x*scale_x,
        old_subtex->top + src_y*scale_y,
        old_subtex->left + (src_x + src_w)*scale_x,
        old_subtex->top + (src_y + src_h)*scale_y,
    };
    img.subtex = &new_subtex;

    if(flip & X_FLIP_HORIZONTAL)
        scale_x = -1.f;
    else
        scale_x = 1.f;
    if(flip & X_FLIP_VERTICAL)
        scale_y = -1.f;
    else
        scale_y = 1.f;

    bool result;
    if(r != 1.f || g != 1.f || b != 1.f || a != 1.f)
    {
        uint32_t color = C2D_Color32f(r, g, b, a);
        const C2D_ImageTint tint = {color, 1., color, 1., color, 1., color, 1.};
        result = C2D_DrawImageAt(img, x, y, 0.f, &tint, scale_x, scale_y);
    }
    else
        result = C2D_DrawImageAt(img, x, y, 0.f, nullptr, scale_x, scale_y);
    img.subtex = old_subtex;
    return result;
}

inline bool C2D_DrawImage_Custom_Rotated(C2D_Image img,
    float x, float y, float w, float h,
    float src_x, float src_y, float src_w, float src_h,
    unsigned int flip, FPoint_t *center, float angle,
    float r, float g, float b, float a)
{
    const Tex3DS_SubTexture* old_subtex = img.subtex;
    // assuming not rotated (it isn't in the textures generated for TheXTech for 3DS)
    float scale_x = (old_subtex->right - old_subtex->left)/old_subtex->width;
    float scale_y = (old_subtex->bottom - old_subtex->top)/old_subtex->height;
    const Tex3DS_SubTexture new_subtex = {
        (uint16_t)w,
        (uint16_t)h,
        old_subtex->left + src_x*scale_x,
        old_subtex->top + src_y*scale_y,
        old_subtex->left + (src_x + src_w)*scale_x,
        old_subtex->top + (src_y + src_h)*scale_y,
    };
    img.subtex = &new_subtex;

    if(flip & X_FLIP_HORIZONTAL)
        scale_x = -1.f;
    else
        scale_x = 1.f;
    if(flip & X_FLIP_VERTICAL)
        scale_y = -1.f;
    else
        scale_y = 1.f;

    float cx, cy;
    if (!center)
    {
        cx = 0.f;
        cy = 0.f;
    }
    else
    {
        cx = center->x;
        cy = center->y;
    }

    const C2D_DrawParams params = {{x, y, w*scale_x, h*scale_y}, {cx, cy}, 0.f, angle * M_PI / 180};
    bool result;
    if(r != 1.f || g != 1.f || b != 1.f || a != 1.f)
    {
        uint32_t color = C2D_Color32f(r, g, b, a);
        const C2D_ImageTint tint = {color, 1., color, 1., color, 1., color, 1.};
        result = C2D_DrawImage(img, &params, &tint);
    }
    else
        result = C2D_DrawImage(img, &params);
    img.subtex = old_subtex;
    return result;
}
