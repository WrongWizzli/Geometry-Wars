#include "Engine.h"
#include "Objects.h"
#include <stdlib.h>
#include <memory.h>

#include <stdio.h>
#include <chrono>
#include <iostream>

#include <stdint.h>

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
BackGround b("fo/square_0x5d3fd3_31.png");
Living_Objects objects;
Player p(2, 10, 500, 500, 0.0, 0.0, "fo/player2.png", "fo/square_0x5d3fd3_31.png");
int32_t FRAME_COUNTER = 0;
int64_t t0 = 0;


void get_fps_count() {
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
void initialize() {
    int width, height, bpp;
    uint8_t* rgb_image = stbi_load("image.png", &width, &height, &bpp, 3);
    Object *sq = new ChaserMob(10, 10, 100, 100, 0.7, 7, "fo/inner_4_0xa7c7e7_250.png", 255);
    Object *sq2 = new BouncerMob(10, 10, 500, 500, -1, -0.6, 100, "fo/inner_3_0xe30b5c_50.png", 255);
    objects.add(sq);
    objects.add(sq2);
}

// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt) {
    if (is_key_pressed(VK_ESCAPE))
        schedule_quit_game();
    if (is_key_pressed(VK_LEFT))
        p.set_xspeed(-1);
    if (is_key_pressed(VK_RIGHT))
        p.set_xspeed(1);
    if (is_key_pressed(VK_DOWN))
        p.set_yspeed(1);
    if (is_key_pressed(VK_UP))
        p.set_yspeed(-1);
    p.set_dir(get_cursor_x(), get_cursor_y());
    p.act();
    objects.act(p.get_xpos(), p.get_ypos());
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw() {
    memcpy(buffer, b.background, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    objects.draw();
    p.draw();
    get_fps_count();
}

// free game data in this function
void finalize() {
}

