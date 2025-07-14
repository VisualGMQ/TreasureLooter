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

    // code generate
    if (!std::filesystem::exists(output_dir)) {
        std::filesystem::create_directories(output_dir);
    }

    if (!std::filesystem::exists(serd_output_dir)) {
        std::filesystem::create_directories(serd_output_dir);
    }

    for (auto& info : manager.m_infos) {
        // generate class declare codes
        {
            std::string code = GenerateSchemaCode(info);
            auto filename = info.m_pure_filename;
            filename.replace_extension(".hpp");
            std::ofstream file(output_dir / filename);
            file.write(code.c_str(), code.length());
        }

        // generate serialize codes
        {
            {
                std::string header_code = GenerateSchemaSerializeHeaderCode(info);
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
    }

    return 0;
}