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

BackGround background("fo/square_0x5d3fd3_31.png");
DeathBackGround deathbackground;
Living_Objects objects;
Player player(4, 100, 10000, 500, 500, 0.0, 0.0, "textures/player.png", "textures/monster_shot.png");
Texture pbullet("textures/monster_shot.png");
Score score_counter(9);
MobCreator mob_creator(0.5, 0.2, 0.5, 10000, 1000);


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
void initialize() {}


// this function is called to update game data,
// dt - time elapsed since the previous update (in seconds)
void act(float dt) {
    if (is_key_pressed(VK_ESCAPE))
        schedule_quit_game();
    if (is_key_pressed(VK_LEFT))
        player.set_xspeed(-1);
    if (is_key_pressed(VK_RIGHT))
        player.set_xspeed(1);
    if (is_key_pressed(VK_DOWN))
        player.set_yspeed(1);
    if (is_key_pressed(VK_UP))
        player.set_yspeed(-1);
    player.set_dir(get_cursor_x(), get_cursor_y());
    if (is_mouse_button_pressed(0) && player.can_shoot()) {
        Object *bullet = new PlayerBullet(2.0, 15, player.get_xdir(), player.get_ydir(),  player.get_xpos(), player.get_ypos(), pbullet, 10);
        objects.add_pbullet(bullet);
    }
    player.act();
    int32_t nlive = objects.act(player.get_xpos(), player.get_ypos());
    objects.give_buffs(player);
    Object *new_mob = mob_creator.act(nlive);
    if (new_mob != nullptr) {
        objects.add(new_mob);
    }
}


// fill buffer in this function
// uint32_t buffer[SCREEN_HEIGHT][SCREEN_WIDTH] - is an array of 32-bit colors (8 bits per R, G, B)
void draw() {
    if (player.is_dead()) {
        memcpy(buffer, deathbackground.background, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
    } else {
        memset(mob_map, 0, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(int32_t));
        memcpy(buffer, background.background, SCREEN_HEIGHT * SCREEN_WIDTH * sizeof(uint32_t));
        score_counter.draw();
        int32_t kill_score = objects.draw();
        score_counter.add_score(kill_score);
        player.draw();
        player.draw_stats();
    }
    //get_fps_count();
}


// free game data in this function
void finalize() {
}

