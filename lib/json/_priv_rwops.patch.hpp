static constexpr size_t s_rwops_input_adapter_buffer_size = 2048;

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

    std::char_traits<uint8_t>::int_type get_character() noexcept
    {
        if(m_temp_buf_cur >= m_temp_buf_end)
        {
            m_temp_buf_cur = 0;
            m_temp_buf_end = SDL_RWread(m_rwops, &m_temp_buf[0], 1, sizeof(m_temp_buf));

            if(m_temp_buf_end == 0)
                return std::char_traits<uint8_t>::eof();
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
