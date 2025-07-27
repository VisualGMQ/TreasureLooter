#include "instance_display.hpp"

#include "dialog.hpp"
#include "image.hpp"
#include "imgui.h"
#include "imgui_id_generator.hpp"
#include "math.hpp"

void InstanceDisplay(const char* name, int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragInt(name, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_S8, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_S16, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_S64, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragInt(name, (int*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S8, (void*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S16, (void*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_S64, (void*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U32, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U8, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U16, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_U64, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const int& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U32, (unsigned int*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const char& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U8, (void*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const short& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U16, (void*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, unsigned const long long& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_U64, (void*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, bool& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::Checkbox(name, &value);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const bool& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::Checkbox(name, (bool*)&value);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, float& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragFloat(name, &value, 0.1);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const float& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragFloat(name, (float*)&value, 01);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, double& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    ImGui::DragScalar(name, ImGuiDataType_Double, &value, 0.1);
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const double& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragScalar(name, ImGuiDataType_Double, (void*)&value, 0.1);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, std::string& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    char buf[1024] = {0};
    memcpy(buf, value.data(), value.size());
    ImGui::InputText(name, buf, sizeof(buf) - 1);
    value = buf;
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const std::string& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::InputText(name, (char*)value.data(), value.size());
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, std::string_view value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::InputText(name, (char*)value.data(), value.size());
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, Vec2& value) {
    ImGui::DragFloat2(name, (float*)&value, 0.1);
}

void InstanceDisplay(const char* name, const Vec2& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::DragFloat2(name, (float*)&value, 0.1);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, Region& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::Text("%s", name);
    InstanceDisplay("top left", value.m_topleft);
    InstanceDisplay("size", value.m_size);
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Region& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("top left", value.m_topleft);
    InstanceDisplay("size", value.m_size);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, Degrees& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    float degree = value.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    value = degree;
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Degrees& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    float degree = value.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    ImGui::EndDisabled();
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, Radians& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    float degree = Degrees{value}.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    value = Degrees{degree};
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Radians& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    float degree = Degrees{value}.Value();
    ImGui::DragFloat(name, &degree, 0.1);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, Handle<Image>& value) {
    std::string button_text = "no image";

    if (value) {
        button_text =  value->Filename().string();
    }

#ifdef TL_ENABLE_EDITOR
    ImGui::PushID(ImGuiIDGenerator::Gen());
    if (ImGui::Button(button_text.c_str())) {
        FileDialog dialog{FileDialog::Type::OpenFile};
        auto base_path = Context::GetInst().GetProjectPath();
        dialog.SetTitle("Select Image");
        dialog.AddFilter("Png", "png");
        dialog.AddFilter("Bitmap", "bmp");
        dialog.AddFilter("JPEG", "jpg");
        dialog.SetDefaultFolder(base_path);
        dialog.Open();

        auto& files = dialog.GetSelectedFiles();
        if (!files.empty()) {
            auto& filename = files[0];

            std::error_code err;
            auto relative_path = std::filesystem::relative(filename, base_path, err);
            std::string relative_path_str = relative_path.string();
            std::replace_if(
                relative_path_str.begin(), relative_path_str.end(),
                [](char c) { return c == '\\'; }, '/');
            relative_path = relative_path_str;
            
            if (err) {
                LOGE("Can only select file under {} dir", base_path);
            } else {
                value = Context::GetInst().m_image_manager->Load(relative_path);
            }
        }
    }
    
    if (value) {
        ImVec2 size;
        size.x = value->GetSize().x;
        size.y = value->GetSize().y;
        ImGui::Image(value->GetTexture(), size);
    }

    ImGui::PopID();
#else
    InstanceDisplay(name, value.Get());
#endif
}

void InstanceDisplay(const char* name, Image* value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);

    if (value) {
        auto filename = value->Filename().string();
        ImGui::InputText(name, (char*)filename.c_str(), filename.length());
    } else {
        char trivial_buf[1] = {0};
        ImGui::InputText(name, trivial_buf, 0);
    }
    
    ImGui::EndDisabled();
    ImGui::PopID();
    
    if (value) {
        ImVec2 size;
        size.x = value->GetSize().x;
        size.y = value->GetSize().y;
        ImGui::Image(value->GetTexture(), size);
    }
}

void InstanceDisplay(const char* name, const Image* value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::BeginDisabled(true);
    if (value) {
        auto filename = value->Filename().string();
        ImGui::InputText(name, (char*)filename.c_str(), filename.length());
    } else {
        ImGui::InputText(name, nullptr, 0);
    }
    ImGui::EndDisabled();
    
    ImGui::PopID();
    
    if (value) {
        ImVec2 size;
        size.x = value->GetSize().x;
        size.y = value->GetSize().y;
        ImGui::Image(value->GetTexture(), size);
    }
   
}

void InstanceDisplay(const char* name, Transform& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());
    
    ImGui::Text("%s", name);
    InstanceDisplay("position", value.m_position);
    InstanceDisplay("scale", value.m_scale);
    InstanceDisplay("rotation", value.m_rotation);
    
    ImGui::PopID();
}

void InstanceDisplay(const char* name, const Transform& value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    ImGui::Text("%s", name);
    InstanceDisplay("position", value.m_position);
    InstanceDisplay("scale", value.m_scale);
    InstanceDisplay("rotation", value.m_rotation);
    ImGui::EndDisabled();

    ImGui::PopID();
}

void InstanceDisplay(const char* name, TilemapHandle& value) {
    std::string button_text = "no tilemap";

    if (value) {
        button_text = value->GetFilename().string();
    }

#ifdef TL_ENABLE_EDITOR
    ImGui::PushID(ImGuiIDGenerator::Gen());
    if (ImGui::Button(button_text.c_str())) {
        FileDialog dialog{FileDialog::Type::OpenFile};
        auto base_path = Context::GetInst().GetProjectPath();
        dialog.SetTitle("Select Image");
        dialog.AddFilter("TileMap", "tmx");
        dialog.SetDefaultFolder(base_path);
        dialog.Open();

        auto& files = dialog.GetSelectedFiles();
        if (!files.empty()) {
            auto& filename = files[0];

            std::error_code err;
            auto relative_path =
                std::filesystem::relative(filename, base_path, err);
            std::string relative_path_str = relative_path.string();
            std::replace_if(
                relative_path_str.begin(), relative_path_str.end(),
                [](char c) { return c == '\\'; }, '/');
            relative_path = relative_path_str;

            if (err) {
                LOGE("Can only select file under {} dir", base_path);
            } else {
                value =
                    Context::GetInst().m_tilemap_manager->Load(relative_path);
            }
        }
    }

    ImGui::PopID();
#else
    InstanceDisplay(name, value.Get());
#endif
}

void InstanceDisplay(const char* name, const TilemapHandle& value) {
    InstanceDisplay(name, value.Get()); 
}

void InstanceDisplay(const char* name, Tilemap* value) {
    InstanceDisplay(name, static_cast<const Tilemap*>(value));
}

void InstanceDisplay(const char* name, const Tilemap* value) {
    ImGui::PushID(ImGuiIDGenerator::Gen());

    ImGui::BeginDisabled(true);
    if (value) {
        auto filename = value->GetFilename().string();
        ImGui::InputText(name, (char*)filename.c_str(), filename.length());
    } else {
        ImGui::InputText(name, nullptr, 0);
    }
    ImGui::EndDisabled();

    ImGui::PopID();
}