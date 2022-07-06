#include "Engine.h"
#include <stdlib.h>
#include <memory.h>

#include <stdio.h>
#include <chrono>
#include <iostream>

//
//  You are free to modify this file
//

//  is_key_pressed(int button_vk_code) - check if a key is pressed,
//                                       use keycodes (VK_SPACE, VK_RIGHT, VK_LEFT, VK_UP, VK_DOWN, VK_RETURN)
//
//  get_cursor_x(), get_cursor_y() - get mouse cursor position
//  is_mouse_button_pressed(int button) - check if mouse button is pressed (0 - left button, 1 - right button)
//  schedule_quit_game() - quit game after act()

// debug FPS counter
int32_t FRAME_COUNTER = 0;
int64_t t0 = 0;


void get_fps_count() 
{
  if (t0 == 0) {
    t0 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
  } else {
    auto t1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (t1 - t0 > 1000) {
      std::cout << "fps: " << FRAME_COUNTER << std::endl;
      t0 = t1;
      FRAME_COUNTER = 0;
    }
    FRAME_COUNTER++;
  }
}


// initialize game data in this function
int basic_pos_x = 40;
int basic_pos_y = 40;
void initialize()
{
  for (int i = basic_pos_y; i < basic_pos_y + 40; ++i) {
    for (int j = basic_pos_x; j < basic_pos_x + 40; ++j) {
      buffer[i][j] = 0xffff;
    }
  }
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt)
{
  if (is_key_pressed(VK_ESCAPE))
    schedule_quit_game();
  if (is_key_pressed(VK_LEFT))
    basic_pos_x = std::max(basic_pos_x - 2, 0);
  if (is_key_pressed(VK_RIGHT))
    basic_pos_x = std::min(basic_pos_x + 2, SCREEN_WIDTH - 40);
  if (is_key_pressed(VK_DOWN))
    basic_pos_y = std::min(basic_pos_y + 2, SCREEN_HEIGHT - 40);
  if (is_key_pressed(VK_UP))
    basic_pos_y = std::max(basic_pos_y - 2, 0);
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw()
{
  // clear backbuffer
  memset(buffer, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
  get_fps_count();
  for (int i = basic_pos_y; i < basic_pos_y + 40; ++i) {
    for (int j = basic_pos_x; j < basic_pos_x + 40; ++j) {
      buffer[i][j] = 0xffff00;
    }
  }
}

// free game data in this function
void finalize()
{
}

