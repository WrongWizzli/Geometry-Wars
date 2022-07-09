#define STB_IMAGE_IMPLEMENTATION
#include "Objects.h"
#include "Engine.h"
#include <iostream>
#include <cmath>

#define MOBS_CODE 0xf00000
int32_t mob_map[SCREEN_HEIGHT][SCREEN_WIDTH] = {0};

// Pixel 
Pixel::Pixel(uint32_t color) {
    r = color >> 16;
    g = color >> 8;
    b = color;
    a = color >> 24;
}


Pixel Pixel::swap_colors() {
    return Pixel(b, g, r, a);
}


uint32_t Pixel::pixel() const {
    return (uint32_t(a) << 24) + (uint32_t(r) << 16) + (uint32_t(g) << 8) + b;
}


uint32_t Pixel::alpha_mix(Pixel color) {
    uint32_t new_r, new_g, new_b;
    new_r = uint32_t(r) * a / 255 + uint32_t(color.r) * (255 - a) / 255;
    new_g = uint32_t(g) * a / 255 + uint32_t(color.g) * (255 - a) / 255;
    new_b = uint32_t(b) * a / 255 + uint32_t(color.b) * (255 - a) / 255;
    return 0xff000000 + (new_r << 16) + (new_g << 8) + new_b;
}


bool Pixel::is_color() const {
    return uint32_t(r) + g + b;
}


void Pixel::set_black(uint8_t alpha) {
    if (!(this->is_color())) {
        a = 0;
    } else {
        a = alpha;
    }
}


// Texture
Texture::Texture(const char *path) {
    data = (Pixel*)stbi_load(path, &width, &height, &channels, sizeof(Pixel));
    rotdata = new Pixel[width * height];
    h2 = height / 2;
    w2 = width / 2;
    for (int i = 0; i < width * height; ++i) {
        data[i] = data[i].swap_colors();
        data[i].set_black(0xff);
        rotdata[i] = data[i];
    }
}


Texture::Texture(const Texture &c) {
    if (c.data != nullptr) {
        height = c.height;
        width = c.width;
        channels = c.channels;
        h2 = c.h2;
        w2 = c.w2;
        data = new Pixel[height * width];
        rotdata = new Pixel[height * width];
        for (int i = 0; i < height * width; ++i) {
            data[i] = c.data[i];
            rotdata[i] = c.rotdata[i];
        }
    }
}


Texture::Texture(Texture &&c) {
    if (c.data != nullptr) {
        data = c.data;
        rotdata = c.rotdata;
        c.rotdata = nullptr;
        c.data = nullptr;
        height = c.height;
        width = c.width;
        channels = c.channels;
        h2 = c.h2;
        w2 = c.w2;
    }
}


Texture& Texture::operator=(const Texture &c) {
    if (&c != this) {
        height = c.height;
        width = c.width;
        h2 = c.h2;
        w2 = c.w2;
        channels = c.channels;
        data = new Pixel[height * width];
        rotdata = new Pixel[height * width];
        for (int i = 0; i < height * width; ++i) {
            data[i] = c.data[i];
            rotdata[i] = c.rotdata[i];
        }
    }
    return *this;
}


Texture& Texture::operator=(Texture &&c) {
    data = c.data;
    rotdata = c.rotdata;
    c.data = nullptr;
    c.rotdata = nullptr;
    height = c.height;
    width = c.width;
    h2 = c.h2;
    w2 = c.w2;
    channels = c.channels;
    return *this;
}


Texture::~Texture() {
    if (data != nullptr) {
        free(data);
    }
    if (rotdata != nullptr) {
        delete[] rotdata;
    }
}


void Texture::tighten_image() {
    int imin = height, imax = 0, jmin = width, jmax = 0;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (rotdata[i * width + j].is_color()) {
                imax = i + 1;
                if (imin > i) {
                    imin = i;
                }
                if (jmax < j) {
                    jmax = j + 1;
                }
                if (jmin > j) {
                    jmin = j;
                }
            }
        }
    }
    if (jmax - jmin <= 0 || imax - imin <= 0) {
        return;
    }
    int new_h = imax - imin + 2, new_w = jmax - jmin + 2;
    Pixel *tight_data = new Pixel[new_h * new_w];
    Pixel *tight_data2 = new Pixel[new_h * new_w];
    for (int i = 0; i < height; ++i) {
        int ioff = i - imin;
        for (int j = 0; j < width; ++j) {
            if (rotdata[i * width + j].is_color()) {
                tight_data[ioff * new_w + j - jmin] = rotdata[i * width + j];
                tight_data2[ioff * new_w + j - jmin] = rotdata[i * width + j];
            }
        }
    }
    delete[] data;
    delete[] rotdata;
    data = tight_data;
    rotdata = tight_data2;
    height = new_h;
    width = new_w;
    h2 = new_h / 2;
    w2 = new_w / 2;
}


