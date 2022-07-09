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
Player p(4, 1000, 10, 500, 500, 0.0, 0.0, "fo/player2.png", "textures/bullet_small.png");
Texture pbullet("textures/bullet_small.png");
Score s(9);


int32_t FRAME_COUNTER = 0;
int64_t fps_timer = 0;
void get_fps_count() {
    if (fps_timer == 0) {
        fps_timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    } else {
        auto fps_timer1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (fps_timer1 - fps_timer > 1000) {
        std::cout << "fps: " << FRAME_COUNTER << std::endl;
        fps_timer = fps_timer1;
        FRAME_COUNTER = 0;
        }
        FRAME_COUNTER++;
    }
}


// initialize game data in this function
void initialize() {
    Object *dd = new BouncerMob(1, 100, 800, 200, 1, 1, 10, "textures/4_yellow_thick.png", 255);
    objects.add(dd);
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
    if (is_mouse_button_pressed(0) && p.can_shoot()) {
        Object *bullet = new PlayerBullet(2.0, 15, p.get_xdir(), p.get_ydir(),  p.get_xpos(), p.get_ypos(), pbullet, 10);
        objects.add_pbullet(bullet);
    }
    p.act();
    objects.act(p.get_xpos(), p.get_ypos());
}

// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw() {
    memset(mob_map, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(int32_t));
    memcpy(buffer, b.background, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    s.draw();
    int32_t kill_score = objects.draw();
    s.add_score(kill_score);
    p.draw();
    get_fps_count();
    if (p.is_dead()) {
        schedule_quit_game();
    }
}

// free game data in this function
void finalize() {
}

