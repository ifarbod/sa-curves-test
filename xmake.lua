set_languages("cxx23")

add_rules("mode.debug", "mode.release")

target("sa-curves-test")
    set_kind("shared")
    set_extension(".asi")
    add_files("src/*.cpp")

    after_link(function (target)
        os.cp(target:targetfile(), "C:/$Files/Games/GTA SA")
        os.cp(target:symbolfile(), "C:/$Files/Games/GTA SA")
    end)
