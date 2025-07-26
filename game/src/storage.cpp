#include "storage.hpp"

#include "log.hpp"
#include "path.hpp"
#include "sdl_call.hpp"

std::string IOMode2Mode(IOMode mode, bool binary, bool advance) {
    std::string result;
    switch (mode) {
        case IOMode::Read:
            result = "r";
            break;
        case IOMode::Write:
            result = "w";
            break;
        case IOMode::Append:
            result = "a";
            break;
    }

    if (binary) {
        result += 'b';
    }

    if (advance) {
        result += '+';
    }

    return result;
}

IOStream::IOStream(const Path& filename, IOMode mode, bool binary,
                   bool advance_mode) {
    m_stream = SDL_IOFromFile(filename.string().c_str(),
                              IOMode2Mode(mode, binary, advance_mode).c_str());
    if (!m_stream) {
        LOGE("open file from {} failed: {}", filename, SDL_GetError());
    }
}

size_t IOStream::GetSize() const {
    Sint64 size = SDL_GetIOSize(m_stream);
    if (size < 0) {
        LOGE("SDL_GetIOSize failed: {}", SDL_GetError());
        return 0;
    }
    return size;
}

std::vector<char> IOStream::Read() const {
    std::vector<char> result(GetSize());
    size_t size = 0;
    size_t read_size;
    while ((read_size = SDL_ReadIO(m_stream, result.data() + size, result.size())) > 0) {
        size += read_size; 
    }

    if (read_size < 0) {
        LOGE("SDL_ReadIO failed: {}", SDL_GetError());
        return {};
    }
    return result;
}

void IOStream::Write(const char* data, size_t size) {
    size_t write_size = SDL_WriteIO(m_stream, data, size);
    if (write_size < size) {
        LOGE("SDL_WriteIO failed: {}", SDL_GetError());
    }
}

IOStream::operator bool() const noexcept {
    return m_stream;
}

IOStream::~IOStream() {
    SDL_CloseIO(m_stream);
}

std::shared_ptr<IOStream> IOStream::CreateFromFile(const Path& filename,
                                                   IOMode mode, bool binary,
                                                   bool advance_mode) {
    return std::shared_ptr<IOStream>(
        new IOStream{filename, mode, binary, advance_mode});
}