#include "dialog.hpp"
#include "sdl_call.hpp"
#include <future>

FileDialog::FileDialog(Type type)
    : m_type{type}, m_properties{SDL_CreateProperties()} {}

FileDialog::~FileDialog() {
    SDL_DestroyProperties(m_properties);
}

void FileDialog::SetTitle(const std::string& title) {
    m_title_label = title;
}

void FileDialog::SetAcceptButtonText(const std::string& text) {
    m_accept_label = text;
}

void FileDialog::SetCancelButtonText(const std::string& text) {
    m_cancel_label = text;
}

void FileDialog::AddFilter(const std::string& name,
                           const std::string& pattern) {
    m_filters.push_back({name, pattern});
}

void FileDialog::AllowMultipleSelect(bool allow) {
    m_allow_multiple_select = allow;
}

void FileDialog::SetDefaultFolder(const std::string& folder) {
    // TODO: check folder is absolute path
    m_default_folder = folder;
}

void FileDialog::Open() {
    std::future f = m_promise.get_future();

    SDL_FileDialogType dialog_type{};
    switch (m_type) {
        case Type::OpenFile:
            dialog_type = SDL_FILEDIALOG_OPENFILE;
            break;
        case Type::SaveFile:
            dialog_type = SDL_FILEDIALOG_SAVEFILE;
            break;
        case Type::OpenFolder:
            dialog_type = SDL_FILEDIALOG_OPENFOLDER;
            break;
    }

    std::vector<SDL_DialogFileFilter> filters;
    filters.reserve(m_filters.size());
    for (auto& filter : m_filters) {
        filters.push_back(
            SDL_DialogFileFilter{filter.name.c_str(), filter.pattern.c_str()});
    }

    SDL_CALL(SDL_SetStringProperty(m_properties,
                                   SDL_PROP_FILE_DIALOG_LOCATION_STRING,
                                   m_default_folder.c_str()));
    SDL_CALL(SDL_SetStringProperty(m_properties,
                                   SDL_PROP_FILE_DIALOG_TITLE_STRING,
                                   m_title_label.c_str()));
    SDL_CALL(SDL_SetStringProperty(m_properties,
                                   SDL_PROP_FILE_DIALOG_ACCEPT_STRING,
                                   m_accept_label.c_str()));
    SDL_CALL(SDL_SetStringProperty(m_properties,
                                   SDL_PROP_FILE_DIALOG_CANCEL_STRING,
                                   m_cancel_label.c_str()));
    SDL_CALL(SDL_SetBooleanProperty(m_properties,
                                    SDL_PROP_FILE_DIALOG_MANY_BOOLEAN,
                                    m_allow_multiple_select));
    SDL_CALL(SDL_SetNumberProperty(
        m_properties, SDL_PROP_FILE_DIALOG_NFILTERS_NUMBER, filters.size()));
    SDL_CALL(SDL_SetPointerProperty(
        m_properties, SDL_PROP_FILE_DIALOG_FILTERS_POINTER, filters.data()));

    SDL_ShowFileDialogWithProperties(dialog_type, &callback, this,
                                     m_properties);

    m_results = f.get();
}

const std::vector<Path>& FileDialog::GetSelectedFiles() const {
    return m_results;
}

void FileDialog::callback(void* userdata, const char* const* filelist, int) {
    auto self = static_cast<FileDialog*>(userdata);

    std::vector<Path> results;
    while (filelist && *filelist) {
        const char* file = *filelist;
        results.push_back(file);
        filelist++;
    }

    self->m_promise.set_value(std::move(results));
}
