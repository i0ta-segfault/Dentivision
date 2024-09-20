#!/bin/bash

# Compile the program
g++ main.cpp glad.c dependencies/imgui.cpp dependencies/imgui_draw.cpp dependencies/imgui_widgets.cpp dependencies/imgui_tables.cpp dependencies/backends/imgui_impl_glfw.cpp dependencies/imgui_impl_opengl3.cpp -o main \
    -Iglm \
    -Iopenal/include \
    -Idependencies\
    -Idependencies/backends \
    -Lfolder \
    -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl -lopenal

# Check if compilation was successful
if [ $? -ne 0 ]; then
    echo "Compilation failed."
    exit 1
fi

# Run the program
./main
