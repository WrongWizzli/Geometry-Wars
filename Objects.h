#pragma once

#include "stb_image.h"
#include "Engine.h"
#include <vector>
#include <cmath>
#include <chrono>
#include <random>

#define MOBS_CODE 0xf00000
#define HP_BUFF_CODE 0xc000000
#define DAMAGE_BUFF_CODE 0x3000000

extern int32_t mob_map[SCREEN_HEIGHT][SCREEN_WIDTH];
extern std::uniform_real_distribution<double> udist;
extern std::mt19937 gen;


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
    bool rotatable = false;

    void vhflip_image();
    void _rotate_image(double angle);
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
    bool is_rotatable() const {return rotatable;}
    void add_rotation_theta(double angle);
    void set_rotation_theta(double theta);
    void calc_rotation_theta(double xdir, double ydir);
    void rotate_image();
    void death_animation();
    void death_animation2(int32_t death_speed);
    void tighten_image();
};


struct BackGround {
    Pixel background[SCREEN_HEIGHT][SCREEN_WIDTH];
    int bwidth, bheight;

    BackGround(const char *s);
    BackGround(const BackGround &c);
    BackGround(BackGround &&c) = delete;
};


struct DeathBackGround {
    Pixel background[SCREEN_HEIGHT][SCREEN_WIDTH];
    const char *spath = "textures/pepechill.png";
    const char *spath2 = "textures/lose2.png";
    DeathBackGround();
};


class Score {
    Texture zero = Texture("textures/0.png");
    Texture one = Texture("textures/1.png");
    Texture two = Texture("textures/2.png");
    Texture three = Texture("textures/3.png");
    Texture four = Texture("textures/4.png");
    Texture five = Texture("textures/5.png");
    Texture six = Texture("textures/6.png");
    Texture seven = Texture("textures/7.png");
    Texture eight = Texture("textures/8.png");
    Texture nine = Texture("textures/9.png");
    int32_t score = 0;
    int32_t score_len;
    public:
    Score(int32_t score_len);
    void add_score(int32_t score) {this->score += score;}
    void draw();
};


struct Object {
    double hp = 0;
    int32_t score = 0;
    double speed = 0;
    double damage = 0;

    int xpos = 0, ypos = 0;
    Texture tex;

    virtual double get_hp() const {return hp;}
    virtual int32_t get_score() const {return score;}
    virtual double get_speed() const {return speed;}
    virtual double get_damage() const {return damage;}
    virtual int get_xpos() const {return xpos;}
    virtual int get_ypos() const {return ypos;}
    virtual void act(int xppos, int yppos){}
    virtual int32_t draw(int32_t mob_idx){return 0;}
    virtual Object* attack(int xppos, int yppos) {return nullptr;}
    virtual void deal_damage(double damage) {hp -= damage;}
    virtual bool is_dead() const {return hp <= 0;}
    virtual ~Object(){}
};


struct ChaserMob: public Object {
    double hp;
    int32_t score;
    double speed;

    double xresidue = 0, yresidue = 0;
    int xpos, ypos;

    Texture tex;

    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;
    bool isnew = true;

    ChaserMob(double hp, int32_t score, int xpos, int ypos, double speed, int32_t upd_ms, Texture &tex, uint8_t alpha);

    void deal_damage(double damage);
    double get_damage() const {return damage;}
    bool is_dead() const;
    double get_hp() const;
    int32_t get_score() const;
    double get_speed() const;
    int get_xpos() const;
    int get_ypos() const;
    void check_new();
    void act_new();
    void act_main(int xppos, int yppos);
    void act(int xppos, int yppos);
    int32_t draw(int32_t mob_idx);
};


struct BouncerMob: public Object {
    double hp;
    int32_t score;
    double speed = 0;

    double xresidue = 0, yresidue = 0;
    int xpos, ypos;
    double xdir, ydir;
    double acc_modifier = 0.2 / RAND_MAX;

    Texture tex;

    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;
    bool isnew = true;

    BouncerMob(double hp, int32_t score, int xpos, int ypos, double xdir, double ydir, int32_t upd_ms, Texture &tex, uint8_t alpha);

    void deal_damage(double damage);
    double get_damage() const {return damage;}
    bool is_dead() const;
    double get_hp() const;
    int32_t get_score() const;
    double get_speed() const;
    int get_xpos() const;
    int get_ypos() const;

    void check_new();
    void act_new();
    void act_main(int xppos, int yppos);
    void act(int xppos, int yppos);
    int32_t draw(int32_t mob_idx);
};


struct AngleShooterMob: public Object {
    double hp = 0;
    int32_t score;
    double speed;
    double bspeed;

    double xresidue = 0, yresidue = 0;
    int xpos, ypos;

    Texture tex;
    Texture bullet;
    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t bullet_timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq, bullet_ms;
    bool isnew = true, ready_to_shoot = false;

    AngleShooterMob(double hp, int32_t score, int xpos, int ypos, double speed, double bspeed, int32_t upd_ms, int32_t bullet_ms, Texture &tex, Texture &btex, uint8_t alpha);

    void deal_damage(double damage);
    double get_damage() const {return damage;}
    bool is_dead() const;
    double get_hp() const;
    int32_t get_score() const;
    double get_speed() const;
    int get_xpos() const;
    int get_ypos() const;

    void check_new();
    void act_new();
    void act_main(int xppos, int yppos);
    void act(int xppos, int yppos);
    int32_t draw(int32_t mob_idx);
    Object* attack(int xppos, int yppos);
};


