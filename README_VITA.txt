You need to have the vitasdk installed and the VITASDK environment variable
defined. 

You need to have a static build of SDL_mixer_x already
installed and built against the audio codecs included in the vitasdk.

You need to have a static build of libFreeImage already installed and built
against the image libraries (libjpeg and libpng are the big ones, libpng
was specifically patched for Vita) installed in your vitasdk directory.

vitaGL is also required, but I believe that is now included in the vitasdk
by default.

--------------------

Configuring

Configure the project as follows:

```
mkdir vita_build
cd vita_build
cmake -DCMAKE_BUILD_TYPE=Debug -DVita=1 ../
```

Ninja is also supported and can be specified here.
By specifying Vita=1, the CMake script will ensure $VITASDK is defined
and the toolchain can be loaded. If so, the project will use the 
Vita Toolchain and you should this reflected in the configuration output.

------------

Building

```
make -j<# of cores>

# Optionally, for verbose mode
VERBOSE=1 make -j<# of cores>
```

OR

```
ninja

# Optionally, for verbose mode
ninja -v
```

If all goes well, you should end up with an ARM ELF, a Sony .self, and 
a VPK file to install on your Vita!
