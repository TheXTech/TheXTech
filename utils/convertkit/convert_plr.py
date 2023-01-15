import os, sys

def blit(fn, sx, sy, dx, dy, w, h, flip):
    flip = '-flop ' if flip else ''
    return f'\\( "{fn}" -crop {w}x{h}+{sx}+{sy} +repage {flip}\\) -geometry +{dx}+{dy} -composite '

def check_trim(fn, sx, sy, w, h):
    return w - int(os.popen(f'convert "{fn}" -crop {w}x{h}+{sx}+{sy} +repage -define trim:edges=east -trim -format \"%w\" info:').read())


def do_sheet(fn, frames, out, w, h, cols, rows, make_flippable=False):
    ow = 100
    oh = 100
    ocols = 10
    orows = 10

    cmd = f'convert -size {w*cols}x{h*rows*2} xc:none -compose Replace -gravity NorthWest -define trim:edges=east '

    for df, sf in enumerate(frames):
        for dir in [-1, 1]:
            o_coord = dir * sf + 49
            sx = ow * (o_coord // orows)
            sy = oh * (o_coord % orows)

            dx = w * (df // rows)
            dy = h * (df % rows)

            flip = False

            dw = w

            if dir == -1:
                dx = (cols - 1) * w - dx
                dy += rows * h

                if make_flippable:
                    flip = (sf == 0)

                    if not flip:
                        trim = check_trim(fn, sx, sy, w, h)
                        print(f'lo-{sf} = {trim}')
                        dx += trim
                        dw -= trim

                    if sf == 25 or sf == 26:
                        flip = True
                elif sf == 0:
                    continue

            cmd += blit(fn, sx, sy, dx, dy, dw, h, flip)

    cmd += '"' + out + '"'

    cmd.replace('\'', '\\\'')

    print(cmd)

    return cmd

def do_sheet_nonlink(fn, outfn):
    return do_sheet(fn, list(range(0,33))+list(range(40,45)), outfn, 48, 64, 10, 4)

def do_sheet_link(fn, outfn):
    return do_sheet(fn, range(1,17), outfn, 64, 64, 4, 4)

if __name__ == '__main__':
    if len(sys.argv) > 1:
        fns = sys.argv[1:]
    else:
        fns = os.listdir()

    for fn in fns:
        if 'link' in fn:
            os.system(do_sheet_link(fn, fn[:-4]+'.out.png'))
        else:
            os.system(do_sheet_nonlink(fn, fn[:-4]+'.out.png'))