void Texture::set_rotation_theta(double theta) {
    this->theta = theta;
    if (theta < 0) {
        this->theta += 2 * M_PI;
    }
}


void Texture::calc_rotation_theta(double xdir, double ydir) {
    next_theta = 0;
    theta = std::atan2(-xdir, ydir);
    if (theta < 0) {
        theta += 2 * M_PI;
    }
}


void Texture::death_animation() {
    Pixel *new_data = new Pixel[width * height];
    for (int i = -h2; i < height - h2; ++i) {
        double ti = tan2 * i;
        for (int j = -w2; j < width - w2; ++j) {
            if (data[(i + h2) * width + (j + h2)].is_color()) {
                double jti = j - ti;
                double sjti = sin * jti;
                int32_t new_i = sjti + i;
                int32_t new_j = jti - tan2 * sjti - ti;
                if (new_i >= -h2 && new_j >= -h2 && new_i < height - h2 && new_j < width - h2) {
                    new_data[(new_i + h2) * width + (new_j + w2)] = data[(i + h2) * width + (j + w2)];
                }
            }
        }
    }
    delete[] data;
    data = new_data;
}


void Texture::death_animation2(int32_t death_speed) {
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (data[i * width + j].is_color() && rand() % death_speed == 0) {
                data[i * width + j].r = 0;
                data[i * width + j].g = 0;
                data[i * width + j].b = 0;
                data[i * width + j].a = 0;
            }
        }
    }
}


void Texture::vhflip_image() {
    Pixel *newdata = new Pixel[width * height];
    int vj = width - 1;
    for (int i = 0; i < height; ++i) {
        int vi = height - 1 - i;
        for (int j = 0; j < width; ++j) {
            if (data[i * width + j].is_color()) {
                newdata[vi * width + vj - j] = data[i * width + j];
            }
        }
    }
    delete[] data;
    data = newdata;
}


void Texture::_rotate_image(double angle) {
    double sin = std::sin(angle);
    double tan2 = std::sin(angle / 2) / std::cos(angle / 2);
    double dh2 = (double)height / 2, dw2 = (double)width / 2;
    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            if (rotdata[i * width + j].is_color()) {
                double new_i = i - dh2, new_j = j - dw2;
                new_j = int(new_j) - tan2 * int(new_i);
                new_i = int(new_i) + sin * int(new_j);
                new_j = int(new_j) - tan2 * int(new_i);
                int ii = new_i, ij = new_j;
                if (ii + dh2 >= 0 && ij + dw2 >= 0 && ii < dh2 && ij < dw2) {
                    data[int(ii + dh2) * width + int(ij + dw2)] = rotdata[i * width + j];
                }
            }
        }
    }
}


void Texture::rotate_image() {
    memset(data, 0, width * height * sizeof(Pixel));
    double angle;
    bool isflip = false;
    next_theta += theta;
    if (next_theta >= 2 * M_PI) {
        next_theta -= 2 * M_PI;
    }
    if (next_theta > M_PI / 2 && next_theta < M_PI + M_PI_2) {
        angle = next_theta + M_PI;
        isflip = true;
        if (angle >= 2 * M_PI) {
            angle -= 2 * M_PI;
        }
    } else {
        angle = next_theta;
    }
    _rotate_image(angle);
    if (isflip) {
        vhflip_image();
    }
}


