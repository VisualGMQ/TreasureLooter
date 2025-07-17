#pragma once
#include "SDL3/SDL.h"
#include "path.hpp"

#include <filesystem>
#include <vector>

/*
 * NOTE: We don't implement by SDL_Storage due to it seems can't find correct
 * path under Android. And we won't support video game console(like PS5) ;-)
 */

enum class IOMode {
    Read,
    Write,
    Append,
};

class IOStream {
public:
    static std::shared_ptr<IOStream> CreateFromFile(
        const Path& filename, IOMode mode, bool binary = false,
        bool advance_mode = false);

    size_t GetSize() const;
    std::vector<char> Read() const;
    void Write(const char* data, size_t size);
    ~IOStream();

private:
    SDL_IOStream* m_stream{};

    IOStream() = default;
    explicit IOStream(const Path& filename, IOMode mode,
                      bool binary, bool advance_mode);
};