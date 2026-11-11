cmake_minimum_required(VERSION 3.16)

if (DEFINED ESP_PLATFORM)

    # idf component
    idf_component_register(
        SRC_DIRS src/Platforms src/Driver/ac101 src/Driver/cs42448 src/Driver/es7243 src/Driver/es8156 src/Driver/es8374 src/Driver/pcm3168 src/Driver/wm8960 src/Driver/wm8994 src/Driver/cs43l22 src/Driver/es7210 src/Driver/es7243e src/Driver/es8311 src/Driver/es8388 src/Driver/tas5805m src/Driver/wm8978
        INCLUDE_DIRS src 
        REQUIRES hal driver
    )
    # activate ESP32 I2C
    add_compile_definitions(ESP32 ESP32_CMAKE)
    add_compile_options(-Wno-unused-variable)

else()

    # set the project name
    project(audio_driver)

    # lots of warnings and all warnings as errors
    ## add_compile_options(-Wall -Wextra )
    set(CMAKE_CXX_STANDARD 17)

    file(GLOB_RECURSE SRC_LIST_C CONFIGURE_DEPENDS  "${PROJECT_SOURCE_DIR}/src/*.c" )
    file(GLOB_RECURSE SRC_LIST_CPP CONFIGURE_DEPENDS  "${PROJECT_SOURCE_DIR}/src/*.cpp" )

    # define libraries
    add_library (audio_driver ${SRC_LIST_C} ${SRC_LIST_CPP})

    # prevent compile errors
    target_compile_options(audio_driver PRIVATE -DUSE_DEFAULT_STDLIB)

    # define location for header files
    target_include_directories(audio_driver PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src )

endif()