file(GLOB_RECURSE SRC ./*.cpp ./*.hpp)
file(GLOB_RECURSE MUSTACHES ./mustaches/*.mustache)

add_executable(schema_parser)
target_sources(schema_parser PRIVATE ${SRC} ${MUSTACHES})
target_compile_features(schema_parser PRIVATE cxx_std_17)
target_link_libraries(schema_parser PRIVATE RapidXML bfg::lyra mustache)
