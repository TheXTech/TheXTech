===============================================================
It's FMT library to easier format your stuff!
https://github.com/fmtlib/fmt
===============================================================

Current version (which is here at now) is 4.1.0!

===============================================================
What changed from orig version:
===============================================================
- every file begins with "fmt_" and ".cc" has been replaced with ".cpp"
  to avoid any possible conflicts with some project sources
- added `fmt_qformat.h` to support Qt-like formatting ("%1, %2, %3....")

===============================================================
How to easily upgrade this with newer version
===============================================================
1) Download a tarball or repo from official download
2) copy content of "fmt" folder into "orig" you see here
3) run from bash the "update.sh"

Note 1: You must have `sed` as GNU-sed. If you are running BSD or macOS, possibly,
        scrilt will fail to update filename references in each file you see here.

Note 2: If any files are added, please modify `update.sh` to refer them too!

