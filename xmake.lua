set_project("NoHurtCam")
set_version("1.0.0")

set_languages("c99", "cxx20")

add_rules("mode.release")

add_cxflags(
    "-O2",
    "-fvisibility=hidden",
    "-ffunction-sections",
    "-fdata-sections",
    "-w"
)

add_cflags(
    "-O2",
    "-fvisibility=hidden",
    "-ffunction-sections",
    "-fdata-sections",
    "-w"
)

add_ldflags(
    "-Wl,--gc-sections,--strip-all",
    "-s",
    {force = true}
)

add_requires(
    "fmt 10.2.1",
    "preloader-android git+https://github.com/nexcaise/preloader-android.git"
)

target("NoHurtCam")
    set_kind("shared")
    add_packages("fmt", "preloader-android")
    add_files("src/main.cpp")
    add_includedirs("src")
    add_syslinks("log")
    before_build(function (target)
        local pkg = target:pkg("preloader-android")
        if not pkg then return end
        local logger = path.join(pkg:installdir(), "include/pl/Logger.h")
        local data = io.readfile(logger)
        if data then
            data = data:gsub("#include <format>", "#include <fmt/format.h>")
            data = data:gsub("std::vformat", "fmt::vformat")
            data = data:gsub("std::make_format_args", "fmt::make_format_args")
            io.writefile(logger, data)
        end
    end)