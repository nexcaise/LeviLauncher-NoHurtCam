set_project("NoHurtCam")
set_version("1.0.0")

set_languages("c99", "cxx23")

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
    "-Wl,--gc-sections",
    "-Wl,--strip-all",
    "-s",
    {force = true}
)

add_repositories("xmake-repo https://github.com/xmake-io/xmake-repo.git")
add_repositories("levimc-repo https://github.com/LiteLDev/xmake-repo.git")

add_requires("preloader_android 0.1.13")

target("NoHurtCam")
    set_kind("shared")
    add_files("src/**.cpp")
    add_headerfiles("src/**.hpp")
    add_includedirs("src", {public = true})
    add_packages("preloader_android")
    add_links("log")