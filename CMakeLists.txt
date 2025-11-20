cmake_minimum_required(VERSION 3.15)

# Projektname
project(XP12AutoTOD_C)

# C++ Standard (C++17 ist modern und sicher)
set(CMAKE_CXX_STANDARD 17)

# --- Pfade definieren ---
# Der Pfad zum SDK (relativ zum Projektordner)
# Wir prüfen, ob XP_SDK_PATH schon von außen (GitHub Actions) gesetzt wurde.
# Wenn nicht, nehmen wir den lokalen Ordner (für dein Visual Studio).
if(NOT DEFINED XP_SDK_PATH)
    set(XP_SDK_PATH "${CMAKE_SOURCE_DIR}/SDK")
endif()

# --- Abhängigkeiten finden ---
# WICHTIG: Wir brauchen OpenGL für das Zeichnen der GUI
find_package(OpenGL REQUIRED)

# --- Header-Verzeichnisse (Includes) ---
# Wo sucht der Compiler nach .h Dateien?
include_directories(${XP_SDK_PATH}/CHeaders/XPLM)
include_directories(${XP_SDK_PATH}/CHeaders/Widgets)
include_directories(${CMAKE_SOURCE_DIR}/src/imgui) # Damit #include "imgui.h" gefunden wird

# --- Quelldateien (Sources) ---
# Hier müssen ALLE .cpp Dateien stehen, die kompiliert werden sollen
set(SOURCES
    # Unser Hauptcode
    src/main.cpp
    
    # Die ImGui Bibliothek
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
)

# --- Das Plugin erstellen (Shared Library) ---
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# --- Windows Spezifische Einstellungen ---
if(WIN32)
    # Präprozessor-Definitionen für X-Plane auf Windows
    add_definitions(-DIBM=1 -DXPLM200 -DXPLM210 -DXPLM300)

    # Bibliotheken verlinken (Linker)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        ${XP_SDK_PATH}/Libraries/Win/XPLM_64.lib
        ${XP_SDK_PATH}/Libraries/Win/XPWidgets_64.lib
        OpenGL::GL  # Hier verlinken wir das gefundene OpenGL
    )

    # Die Dateiendung von .dll auf .xpl ändern (Wichtig für X-Plane!)
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".xpl")

elseif(APPLE)
    add_definitions(-DAPL=1 -DXPLM200 -DXPLM210 -DXPLM300)
    
    # Auf dem Mac müssen wir System-Frameworks finden
    find_library(COCOA_LIBRARY Cocoa)
    find_library(OPENGL_LIBRARY OpenGL)
    find_library(IOKIT_LIBRARY IOKit)
    find_library(COREVIDEO_LIBRARY CoreVideo)

    # Wir linken gegen die Frameworks
    # "-F" sagt dem Linker, wo er nach X-Plane Frameworks suchen soll
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        "-F ${XP_SDK_PATH}/Libraries/Mac -framework XPLM"
        "-F ${XP_SDK_PATH}/Libraries/Mac -framework XPWidgets"
        ${COCOA_LIBRARY}
        ${OPENGL_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
    
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".xpl")
# --- Linux Einstellungen ---
elseif(UNIX)
    add_definitions(-DLIN=1 -DXPLM200 -DXPLM210 -DXPLM300)
    # Linux Linker Flags sind oft etwas komplexer, hier die Basis:
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fvisibility=hidden")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        OpenGL::GL
    )
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".xpl")
endif()