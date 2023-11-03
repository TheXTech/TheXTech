#!/usr/bin/python3

# this is a simple script

import os
import sys
import math
import shutil
import configparser

from convert_plr import *

PREVIEW = False
REDO = False
datadir = sys.argv[1]
graphicsdir = os.path.join(datadir, 'graphics')
fallbackdir = os.path.join(datadir, 'graphics', 'fallback')
outdir = sys.argv[2]

if not datadir.endswith('/'): datadir += '/'
if not outdir.endswith('/'): outdir += '/'

for dirpath, _, files in os.walk(datadir, topdown=True):
    if dirpath.endswith('fallback'):
        continue

    if dirpath.endswith('fonts'):
        print('found fonts dir', dirpath)
        is_fonts_dir = True
        texture_1x = set()

        for fn in files:
            if fn.endswith('.ini'):
                rfn = os.path.join(dirpath, fn)

                font = configparser.ConfigParser(inline_comment_prefixes=';')
                font.read(rfn)

                if 'font-map' in font:
                    if 'texture-scale' in font['font-map'] and font['font-map']['texture-scale'].strip != '1':
                        if 'texture' in font['font-map']:
                            tex_string = font['font-map']['texture'].strip().strip('\"')
                            print(f"{tex_string} is 1x")
                            texture_1x.add(tex_string)

    else:
        is_fonts_dir = False

    outpath = outdir+dirpath[len(datadir):]
    os.makedirs(outpath, exist_ok=True)
    for fn in files:
        rfn = os.path.join(dirpath, fn)
        if not os.path.isfile(rfn): continue
        destfn = os.path.join(outpath, fn)
        bmpfn = os.path.join(outpath, fn[:-3]+'bmp')
        dsgfn = os.path.join(outpath, fn[:-3]+'dsg')
        if not REDO and (os.path.isfile(destfn) or os.path.isfile(destfn+'.wav') or ((fn.endswith('.gif') or fn.endswith('.png')) and os.path.isfile(dsgfn))): continue
        print(rfn)

        is_1x = is_fonts_dir and fn in texture_1x

        if is_1x:
            downscale = "-sample 100%"
        else:
            downscale = "-sample 50%"

        if not REDO and not is_fonts_dir and (os.path.isfile(destfn) or os.path.isfile(destfn+'.wav') or ((fn.endswith('.gif') or fn.endswith('.png')) and os.path.isfile(dsgfn))): continue

        is_image = fn.endswith('.png') or fn.endswith('.gif')
        if fn.startswith('link') and is_image:
            os.system(do_sheet_link(rfn, bmpfn))
            rfn = bmpfn
        elif fn[:5] in ('mario', 'luigi', 'peach', 'toad-') and is_image:
            os.system(do_sheet_nonlink(rfn, bmpfn))
            rfn = bmpfn

        if fn.endswith('.png'):
            os.system(f'convert {downscale} "{rfn}" "{bmpfn}"')
        elif fn.endswith('m.gif') and os.path.isfile(rfn[:-5]+'.gif'):
            continue
        elif fn.endswith('.gif'):
            maskfn = rfn[:-4]+'m.gif'
            ftype = fn[:fn.rfind('-')]
            altmaskfn_gif_1 = os.path.join(fallbackdir, fn[:-4]+'m.gif')
            altmaskfn_gif = os.path.join(graphicsdir, ftype, fn[:-4]+'m.gif')
            altmaskfn_png = os.path.join(graphicsdir, ftype, fn[:-4]+'.png')
            if os.path.isfile(maskfn):
                os.system(f'convert "{rfn}" "{maskfn}" -alpha Off -compose CopyOpacity -composite -channel a -negate +channel {downscale} "{bmpfn}"')
            elif os.path.isfile(altmaskfn_gif_1):
                if os.popen(f'identify -format "%[fx:w*2],%[fx:h*2]" "{rfn}"').read() == os.popen(f'identify -format "%[fx:w*2],%[fx:h*2]" "{altmaskfn_gif_1}"').read():
                    os.system(f'convert "{rfn}" "{altmaskfn_gif_1}" -alpha Off -compose CopyOpacity -composite -channel a -negate +channel {downscale} "{bmpfn}"')
                else:
                    os.system(f'convert {downscale} "{rfn}" "{bmpfn}"')
            elif os.path.isfile(altmaskfn_gif):
                if os.popen(f'identify -format "%[fx:w*2],%[fx:h*2]" "{rfn}"').read() == os.popen(f'identify -format "%[fx:w*2],%[fx:h*2]" "{altmaskfn_gif}"').read():
                    os.system(f'convert "{rfn}" "{altmaskfn_gif}" -alpha Off -compose CopyOpacity -composite -channel a -negate +channel {downscale} "{bmpfn}"')
                else:
                    os.system(f'convert {downscale} "{rfn}" "{bmpfn}"')
            elif os.path.isfile(altmaskfn_png):
                if os.popen(f'identify -format "%[fx:w*2],%[fx:h*2]" "{rfn}"').read() == os.popen(f'identify -format "%[fx:w*2],%[fx:h*2]" "{altmaskfn_png}"').read():
                    os.system(f'convert "{rfn}" "{altmaskfn_png}" -alpha On -compose CopyOpacity -composite {downscale} "{bmpfn}"')
                else:
                    os.system(f'convert {downscale} "{rfn}" "{bmpfn}"')
            else:
                os.system(f'convert {downscale} "{rfn}" "{bmpfn}"')
        elif fn.endswith('.db'):
            continue
        elif fn.endswith('.ogg') and '/sound/' in rfn:
            # os.system(f'ffmpeg -i "{rfn}" "{destfn}.wav"')
            # os.system(f'ffmpeg -i "{rfn}" -acodec pcm_u8 -ar 16000 "{destfn}.wav"')
            continue
        elif is_fonts_dir and fn.endswith('.ini'):
            shutil.copy(rfn, destfn)
            os.system(f'sed \'s/\\.png/\\.dsg/\' -i "{destfn}"')
            continue
        elif fn == 'sounds.ini':
            # shutil.copy(rfn, destfn)
            # os.system(f'sed \'s/\\.ogg"/\\.ogg.wav"/\' -i "{destfn}"')
            continue
        elif fn.endswith('.mp3'):
            # os.system(f'ffmpeg -i "{rfn}" -aq 1 "{destfn}.ogg"')
            # shutil.move(destfn+'.ogg', destfn)
            continue
        else:
            shutil.copy(rfn, destfn)
            continue
        w, h, op = os.popen(f'identify -format "%[fx:w*2],%[fx:h*2],%[opaque]" "{bmpfn}"').read().split(',')

        # downscale further if needed
        flags = 0
        i_w = int(w) / 2; i_h = int(h) / 2
        i_w = max(8, 2 ** math.ceil(math.log2(i_w)))
        i_h = max(8, 2 ** math.ceil(math.log2(i_h)))
        while (i_w * i_h) > 65536:
            print("SCALING DOWN. Consider revising the asset.")
            flags += 1
            i_w /= 2
            i_h /= 2
        i_w = int(w) >> (flags + 1)
        i_h = int(h) >> (flags + 1)

        if i_w != int(w) / 2:
            os.system(f'convert -resize {200 * i_w / int(w)}% "{bmpfn}" "{bmpfn}"')

        if op == 'true':
            flags |= 1 << 4

        open(dsgfn+'.size','w').write(f'{w:>4}\n{h:>4}\n{flags}\n')
        dsgfns = [dsgfn]
        bmpfns = [bmpfn]
        if int(h) > 2048:
            os.system(f'convert "{bmpfn}" -crop {w}x{2048 * i_h / int(h)} "{bmpfn}%d.bmp"')
            os.remove(bmpfn)
            shutil.move(bmpfn+'0.bmp', bmpfn) #???
            dsgfns.append(dsgfn+'1')
            bmpfns.append(bmpfn+'1.bmp')
            if int(h) > 4096:
                dsgfns.append(dsgfn+'2')
                bmpfns.append(bmpfn+'2.bmp')
        for dsgfn_i, bmpfn_i in zip(dsgfns, bmpfns):
            h_i = int(h)
            if h_i > 2048:
                h_i = 2048
            h = int(h) - h_i

            cont_w = max(8, 2 ** math.ceil(math.log2(i_w)))
            cont_h = max(8, 2 ** math.ceil(math.log2(h_i * i_w / int(w))))

            basefn_i = bmpfn_i.replace('.bmp', '')
            pngfn_i = basefn_i + '.png'
            os.system(f'convert "{bmpfn_i}" "{pngfn_i}"')
            os.system(f'pngquant 16 "{pngfn_i}"')
            os.system(f'grit "{basefn_i}-fs8.png" -gb -gB 4 -pe16 -ftb -aw {cont_w} -ah {cont_h} -o "{basefn_i}-fs8.png"')
            pal = open(f'{basefn_i}-fs8.pal.bin', 'rb').read()
            img = open(f'{basefn_i}-fs8.img.bin', 'rb').read()
            if len(pal) < 32:
                pal = pal + bytes([0]) * (32 - len(pal))
            open(dsgfn_i, 'wb').write(pal + img)

            os.remove(bmpfn_i)
            os.remove(pngfn_i)
            os.remove(basefn_i+'-fs8.pal.bin')
            os.remove(basefn_i+'-fs8.img.bin')
            os.remove(basefn_i+'-fs8.h')

            if not PREVIEW:
                os.remove(basefn_i+'-fs8.png')

