#include "Objects.h"
#include "Engine.h"
#include <iostream>
#include <vector>
#include <chrono>


// Pixel::Pixel(uint32_t color) {
//     r = color >> 16;
//     g = color >> 8;
//     b = color;
//     a = color >> 24;
// }
// Pixel Pixel::swap_colors() {
//     return Pixel(b, g, r, a);
// }
// uint32_t Pixel::pixel() const {
//     return (uint32_t(a) << 24) + (uint32_t(r) << 16) + (uint32_t(g) << 8) + b;
// }
// uint32_t Pixel::alpha_mix(Pixel color) {
//     uint32_t new_r, new_g, new_b;
//     new_r = uint32_t(r) * a / 255 + uint32_t(color.r) * (255 - a) / 255;
//     new_g = uint32_t(g) * a / 255 + uint32_t(color.g) * (255 - a) / 255;
//     new_b = uint32_t(b) * a / 255 + uint32_t(color.b) * (255 - a) / 255;
//     return 0xff000000 + (new_r << 16) + (new_g << 8) + new_b;
// }
// bool Pixel::full_black() const {
//     return uint32_t(r) + g + b;
// }
// void Pixel::set_black(uint8_t alpha) {
//     if (!(this->full_black())) {
//         a = 0;
//     } else {
//         a = alpha;
//     }
// }