set_languages("cxx23")

add_rules("mode.debug", "mode.release")

target("sa-curves-test")
    set_kind("shared")
    set_extension(".asi")
    add_files("src/main.cpp")