# construct graphics load lists
for dirpath, dirs, files in os.walk(outdir, topdown=True):
    print(dirpath)

    if dirpath.endswith('graphics'):
        l = open(os.path.join(dirpath, 'graphics.list'), 'w')
        pak = open(os.path.join(dirpath, 'graphics.pak'), 'wb')

        for d in dirs:
            if d == 'touchscreen' or d == 'ui':
                continue

            for f in os.listdir(os.path.join(dirpath, d)):
                if not f.endswith('.size'):
                    continue

                abs_f = os.path.join(dirpath, d, f)

                basename = f[:f.find('.')]
                if len(basename.split('-')) != 2:
                    continue

                basename = basename.replace('-', ' ')

                if not basename[:basename.find(' ')] in ('background', 'background2', 'block', 'effect', 'level',
                        'link', 'luigi', 'mario', 'npc', 'path',
                        'peach', 'player', 'scene', 'tile', 'toad',
                        'yoshib', 'yoshit'):
                    continue

                gfx_f = abs_f[:-5]

                if os.path.isfile(gfx_f):
                    offset = pak.tell()
                    pak.write(open(gfx_f, 'rb').read())

                    # continuation textures
                    i = 1
                    while os.path.isfile(gfx_f + str(i)):
                        pak.write(open(gfx_f + str(i), 'rb').read())
                        i += 1

                    fullname = 'graphics.pak'
                else:
                    offset = -1
                    fullname = os.path.join(d, f[:-5])

                l.write(basename+'\n')
                l.write(fullname+'\n')
                if offset != -1:
                    l.write(f'pack_offset {offset}\n')
                l.write(open(abs_f, 'r').read())
                l.write('\n')

        l.close()
        continue
    elif 'graphics' in os.path.split(dirpath):
        continue

    opened = False

    for f in files:
        if not f.endswith('.size'):
            continue

        if not opened:
            l = open(os.path.join(dirpath, 'graphics.list'), 'w')
            opened = True

        abs_f = os.path.join(dirpath, f)

        basename = f[:f.find('.')]
        if len(basename.split('-')) != 2:
            continue

        basename = basename.replace('-', ' ')

        if not basename[:basename.find(' ')] in ('background', 'background2', 'block', 'effect', 'level',
                'link', 'luigi', 'mario', 'npc', 'path',
                'peach', 'player', 'scene', 'tile', 'toad',
                'yoshib', 'yoshit'):
            continue

        fullname = f[:-5]

        l.write(basename+'\n')
        l.write(fullname+'\n')
        l.write(open(abs_f, 'r').read())
        l.write('\n')


    if opened:
        l.close()
