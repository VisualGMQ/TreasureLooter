if(NOT PARTS OR NOT OUT)
  message(FATAL_ERROR "concat_defs.cmake: PARTS and OUT required")
endif()

set(COMBINED "--!strict\n")
set(COMBINED "${COMBINED}----------------- COMBINED luau-lsp definition file (GENERATED). Edit the *.d.luau fragments instead. -----------------\n")

foreach(part IN LISTS PARTS)
  get_filename_component(part_name "${part}" NAME)
  file(READ "${part}" part_content)
  set(COMBINED "${COMBINED}\n-- ================= ${part_name} =================\n${part_content}\n")
endforeach()

get_filename_component(OUT_DIR "${OUT}" DIRECTORY)
file(MAKE_DIRECTORY "${OUT_DIR}")
file(WRITE "${OUT}" "${COMBINED}")
message(STATUS "Generated combined luau definitions -> ${OUT}")
