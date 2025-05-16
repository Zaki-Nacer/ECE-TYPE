cmake_minimum_required(VERSION 3.16)
project(untitled4 C)

set(CMAKE_C_STANDARD 11)

add_executable(untitled4 main.c
        defs.h
        graphics.h
        graphics.c
        player.c
        player.h
        enemy.h
        enemy.c
        projectile.h
        projectile.c
        hud.h
        hud.c
        effect.h
        effect.c
        savegame.h
        savegame.c
)
target_link_libraries(untitled4 -lmingw32 -lalleg44)