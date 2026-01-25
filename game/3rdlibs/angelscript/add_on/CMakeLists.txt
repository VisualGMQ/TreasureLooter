# add_subdirectory(autowrapper)
macro(add_addon name)
    file(GLOB_RECURSE ${name}_SRC ${name}/*.h ${name}/*.cpp)
    add_library(angelscript-addon-${name} STATIC)
    target_sources(angelscript-addon-${name} PRIVATE ${${name}_SRC})
    target_link_libraries(angelscript-addon-${name} PRIVATE angelscript)
    target_include_directories(angelscript-addon-${name} PUBLIC .)
    add_library(angelscript::${name} ALIAS angelscript-addon-${name})
endmacro()

add_addon(weakref)
add_addon(serializer)
add_addon(scriptsocket)
add_addon(scriptmath)
add_addon(scripthelper)
add_addon(scripthandle)
add_addon(scriptgrid)
add_addon(scriptfile)
add_addon(scriptdictionary)
add_addon(scriptbuilder)
add_addon(scriptarray)
add_addon(scriptstdstring)
add_addon(scriptany)
add_addon(contextmgr)
add_addon(datetime)
add_addon(debugger)

add_library(angel_script_auto_wrapper INTERFACE)
target_include_directories(angel_script_auto_wrapper INTERFACE .)
add_library(angelscript::auto_wrapper ALIAS angel_script_auto_wrapper)

add_library(angel_script_all_addon INTERFACE)
target_link_libraries(angel_script_all_addon INTERFACE
    angelscript::auto_wrapper
    angelscript::contextmgr
    angelscript::datetime
    angelscript::debugger
    angelscript::scriptany
    angelscript::scriptarray
    angelscript::scriptbuilder
    angelscript::scriptdictionary
    angelscript::scriptfile
    angelscript::scriptgrid
    angelscript::scripthandle
    angelscript::scripthelper
    angelscript::scriptmath
    angelscript::scriptsocket
    angelscript::scriptstdstring
    angelscript::serializer
    angelscript::weakref
)

add_library(angelscript::all_addon ALIAS angel_script_all_addon)
