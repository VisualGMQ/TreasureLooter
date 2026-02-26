#include "code_generate.hpp"
#include "common.hpp"
#include "lyra/lyra.hpp"
#include "mustache.hpp"
#include "parse.hpp"
#include "rapidxml.hpp"
#include <filesystem>
#include <fstream>

int main(int argc, char** argv) {
    // cli parameter parse

    std::filesystem::path parse_dir;
    std::filesystem::path output_dir = "generate";
    std::filesystem::path src_prefix;
    auto cli = lyra::cli() | lyra::arg(parse_dir, "parse directory") |
               lyra::opt(output_dir, "output directory")["-o"]["--output-dir"] |
               lyra::opt(src_prefix, "src prefix")["--src-prefix"];

    lyra::parse_result result = cli.parse({argc, argv});
    if (!result) {
        std::cerr << "Error in command line: " << result.message() << std::endl;
        return 1;
    }

    std::filesystem::path serd_output_dir = output_dir / "serialize";
    std::filesystem::path display_output_dir = output_dir / "display";
    std::filesystem::path binding_output_dir = output_dir / "binding";

    if (!std::filesystem::exists(parse_dir)) {
        std::cerr << "invalid parse directory: " << parse_dir << std::endl;
        return 1;
    }

    // visit directory

    std::vector<std::filesystem::path> filenames;
    for (auto entry : std::filesystem::directory_iterator{parse_dir}) {
        if (!entry.is_regular_file()) {
            continue;
        }

        if (entry.path().extension() != ".xml") {
            continue;
        }

        filenames.push_back(entry.path());
    }

    // schema parse

    SchemaInfoManager manager;
    for (auto& filename : filenames) {
        auto schema_info = ParseSchema(filename);
        if (schema_info) {
            manager.m_infos.push_back(schema_info.value());
        }
    }

    if (std::filesystem::exists(output_dir)) {
        std::filesystem::remove_all(output_dir);
    }

    // code generate
    std::filesystem::create_directories(output_dir);
    std::filesystem::create_directories(serd_output_dir);
    std::filesystem::create_directories(display_output_dir);
    std::filesystem::create_directories(binding_output_dir);

    for (auto& info : manager.m_infos) {
        // generate class declare codes
        {
            std::string code = GenerateSchemaCode(info);
            std::ofstream file(output_dir / info.m_generate_filename);
            file.write(code.c_str(), code.length());
            std::cout << "generate file to : " << output_dir / info.
                m_generate_filename << std::endl;
        }

        // generate serialize codes
        {
            {
                std::string header_code =
                    GenerateSchemaSerializeHeaderCode(info);
                auto header_filename = info.m_pure_filename;
                header_filename.replace_extension(".hpp");
                std::ofstream file(serd_output_dir / header_filename);
                file.write(header_code.c_str(), header_code.length());
            }

            {
                std::string impl_code = GenerateSchemaSerializeImplCode(info);
                auto impl_filename = info.m_pure_filename;
                impl_filename.replace_extension(".cpp");
                std::ofstream file(serd_output_dir / impl_filename);
                file.write(impl_code.c_str(), impl_code.length());
            }
        }

        // generate display codes
        {
            {
                std::string code =
                    GenerateSchemaDisplayHeaderCode(info);
                auto header_filename = info.m_pure_filename;
                header_filename.replace_extension(".hpp");
                std::ofstream file(display_output_dir / header_filename);
                file.write(code.c_str(), code.length());
            }

            {
                std::string impl_code = GenerateSchemaDisplayImplCode(info);
                auto impl_filename = info.m_pure_filename;
                impl_filename.replace_extension(".cpp");
                std::ofstream file(display_output_dir / impl_filename);
                file.write(impl_code.c_str(), impl_code.length());
            }
        }

        // generate script binding codes
        {
            {
                std::string header_code =
                    GenerateSchemaScriptBindHeaderCode(info);
                auto header_filename = info.m_pure_filename;
                header_filename.replace_extension(".hpp");
                std::ofstream file(binding_output_dir / header_filename);
                file.write(header_code.c_str(), header_code.length());
                std::cout << "generate script bind header to : " << binding_output_dir / header_filename << std::endl;
            }

            {
                std::string impl_code = GenerateSchemaScriptBindImplCode(info);
                auto impl_filename = info.m_pure_filename;
                impl_filename.replace_extension(".cpp");
                std::ofstream file(binding_output_dir / impl_filename);
                file.write(impl_code.c_str(), impl_code.length());
                std::cout << "generate script bind impl to : " << binding_output_dir / impl_filename << std::endl;
            }
        }
    }

    // generate asset_info.hpp
    {
        std::string code = GenerateAssetInfoHeaderCode(manager);
        std::ofstream file(output_dir / "asset_info.hpp");
        file.write(code.c_str(), code.length());
    }

    // generate asset_info.cpp
    {
        std::string code = GenerateAssetInfoImplCode(manager);
        std::ofstream file(output_dir / "asset_info.cpp");
        file.write(code.c_str(), code.length());
    }

    // generate serialize.hpp
    {
        std::string code = GenerateAssetSerializeTotleHeaderCode(manager);
        std::ofstream file(serd_output_dir / "serialize.hpp");
        file.write(code.c_str(), code.length());
    }

    // generate display.hpp
    {
        std::string code = GenerateAssetDisplayTotleHeaderCode(manager);
        std::ofstream file(display_output_dir / "display.hpp");
        file.write(code.c_str(), code.length());
    }

    // generate binding/binding.hpp
    {
        std::string code = GenerateBindingHeaderCode(manager);
        std::ofstream file(binding_output_dir / "binding.hpp");
        file.write(code.c_str(), code.length());
        std::cout << "generate binding header to : " << binding_output_dir / "binding.hpp" << std::endl;
    }

    // generate binding/binding.cpp
    {
        std::string code = GenerateBindingImplCode(manager);
        std::ofstream file(binding_output_dir / "binding.cpp");
        file.write(code.c_str(), code.length());
        std::cout << "generate binding impl to : " << binding_output_dir / "binding.cpp" << std::endl;
    }

    // generate schema_types.luau.inc to output_dir (schema_generate/schema); merged with tl_types.luau.inc by CMake to produce tl_types.luau
    {
        std::string code = GenerateSchemaTypesLuauCode(manager);
        std::ofstream file(output_dir / "schema_types.luau.inc");
        file.write(code.c_str(), code.length());
        std::cout << "generate schema_types.luau.inc to : " << output_dir / "schema_types.luau.inc" << std::endl;
    }

    return 0;
}
