#pragma once

#include <memory>
#include <string>
#include <vector>

#include "path.hpp"

#include "SDL3/SDL.h"
#include <future>
#include <memory>
#include <vector>


struct Filter {
    std::string name;
    std::string pattern;
};

class FileDialog {
public:
    enum class Type { OpenFile, SaveFile, OpenFolder };

    explicit FileDialog(Type type);
    ~FileDialog();
    void SetTitle(const std::string&);
    void SetAcceptButtonText(const std::string&);
    void SetCancelButtonText(const std::string&);
    void AddFilter(const std::string& name, const std::string& pattern);
    void AddFilter(const Filter&);
    void AllowMultipleSelect(bool);
    void SetDefaultFolder(const Path&);
    void Open();

    const std::vector<Path>& GetSelectedFiles() const;

private:
    Type m_type;
    std::vector<Filter> m_filters;
    Path m_default_folder;
    bool m_allow_multiple_select = false;
    std::string m_title_label;
    std::string m_accept_label;
    std::string m_cancel_label;
    SDL_PropertiesID m_properties;

    std::promise<std::vector<Path>> m_promise;
    std::vector<Path> m_results;

    static void callback(void* userdata, const char* const* filelist,
                         int filter);
};
