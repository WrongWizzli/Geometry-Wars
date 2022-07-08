#pragma once

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Engine.h"
#include <iostream>
#include <vector>
#include <chrono>

struct Pixel {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;

    Pixel(){}
    Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a): r(r), g(g), b(b), a(a) {}
    Pixel(uint32_t color) {
        r = color >> 16;
        g = color >> 8;
        b = color;
        a = color >> 24;
    }
    Pixel swap_colors() {
        return Pixel(b, g, r, a);
    }
    uint32_t pixel() const {
        return (uint32_t(a) << 24) + (uint32_t(r) << 16) + (uint32_t(g) << 8) + b;
    }
    uint32_t alpha_mix(Pixel color) {
        uint32_t new_r, new_g, new_b;
        new_r = uint32_t(r) * a / 255 + uint32_t(color.r) * (255 - a) / 255;
        new_g = uint32_t(g) * a / 255 + uint32_t(color.g) * (255 - a) / 255;
        new_b = uint32_t(b) * a / 255 + uint32_t(color.b) * (255 - a) / 255;
        return 0xff000000 + (new_r << 16) + (new_g << 8) + new_b;
    }
    bool full_black() const {
        return uint32_t(r) + g + b;
    }
    void set_black(uint8_t alpha) {
        if (!(this->full_black())) {
            a = 0;
        } else {
            a = alpha;
        }
    }
};


struct BackGround {
    Pixel background[SCREEN_HEIGHT][SCREEN_WIDTH];
    int bwidth, bheight;

    BackGround(const char *s) {
        int width, height, channels;
        Pixel *data = (Pixel*)stbi_load(s, &width, &height, &channels, sizeof(Pixel));
        for (int i = 0; i < width * height; ++i) {
            data[i] = data[i].swap_colors();
            data[i].set_black(0xff);
        }
        bheight = SCREEN_HEIGHT, bwidth = SCREEN_WIDTH;
        int idx_i = 0, idx_j = 0;
        for (int i = 0; i < bheight; ++i) {
            idx_j = 0;
            for (int j = 0; j < bwidth; ++j) {
                background[i][j] = data[idx_i * width + idx_j];
                idx_j = (idx_j + 1) % width;
            }
            idx_i = (idx_i + 1) % height;
        }
        stbi_image_free(data);
    }

    BackGround(const BackGround &c) {
        bwidth = c.bwidth, bheight = c.bheight;
        for (int i = 0; i < bheight; ++i) {
            for (int j = 0; j < bwidth; ++j) {
                background[i][j] = c.background[i][j];
            }
        }
    }
};


struct Object {
    double hp = 0;
    double score = 0;
    double speed = 0;
    int xpos = 0, ypos = 0;
    int height = 0, width = 0, channels = 0;

    virtual void get_damage(double damage) {hp -= damage;}
    virtual bool is_dead() const {return hp <= 0;}
    virtual double get_hp() const {return hp;}
    virtual double get_score() const {return score;}
    virtual double get_speed() const {return speed;}
    virtual int get_xpos() const {return xpos;}
    virtual int get_ypos() const {return ypos;}
    virtual int get_h() const {return height;}
    virtual int get_w() const {return width;}
    virtual int get_c() const {return channels;}
    virtual void act(int xppos, int yppos) = 0;
    virtual void draw() = 0;
    virtual ~Object(){}
};


struct SquareMob: public Object {
    double hp = 3298;
    double score;
    double speed;
    double xpath = 0, ypath = 0;
    int xpos, ypos;
    int height, width, channels;
    int h2, w2;
    Pixel *data = nullptr;
    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_period_ms = 10;

    SquareMob(double hp, double score, int xpos, int ypos, double speed, int32_t upd_ms, const char *path, uint8_t alpha): 
              hp(hp), score(score), xpos(xpos), ypos(ypos), speed(speed), upd_period_ms(upd_ms) {
        data = (Pixel*)stbi_load(path, &width, &height, &channels, sizeof(Pixel));
        for (int i = 0; i < width * height; ++i) {
            data[i] = data[i].swap_colors();
            data[i].set_black(alpha);
        }
        h2 = height / 2, w2 = width / 2;
    }

    void get_damage(double damage) {hp -= damage;}
    bool is_dead() const {return hp <= 0;}
    double get_hp() const {return hp;}
    double get_score() const {return score;}
    double get_speed() const {return speed;}
    int get_xpos() const {return xpos;}
    int get_ypos() const {return ypos;}

    void act(int xppos, int yppos) {
        int64_t cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (cur_time - timer > 4) {
            xpath += speed, ypath += speed;
            int32_t xp1 = int32_t(xpath), yp1 = int32_t(ypath);
            xpath -= xp1, ypath -= yp1;
            if (xppos <= xpos - xp1) {
                xpos -= xp1;
            } else if (xppos >= xpos + xp1) {
                xpos += xp1;
            }
            if (yppos <= ypos - yp1) {
                ypos -= yp1;
            } else if (yppos >= ypos + yp1) {
                ypos += yp1;
            }
            xpos = std::min(std::max(xpos, w2), SCREEN_WIDTH - 1 - w2);
            ypos = std::min(std::max(ypos, h2), SCREEN_HEIGHT - 1 - h2);
            timer = cur_time;
        }
    }
    void draw() {
        int di = 0, dj = 0;
        for (int i = ypos - h2; i < ypos + h2; ++i, ++di) {
            dj = 0;
            for (int j = xpos - w2; j < xpos + w2; ++j, ++dj) {
                buffer[i][j] = data[di * width + dj].alpha_mix(buffer[i][j]);
            }
        }
    }
    ~SquareMob() {
        stbi_image_free(data);
    }
};


class Living_Objects {
    std::vector<Object*> objects;
    int num_deleted = 0;
    
    void remake_vector() {
        std::vector<Object*> tmp;
        for (int i = 0; i < objects.size(); ++i) {
            if (objects[i] != nullptr) {
                tmp.push_back(objects[i]);
            }
        }
        objects = tmp;
    }

    public:
    Living_Objects(){}

    void add(Object *obj) {
        objects.push_back(obj);
    }

    void act(int xppos, int yppos) {
        for (int i = 0; i < objects.size(); ++i) {
            if (objects[i] != nullptr) {
                objects[i]->act(xppos, yppos);
            }
        }
    }

    void draw() {
        if (num_deleted > 30 || num_deleted == objects.size()) {
            remake_vector();
        }
        for (int i = 0; i < objects.size(); ++i) {
            if (objects[i] == nullptr) {
                continue;
            }
            if (objects[i]->is_dead()) {
                delete objects[i];
                objects[i] = nullptr;
                num_deleted++;
            } else {
                objects[i]->draw();
            }
        }
    }

    ~Living_Objects() {
        for (int i = 0; i < objects.size(); ++i) {
            if (objects[i] != nullptr)
                delete objects[i];
        }
    }
};


BackGround b("fo/square_0x5d3fd3_31.png");
Living_Objects objects;