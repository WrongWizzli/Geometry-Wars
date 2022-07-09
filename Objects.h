#pragma once

#include "stb_image.h"
#include "Engine.h"
#include <vector>
#include <chrono>

struct Pixel {
    uint8_t b = 0;
    uint8_t g = 0;
    uint8_t r = 0;
    uint8_t a = 0;

    Pixel(){}
    Pixel(uint8_t r, uint8_t g, uint8_t b, uint8_t a): r(r), g(g), b(b), a(a) {}
    Pixel(uint32_t color);
    Pixel swap_colors();
    uint32_t pixel() const;
    uint32_t alpha_mix(Pixel color);
    bool is_color() const;
    void set_black(uint8_t alpha);
};


class Texture {
    private:
    Pixel *data = nullptr;
    Pixel *rotdata = nullptr;
    int height, width, channels;
    int h2, w2;
    double tan2 = 0.0, sin = 0.0;
    double theta = 0.0;
    double next_theta = 0.0;

    void vhflip_image();
    void _rotate_image(double angle);
    void tighten_image();
    public:

    Texture(){}
    Texture(const char *path);
    Texture(const Texture &c);
    Texture(Texture &&c);
    Texture& operator=(const Texture &c);
    Texture& operator=(Texture &&c);
    ~Texture();

    Pixel& operator[](const int i) {return data[i];}
    int get_h() const {return height;}
    int get_h2() const {return h2;}
    int get_w() const {return width;}
    int get_w2() const {return w2;}
    int get_c() const {return channels;}
    bool is_rotatable() const {return theta > 0;}
    void set_rotation_theta(double theta);
    void calc_rotation_theta(double xdir, double ydir);
    void rotate_image();
    void death_animation();
    void death_animation2(int32_t death_speed);
};


struct BackGround {
    Pixel background[SCREEN_HEIGHT][SCREEN_WIDTH];
    int bwidth, bheight;

    BackGround(const char *s);
    BackGround(const BackGround &c);
    BackGround(BackGround &&c) = delete;
};


struct Object {
    double hp = 0;
    double score = 0;
    double speed = 0;

    int xpos = 0, ypos = 0;
    Texture tex;

    virtual double get_hp() const {return hp;}
    virtual double get_score() const {return score;}
    virtual double get_speed() const {return speed;}
    virtual int get_xpos() const {return xpos;}
    virtual int get_ypos() const {return ypos;}
    virtual void act(int xppos, int yppos) = 0;
    virtual void draw() = 0;
    virtual void attack(){}
    virtual void get_damage(double damage) {hp -= damage;}
    virtual bool is_dead() const {return hp <= 0;}
    virtual ~Object(){}
};


struct ChaserMob: public Object {
    double hp;
    double score;
    double speed;

    double xresidue = 0, yresidue = 0;
    int xpos, ypos;

    Texture tex;

    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;

    ChaserMob(double hp, double score, int xpos, int ypos, double speed, int32_t upd_ms, const char *path, uint8_t alpha);

    void get_damage(double damage);
    bool is_dead() const;
    double get_hp() const;
    double get_score() const;
    double get_speed() const;
    int get_xpos() const;
    int get_ypos() const;

    void act(int xppos, int yppos);
    void draw();
};


struct BouncerMob: public Object {
    double hp;
    double score;
    double speed = 0;

    double xresidue = 0, yresidue = 0;
    int xpos, ypos;
    double xdir, ydir;
    double acc_modifier = 0.2 / RAND_MAX;

    Texture tex;

    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;

    BouncerMob(double hp, double score, int xpos, int ypos, double xdir, double ydir, int32_t upd_ms, const char *path, uint8_t alpha);

    void get_damage(double damage);
    bool is_dead() const;
    double get_hp() const;
    double get_score() const;
    double get_speed() const;
    int get_xpos() const;
    int get_ypos() const;

    void act(int xppos, int yppos);
    void draw();
};


struct AngleShooterMob: public Object {
    double hp;
    double score;
    double speed;

    double xresidue = 0, yresidue = 0;
    int xpos, ypos;

    Texture tex;
    Texture bullet;
    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;

    AngleShooterMob(double hp, double score, int xpos, int ypos, double speed, int32_t upd_ms, const char *path, const char *path_bullet, uint8_t alpha);
    // ~AngleShooterMob();

    void get_damage(double damage);
    bool is_dead() const;
    double get_hp() const;
    double get_score() const;
    double get_speed() const;
    int get_xpos() const;
    int get_ypos() const;

    // void act(int xppos, int yppos);
    // void draw();
    // void attack();
};


class Living_Objects {
    std::vector<Object*> objects;
    int num_deleted = 0;
    
    void remake_vector();
    public:
    Living_Objects(){}
    ~Living_Objects();

    void add(Object *obj);
    void act(int xppos, int yppos);
    void draw();
};


class Player {
    double speed;
    int xspeed = 0, yspeed = 0;
    double shoot_speed;
    double damage;
    int xpos, ypos;
    double xdir = 0, ydir = 1;
    Texture tex;
    Texture bullet_tex;
    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;

    public:
    Player(double speed, double damage, double xpos, double ypos, double xdir, double ydir, const char *path, const char *bpath);
    void set_damage(double damage) {this->damage = damage;}
    void set_speed(double speed) {this->speed = speed;}
    void set_xspeed(int xspeed) {this->xspeed = xspeed;}
    void set_yspeed(int yspeed) {this->yspeed = yspeed;}
    void set_dir(double xdir, double ydir) {this->xdir = xdir - xpos, this->ydir = ydir - ypos;}
    void set_bullet_tex(Texture &tex) {this->tex = tex;}
    int get_xpos() const {return xpos;}
    int get_ypos() const {return ypos;}
    int get_xdir() const {return xdir;}
    int get_ydir() const {return ydir;}


    void act();
    void draw();
};