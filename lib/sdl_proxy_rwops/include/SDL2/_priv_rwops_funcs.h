#ifdef RWOPS_C_DEFINITIONS
/* non-inline definitions of normal RWops functions for C */
#    define RWOPS_PREFIX
#    define RWOPS_BODY(x) x
#elif defined(__cplusplus)
// inline definitions of normal RWops functions for C++
#    define RWOPS_PREFIX inline
#    define RWOPS_BODY(x) x
#else
/* non-inline declarations of normal RWops functions for C */
#    define RWOPS_PREFIX
#    define RWOPS_BODY(x) ;
#endif

RWOPS_PREFIX int64_t SDL_RWsize(SDL_RWops* stream)
RWOPS_BODY({
    return stream->size(stream);
})

RWOPS_PREFIX int64_t SDL_RWseek(SDL_RWops* stream, int64_t offset, int whence)
RWOPS_BODY({
    return stream->seek(stream, offset, whence);
})

RWOPS_PREFIX size_t SDL_RWread(SDL_RWops* stream, void* ptr, size_t size, size_t nmemb)
RWOPS_BODY({
    return stream->read(stream, ptr, size, nmemb);
})

RWOPS_PREFIX size_t SDL_RWwrite(SDL_RWops* stream, const void* ptr, size_t size, size_t nmemb)
RWOPS_BODY({
    return stream->write(stream, ptr, size, nmemb);
})

RWOPS_PREFIX int SDL_RWclose(SDL_RWops* stream)
RWOPS_BODY({
    if(!stream)
        return 0;

    int ret = stream->close(stream);
    SDL_FreeRW(stream);
    return ret;
})

RWOPS_PREFIX long SDL_RWtell(SDL_RWops* stream)
RWOPS_BODY({
    return stream->seek(stream, 0, RW_SEEK_CUR);
})

#undef RWOPS_PREFIX
#undef RWOPS_BODY