// BackGround
BackGround::BackGround(const char *s) {
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

BackGround::BackGround(const BackGround &c) {
    bwidth = c.bwidth, bheight = c.bheight;
    for (int i = 0; i < bheight; ++i) {
        for (int j = 0; j < bwidth; ++j) {
            background[i][j] = c.background[i][j];
        }
    }
}


// Score
Score::Score(int32_t score_len): score_len(score_len) {
    zero.tighten_image();
    one.tighten_image();
    two.tighten_image();
    three.tighten_image();
    four.tighten_image();
    five.tighten_image();
    six.tighten_image();
    seven.tighten_image();
    eight.tighten_image();
    nine.tighten_image();
}


void Score::draw() {
    int tmp_score = score;
    int len = 0;
    Texture draw_tex;
    std::vector<int32_t> nums;
    while (len != score_len) {
        nums.push_back(tmp_score % 10);
        tmp_score /= 10;
        len++;
    }
    int joff = 0;
    for (int i = nums.size() - 1; i >= 0; --i) {
        switch (nums[i]) {
            case 0:
                draw_tex = zero;
                break;
            case 1:
                draw_tex = one;
                break;
            case 2:
                draw_tex = two;
                break;
            case 3:
                draw_tex = three;
                break;
            case 4:
                draw_tex = four;
                break;
            case 5:
                draw_tex = five;
                break;
            case 6:
                draw_tex = six;
                break;
            case 7:
                draw_tex = seven;
                break;
            case 8:
                draw_tex = eight;
                break;
            case 9:
                draw_tex = nine;
                break;
        }
        for (int i = 0; i < draw_tex.get_h(); ++i) {
            for (int j = 0; j < draw_tex.get_w(); ++j) {
                if (draw_tex[i * draw_tex.get_w() + j].is_color()) {
                buffer[i][j + joff] = draw_tex[i * draw_tex.get_w() + j].alpha_mix(buffer[i][j]);
                }
            }
        }
        joff += draw_tex.get_w();
    }
}

// Chaser
ChaserMob::ChaserMob(double hp, int32_t score, int xpos, int ypos, double speed, int32_t upd_ms, const char *path, uint8_t alpha): 
            hp(hp), score(score), xpos(xpos), ypos(ypos), speed(speed), upd_freq(upd_ms) {
    tex = Texture(path);
}

void ChaserMob::deal_damage(double damage) {hp -= damage;}
bool ChaserMob::is_dead() const {return hp <= 0;}
double ChaserMob::get_hp() const {return hp;}
int32_t ChaserMob::get_score() const {return score;}
double ChaserMob::get_speed() const {return speed;}
int ChaserMob::get_xpos() const {return xpos;}
int ChaserMob::get_ypos() const {return ypos;}

void ChaserMob::act(int xppos, int yppos) {
    int64_t cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (cur_time - timer > upd_freq) {
        xresidue += speed, yresidue += speed;
        int32_t xp1 = int32_t(xresidue), yp1 = int32_t(yresidue);
        xresidue -= xp1, yresidue -= yp1;
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
        xpos = std::min(std::max(xpos, tex.get_w2()), SCREEN_WIDTH - 1 - tex.get_w2());
        ypos = std::min(std::max(ypos, tex.get_h2()), SCREEN_HEIGHT - 1 - tex.get_h2());
        timer = cur_time;
    }
}

int32_t ChaserMob::draw(int32_t mob_idx) {
    int32_t pbullet_id = -1;
    int di = 0, dj = 0;
    for (int i = ypos - tex.get_h2(); i < ypos + tex.get_h2(); ++i, ++di) {
        dj = 0;
        for (int j = xpos - tex.get_w2(); j < xpos + tex.get_w2(); ++j, ++dj) {
            buffer[i][j] = tex[di * tex.get_w() + dj].alpha_mix(buffer[i][j]);
            if(tex[di * tex.get_w() + dj].is_color()) {
                if (mob_map[i][j] != 0) {       //Pbullet here
                    pbullet_id = mob_map[i][j];
                }
                mob_map[i][j] = (mob_idx | MOBS_CODE);
            }
        }
    }
    return pbullet_id;
}

// Bouncer
BouncerMob::BouncerMob(double hp, int32_t score, int xpos, int ypos, double xdir, double ydir, int32_t upd_ms, const char *path, uint8_t alpha): 
            hp(hp), score(score), xpos(xpos), ypos(ypos), xdir(xdir), ydir(ydir), upd_freq(upd_ms) {
    tex = Texture(path);
    tex.set_rotation_theta(M_PI / 18);
}

void BouncerMob::deal_damage(double damage) {hp -= damage;}
bool BouncerMob::is_dead() const {return hp <= 0;}
double BouncerMob::get_hp() const {return hp;}
int32_t BouncerMob::get_score() const {return score;}
double BouncerMob::get_speed() const {return speed;}
int BouncerMob::get_xpos() const {return xpos;}
int BouncerMob::get_ypos() const {return ypos;}

void BouncerMob::act(int xppos, int yppos) {
    int64_t cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (cur_time - timer > upd_freq) {
        tex.rotate_image();
        xresidue += xdir, yresidue += ydir;
        int32_t xp1 = int32_t(xresidue), yp1 = int32_t(yresidue);
        xresidue -= xp1, yresidue -= yp1;
        xpos += xp1;
        ypos += yp1;
        if (xpos < tex.get_w2() || xpos > SCREEN_WIDTH - 1 - tex.get_w2()) {
            xdir = -xdir * (rand() * acc_modifier + 0.9);
        } else if (ypos < tex.get_h2() || ypos > SCREEN_HEIGHT - 1 - tex.get_h2()) {
            ydir = -ydir * (rand() * acc_modifier + 0.9);
        }
        xpos = std::min(std::max(xpos, tex.get_w2()), SCREEN_WIDTH - 1 - tex.get_w2());
        ypos = std::min(std::max(ypos, tex.get_h2()), SCREEN_HEIGHT - 1 - tex.get_h2());
        timer = cur_time;
    }
}

int32_t BouncerMob::draw(int32_t mob_idx) {
    int32_t pbullet_id = -1;
    int di = 0, dj = 0;
    for (int i = ypos - tex.get_h2(); i < ypos + tex.get_h2(); ++i, ++di) {
        dj = 0;
        for (int j = xpos - tex.get_w2(); j < xpos + tex.get_w2(); ++j, ++dj) {
            buffer[i][j] = tex[di * tex.get_w() + dj].alpha_mix(buffer[i][j]);
            if (tex[di * tex.get_w() + dj].is_color()) {
                if (mob_map[i][j] != 0) {
                    pbullet_id = mob_map[i][j];
                }
                mob_map[i][j] = (mob_idx | MOBS_CODE);
            }
        }
    }
    return pbullet_id;
}


// Shooter
AngleShooterMob::AngleShooterMob(double hp, int32_t score, int xpos, int ypos, double speed, int32_t upd_ms, const char *path, const char *path_bullet, uint8_t alpha): 
            hp(hp), score(score), xpos(xpos), ypos(ypos), speed(speed), upd_freq(upd_ms) {
    tex = Texture(path);
    bullet = Texture(path_bullet);

}

void AngleShooterMob::deal_damage(double damage) {hp -= damage;}
bool AngleShooterMob::is_dead() const {return hp <= 0;}
double AngleShooterMob::get_hp() const {return hp;}
int32_t AngleShooterMob::get_score() const {return score;}
double AngleShooterMob::get_speed() const {return speed;}
int AngleShooterMob::get_xpos() const {return xpos;}
int AngleShooterMob::get_ypos() const {return ypos;}


// Living Objects
void Living_Objects::remake_vector() {
    std::vector<Object*> tmp, tmp2;
    for (int i = 0; i < objects.size(); ++i) {
        if (objects[i] != nullptr) {
            tmp.push_back(objects[i]);
        }
    }
    for (int i = 0; i < pbullets.size(); ++i) {
        if (pbullets[i] != nullptr) {
            tmp2.push_back(pbullets[i]);
        }
    }
    objects = tmp;
    pbullets = tmp2;
}

void Living_Objects::add(Object *obj) {
    objects.push_back(obj);
}

void Living_Objects::add_pbullet(Object *obj) {
    pbullets.push_back(obj);
}

void Living_Objects::act(int xppos, int yppos) {
    for (int i = 0; i < objects.size(); ++i) {
        if (objects[i] != nullptr) {
            objects[i]->act(xppos, yppos);
        }
    }
    for (int i = 0; i < pbullets.size(); ++i) {
        if (pbullets[i] != nullptr) {
            pbullets[i]->act(xppos, yppos);
        }
    }
}

int32_t Living_Objects::draw() {
    int32_t score = 0;
    if (num_deleted > 30) {
        remake_vector();
    }
    for (int i = 0; i < pbullets.size(); ++i) {
        if (pbullets[i] == nullptr) {
            continue;
        }
        if (pbullets[i]->is_dead()) {
            delete pbullets[i];
            pbullets[i] = nullptr;
            num_deleted++;
        } else {
             pbullets[i]->draw(i);
        }
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
            int32_t pbid = objects[i]->draw(i);
            if (pbid != -1) {
                objects[i]->deal_damage(pbullets[pbid]->get_damage());
                pbullets[pbid]->deal_damage(1.0);
                if (objects[i]->is_dead()) {
                    score += objects[i]->get_score();
                }
            }
        }
    }
    return score;
}

