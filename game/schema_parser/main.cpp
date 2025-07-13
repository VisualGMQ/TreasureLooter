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
        auto schema_info = parseSchema(filename);
        if (schema_info) {
            manager.m_infos.push_back(schema_info.value());
        }
    }

    // code generate
    if (!std::filesystem::exists(output_dir)) {
        std::filesystem::create_directories(output_dir);
    }
    
    for (auto& info : manager.m_infos) {
        std::string code = generateSchemaCode(info);
        auto filename = info.m_filename.filename();
        filename.replace_extension(".hpp");
        std::ofstream file(output_dir / filename);
        file.write(code.c_str(), code.length());
    }
    
    return 0;
}