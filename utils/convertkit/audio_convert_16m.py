#!/usr/bin/python3

import os
import sys
import shutil
import configparser

def convert_audio(datadir, outdir):
    music_filenames = {}
    sound_filenames = {}

    os.makedirs(os.path.join(outdir, 'temp', 'music'), exist_ok=True)
    os.makedirs(os.path.join(outdir, 'temp', 'sound'), exist_ok=True)

    # load the INI files
    for filenames_dict, ini_filename in [(music_filenames, 'music.ini'), (sound_filenames, 'sounds.ini')]:
        ini = configparser.ConfigParser(inline_comment_prefixes=';')
        ini.read(os.path.join(datadir, ini_filename))

        for name in ini.sections():
            try:
                file = ini[name]['file']
            except KeyError:
                continue

            if len(file) < 3 or file[0] != '"' or file[-1] != '"':
                continue

            file = file[1:-1]

            if ini_filename == "music.ini" and '|' in file:
                file = '|'.join(file.split('|')[:-1])

            filenames_dict[name] = file

    # temporary transformations (spc2it, ogg2wav)
    for key in list(music_filenames.keys()):
        filename = music_filenames[key]
        use_fn = os.path.join(datadir, 'music', filename)

        if os.path.exists(use_fn + '.spc'):
            filename += '.spc'
            use_fn += '.spc'

        if os.path.exists(use_fn + '.it'):
            filename += '.it'
            use_fn += '.it'

        temp_fn = os.path.join(outdir, 'temp', 'music', filename)

        if filename.endswith('.spc'):
            shutil.copy(use_fn, temp_fn)
            os.system(f'spc2it "{temp_fn}"')
            use_fn = temp_fn[:-4] + '.it'

        music_filenames[key] = use_fn

        okay = (music_filenames[key].endswith('.it')
            or music_filenames[key].endswith('.mod')
            or music_filenames[key].endswith('.s3m')
            or music_filenames[key].endswith('.xm'))

        if not okay:
            del music_filenames[key]

    for key in list(sound_filenames.keys()):
        filename = sound_filenames[key]
        use_fn = os.path.join(datadir, 'sound', filename)
        temp_fn = os.path.join(outdir, 'temp', 'sound', filename)

        if filename.endswith('.ogg'):
            os.system(f'ffmpeg -i "{use_fn}" -ac 1 -ar 24000 -c:a pcm_u8 -y "{temp_fn}.wav"')
            use_fn = temp_fn + '.wav'

        sound_filenames[key] = use_fn

        okay = (sound_filenames[key].endswith('.wav'))

        if not okay:
            del sound_filenames[key]

    # make the mmutil invocation
    mmutil_invoke = f'mmutil -d "-o{outdir}soundbank.bin" "-c{outdir}soundbank.ini"'
    for filename in set(music_filenames.values()):
        mmutil_invoke += f' "{filename}"'

    for filename in set(sound_filenames.values()):
        mmutil_invoke += f' "{filename}"'

    os.system(mmutil_invoke)

    # update the INI files
    mmutil_out = configparser.ConfigParser(inline_comment_prefixes=';')
    mmutil_out.read(os.path.join(outdir, 'soundbank.ini'))

    for filenames_dict, ini_filename, group_name, resolved_name in [(music_filenames, 'music.ini', 'modules', 'resolved-mod'), (sound_filenames, 'sounds.ini', 'samples', 'resolved-sfx')]:
        ini = configparser.ConfigParser(inline_comment_prefixes=';')
        ini.read(os.path.join(datadir, ini_filename))

        for key in filenames_dict.keys():
            ini[key][resolved_name] = mmutil_out[group_name][filenames_dict[key]]

        ini.write(open(os.path.join(outdir, ini_filename), 'w'))

    # delete temp files
    os.remove(os.path.join(outdir, 'soundbank.ini'))
    shutil.rmtree(os.path.join(outdir, 'temp'))