Living_Objects::~Living_Objects() {
    for (int i = 0; i < objects.size(); ++i) {
        if (objects[i] != nullptr)
            delete objects[i];
    }
}


// Player
Player::Player(double speed, double shoot_speed_ms, double damage, double xpos, double ypos, double xdir, double ydir, const char *path, const char *bpath):
            speed(speed), shoot_speed_ms(shoot_speed_ms), damage(damage), xpos(xpos), ypos(ypos), xdir(xdir), ydir(ydir) {
    tex = Texture(path);
    bullet_tex = Texture(bpath);
}


void Player::act() {
    int64_t cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (cur_time - timer > upd_freq) {
        tex.calc_rotation_theta(xdir, ydir);
        tex.rotate_image();
        xpos += xspeed * speed;
        ypos += yspeed * speed;
        xpos = std::min(std::max(xpos, tex.get_w2()), SCREEN_WIDTH - 1 - tex.get_w2());
        ypos = std::min(std::max(ypos, tex.get_h2()), SCREEN_HEIGHT - 1 - tex.get_h2());
        timer = cur_time;
        xspeed = 0;
        yspeed = 0;
    }
}


void Player::draw() {
    int di = 0, dj = 0;
    for (int i = ypos - tex.get_h2(); i < ypos + tex.get_h2(); ++i, ++di) {
        dj = 0;
        for (int j = xpos - tex.get_w2(); j < xpos + tex.get_w2(); ++j, ++dj) {
            buffer[i][j] = tex[di * tex.get_w() + dj].alpha_mix(buffer[i][j]);
            if(tex[di * tex.get_w() + dj].is_color() && (mob_map[i][j] & MOBS_CODE)) {
                isdead = true;
            }
        }
    }
}


