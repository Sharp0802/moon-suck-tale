cmake_minimum_required(VERSION 3.26)
project(sprog C)

set(CMAKE_C_STANDARD 99)

add_executable(sprog
        src/audio.c
        src/error.c
        src/events.c
        src/fs.c
        src/iid.c
        src/main.c
        src/updator.c
        src/bitmap.c
        src/egdi/rdrctx.c
        src/egdi/init.c
)
target_include_directories(sprog PRIVATE inc)
target_link_libraries(sprog PRIVATE gdiplus)