class PlayerBullet: public Object {
    double speed;
    double xdir, ydir, sdir;
    double xresidue = 0, yresidue = 0;
    double damage;
    int32_t xpos, ypos;
    Texture tex;
    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;
    double hp = 0.0001;

    public:
    PlayerBullet(double damage, double speed, double xdir, double ydir, int32_t xpos, int32_t ypos, Texture &tex, int32_t upd_freq);
    void act(int xppos, int yppos);
    int32_t draw(int32_t mob_idx);
    double get_damage() const {return damage;}
    void deal_damage(double damage) {hp -= damage;}
    bool is_dead() const {return hp <= 0;}
};


class Player {
    int32_t hp = 1;
    double speed;
    int xspeed = 0, yspeed = 0;
    double shoot_speed_ms, last_shot_time = 0;
    int32_t damage;
    int xpos, ypos;
    double xdir = 0, ydir = 1;
    Texture tex;
    Texture bullet_tex;
    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;
    std::vector<Texture> nums {
    Texture("textures/0.png"),
    Texture("textures/1.png"),
    Texture("textures/2.png"),
    Texture("textures/3.png"),
    Texture("textures/4.png"),
    Texture("textures/5.png"),
    Texture("textures/6.png"),
    Texture("textures/7.png"),
    Texture("textures/8.png"),
    Texture("textures/9.png")
    };
    Texture texhp = Texture("textures/health.png");
    Texture texdmg = Texture("textures/speed.png");

    public:
    Player(double speed, double shoot_speed_ms, double damage, double xpos, double ypos, double xdir, double ydir, const char *path, const char *bpath);
    Player(){}
    void set_damage(int32_t damage) {this->damage = damage;}
    void set_speed(double speed) {this->speed = speed;}
    void set_xspeed(int xspeed) {this->xspeed = xspeed;}
    void set_yspeed(int yspeed) {this->yspeed = yspeed;}
    void set_dir(double xdir, double ydir) {this->xdir = xdir - xpos, this->ydir = ydir - ypos;}
    void set_bullet_tex(Texture &tex) {this->tex = tex;}
    Texture& get_bullet_tex() {return bullet_tex;}
    int get_xpos() const {return xpos;}
    int get_ypos() const {return ypos;}
    int get_xdir() const {return xdir;}
    int get_ydir() const {return ydir;}
    bool is_dead() const {return hp <= 0;}
    void add_damage(int32_t damage) {this->damage = std::min(damage + this->damage, 999);}
    void add_hp(int32_t hp) {this->hp = std::min(hp + this->hp, 9);}

    void act();
    void draw();
    void draw_stats();
    bool can_shoot();
};


class Living_Objects {
    std::vector<Object*> objects;
    std::vector<Object*> pbullets;
    std::vector<Object*> buffs;
    std::vector<uint32_t> collected_buffs;
    int num_deleted = 0;
    
    void remake_vectors();
    public:
    Living_Objects(){}
    ~Living_Objects();

    void add(Object *obj);
    void add_pbullet(Object *obj);
    void give_buffs(Player &p);
    
    int32_t act(int xppos, int yppos);
    int32_t draw();
};


class Buff: public Object {
    double hp = 0.001;
    int32_t buff_type;
    

    int xpos, ypos;
    Texture tex;

    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int32_t upd_freq = 10;
    public:
    Buff(int32_t buff_type, int32_t xpos, int32_t ypos, Texture &tex): buff_type(buff_type), xpos(xpos), ypos(ypos), tex(tex) {}
    void act(int xppos, int yppos);
    int32_t draw(int32_t mob_idx);
    int32_t get_score() const {return buff_type;}
    bool is_dead() const {return hp < 0;};
};


class MobCreator {
    std::vector<Texture> bouncer_enemies {
        Texture("textures/3_green_med.png"),
        Texture("textures/3_salad_med.png"),
        Texture("textures/4_yellow_thin.png"),
        Texture("textures/inner_5_0xe97451_50.png"),
        Texture("textures/inner_6_0xfafa33_75.png")};
    std::vector<Texture> chaser_enemies {
        Texture("textures/circle_0xa36c_175_43.png"),
        Texture("textures/circle_green.png"),
        Texture("textures/circle_purple.png")
    };
    std::vector<Texture> shooters {
        Texture("textures/joe.png")
    };
    std::vector<Texture> shooter_bullets {
        Texture("textures/joebullet.png")
    };
    std::vector<Texture> buffs {
        Texture("textures/bonus.png"),
    };
    std::vector<int32_t> buff_codes {
        HP_BUFF_CODE,
        DAMAGE_BUFF_CODE
    };
    double hp_rate;
    double speed_rate;
    int64_t upd_ms;
    int64_t mob_create_ms;
    double score_rate;
    int multiplier = 0;
    double create_chance = 0.2;
    double aspect_res_x = 1. / 2. / (1. + double(SCREEN_WIDTH) / SCREEN_HEIGHT);
    double aspect_res_y = 1. / 2. / (1. + double(SCREEN_HEIGHT) / SCREEN_WIDTH);
    int64_t timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    int64_t mob_timer = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    public:
    MobCreator(double hp_rate, double speed_rate, double score_rate, int64_t upd_ms, int64_t mob_create_ms):
            hp_rate(hp_rate), speed_rate(speed_rate), score_rate(score_rate), upd_ms(upd_ms), mob_create_ms(mob_create_ms) {}
    Object* create_buff();
    Object* create_bouncer();
    Object* create_chaser();
    Object* create_shooter();
    Object* create_random_mob();
    Object* act(int32_t nmobs);
};