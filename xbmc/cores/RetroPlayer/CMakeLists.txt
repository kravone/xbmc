set(SOURCES PixelConverter.cpp
            RetroPlayer.cpp
            RetroPlayerAudio.cpp
            RetroPlayerVideo.cpp)

set(HEADERS PixelConverter.h
            RetroPlayer.h
            RetroPlayerAudio.h
            RetroPlayerVideo.h)

core_add_library(retroplayer)
add_dependencies(retroplayer libcpluff)

if(ENABLE_INTERNAL_FFMPEG)
  add_dependencies(retroplayer ffmpeg)
endif()
