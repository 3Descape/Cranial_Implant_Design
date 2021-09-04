#pragma once

#include "imgui/imgui.h"
#include <GLFW/glfw3.h>

int viewlayer_init();
void viewlayer_destroy();
void viewlayer_start();
void viewlayer_end();
void viewlayer_configure();

void viewlayer_draw_vdb();
void viewlayer_draw_cleanup();
void viewlayer_draw_objects();
void viewlayer_draw_align();