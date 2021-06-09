include(CheckCXXSourceCompiles)

add_definitions(-DTCLAP_VERSION_MAJOR=1 -DTCLAP_VERSION_MINOR=4)

check_cxx_source_compiles("#include <strstream>
int main() { std::istrstream iss; return 0; }" TCLAP_HAVE_STRSTREAM)
if(TCLAP_HAVE_STRSTREAM)
    message("-- found TCLAP_HAVE_STRSTREAM")
    add_definitions(-DTCLAP_HAVE_STRSTREAM)
endif()

#check_cxx_source_compiles("#include <sstream>
#int main() { std::istringstream iss; return 0; }" TCLAP_HAVE_SSTREAM)
#if(TCLAP_HAVE_SSTREAM)
#    message("-- found TCLAP_HAVE_SSTREAM")
#    add_definitions(-DTCLAP_HAVE_SSTREAM)
#endif()
#