bool Player::can_shoot() {
    int64_t cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (cur_time - last_shot_time > shoot_speed_ms) {
        last_shot_time = cur_time;
        return true;
    }
    return false;
}

// Player Bullet
PlayerBullet::PlayerBullet(double damage, double speed, double xdir, double ydir, int32_t xpos, int32_t ypos, Texture &tex, int32_t upd_freq):
            damage(damage), speed(speed), xdir(xdir), ydir(ydir), xpos(xpos), ypos(ypos), tex(tex), upd_freq(upd_freq) {
    sdir = std::sqrt(xdir * xdir + ydir * ydir);
    this->xdir /= sdir;
    this->ydir /= sdir;
    this->tex.calc_rotation_theta(xdir, ydir);
    this->tex.rotate_image();
}


void PlayerBullet::act(int32_t xppos, int32_t yppos) {
    int64_t cur_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (cur_time - timer > upd_freq) {
        xresidue += xdir * speed, yresidue += ydir * speed;
        int32_t xp1 = int32_t(xresidue), yp1 = int32_t(yresidue);
        xresidue -= xp1, yresidue -= yp1;
        xpos += xp1;
        ypos += yp1;
        if (xpos < tex.get_w2() || xpos > SCREEN_WIDTH - 1 - tex.get_w2() || ypos < tex.get_h2() || ypos > SCREEN_HEIGHT - 1 - tex.get_h2()) {
            hp = -1.0;
        }
        timer = cur_time;
    }
}


int32_t PlayerBullet::draw(int32_t mob_idx) {
    int di = 0, dj = 0;
    for (int i = ypos - tex.get_h2(); i < ypos + tex.get_h2(); ++i, ++di) {
        dj = 0;
        for (int j = xpos - tex.get_w2(); j < xpos + tex.get_w2(); ++j, ++dj) {
            buffer[i][j] = tex[di * tex.get_w() + dj].alpha_mix(buffer[i][j]);
            if(tex[di * tex.get_w() + dj].is_color()) {
                mob_map[i][j] = mob_idx;
            }
        }
    }
    return -1;
}