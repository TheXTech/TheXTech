#ifndef PRIV_RVOPS_PATCH
#define PRIV_RVOPS_PATCH

#   ifndef JSON_NO_IO
#include <string>
#include <SDL2/SDL_rwops.h>

#ifdef NLOHMANN_JSON_NAMESPACE_BEGIN
#   error Include this file BEFORE the <json/json.hpp>!
#endif

#define NLOHMANN_JSON_NAMESPACE_NO_VERSION 1 // Disable versioning

namespace nlohmann
{
#if NLOHMANN_JSON_NAMESPACE_NO_VERSION
inline namespace json_abi
#else
inline namespace json_abi_v3_11_3
#endif
{
namespace detail
{


static constexpr size_t s_rwops_input_adapter_buffer_size = 2048;

namespace rwops
{
    // Primary template of char_traits calls std char_traits
    template<typename T>
    struct char_traits : std::char_traits<T>
    {};

    // Explicitly define char traits for unsigned char since it is not standard
    template<>
    struct char_traits<unsigned char> : std::char_traits<char>
    {
        using char_type = unsigned char;
        using int_type = uint64_t;

        // Redefine to_int_type function
        static int_type to_int_type(char_type c) noexcept
        {
            return static_cast<int_type>(c);
        }

        static char_type to_char_type(int_type i) noexcept
        {
            return static_cast<char_type>(i);
        }

        static constexpr int_type eof() noexcept
        {
            return static_cast<int_type>(EOF);
        }
    };
}

/*!
Input adapter for SDL_RWops file access. Uses a 2KB cache.
*/
class rwops_input_adapter
{
public:
    using char_type = uint8_t;

    explicit rwops_input_adapter(SDL_RWops* rwops) noexcept
        : m_rwops(rwops) {}

    // make class move-only
    rwops_input_adapter(const rwops_input_adapter&) = delete;
    rwops_input_adapter(rwops_input_adapter&&) noexcept = default;
    rwops_input_adapter& operator=(const rwops_input_adapter&) = delete;
    rwops_input_adapter& operator=(rwops_input_adapter&&) = delete;
    ~rwops_input_adapter() = default;

    rwops::char_traits<uint8_t>::int_type get_character() noexcept
    {
        if(m_temp_buf_cur >= m_temp_buf_end)
        {
            m_temp_buf_cur = 0;
            m_temp_buf_end = SDL_RWread(m_rwops, &m_temp_buf[0], 1, sizeof(m_temp_buf));

            if(m_temp_buf_end == 0)
                return rwops::char_traits<uint8_t>::eof();
        }

        return m_temp_buf[m_temp_buf_cur++];
    }

private:
    /// the rwops stream to read from
    SDL_RWops* m_rwops;

    uint8_t m_temp_buf[s_rwops_input_adapter_buffer_size];
    int m_temp_buf_cur = 0;
    int m_temp_buf_end = 0;
};

inline rwops_input_adapter input_adapter(SDL_RWops* rwops)
{
    return rwops_input_adapter(rwops);
}



} // namespace detail
} // inline namespace json_abi
} // namespace nlohmann

#   endif // #ifndef JSON_NO_IO
#endif // PRIV_RVOPS_PATCH
