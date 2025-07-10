file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)

add_executable(schema_parser)
target_sources(schema_parser PRIVATE ${SRC})
target_compile_features(schema_parser PRIVATE cxx_std_17)
target_link_libraries(schema_parser PRIVATE RapidXML bfg::lyra)
