/*
  @file textgame.h

  @brief A platform independent library for text-based games such as Dwarf Fortress and Rogue

  Copyright (c) 2025 Shmuel
  Available under the the MIT License 

  Supports 32-bit Unicode, including the Unicode DOS Code Page Block (which may be
  supported by more fonts than the Unicode Box Drawing Block), the Unicode Geometric
  Shapes Block, the Unicode Arrows Block, and the Unicode Chess Pieces. Some useful characters:

  ←  ↑  →  ↓  ↔  ↕  ↖  ↗  ↘  ↙
  ⇦  ⇧  ⇨  ⇩  ↰  ↱  ↲  ↳  ↴

  ░  ▒  ▓  █  ▄  ▌  ▐  ▀

  ┌  ┬  ┐  ├  ┼  ┤  └  ┴  ┘  │  ─
  
  ╔  ╦  ╗  ╠  ╬  ╣  ╚  ╩  ╝  ║  ═

  ╒  ╤  ╕  ╞  ╪  ╡  ╘  ╧  ╛  ╓  ╥

  ╖  ╟  ╫  ╢  ╙  ╨  ╜

  ▲  ▼  ◀  ▶ 

  △  ▽  ◁  ▷  
  
  ▴  ▾  ◂  ▸
  
  ▵  ▿  ◃  ▹  
  
  ◢  ◣  ◤  ◥
  
  ◿  ◺  ◸  ◹    
  
  ■  □  ▢  ▣    
  
  ●  ○  ◎  ◉  ◇  ◆  ◈
  
  ♠  ♥  ♦  ♣
  
  ♤  ♡  ♢  ♧
  
  ♔  ♕  ♖  ♗  ♘  ♙
  
  ♚  ♛  ♜  ♝  ♞  ♟

 */
#pragma once

#include <algorithm>
#include <string>
#include <vector>

template<typename T>
T clamp(T value, T min_val, T max_val) {
    return std::max(min_val, std::min(value, max_val));
}

using String = std::u32string;
using Character = char32_t;
struct Vector2i;
struct Color3i;

/////////////////////////////////////////////////////////////////////////

/* See the KEY_ constants */
using Key = uint32_t;

extern const Key KEY_NONE;
extern const Key KEY_ESCAPE;
extern const Key KEY_TAB;
extern const Key KEY_ENTER;
extern const Key KEY_DELETE;
extern const Key KEY_DOWN;
extern const Key KEY_UP;
extern const Key KEY_LEFT;
extern const Key KEY_RIGHT;
extern const Key KEY_HOME;
extern const Key KEY_BACKSPACE;
extern const Key KEY_F0;
extern const Key KEY_F1;
extern const Key KEY_F2;
extern const Key KEY_F3;
extern const Key KEY_F4;
extern const Key KEY_F5;
extern const Key KEY_F6;
extern const Key KEY_F7;
extern const Key KEY_F8;
extern const Key KEY_F9;
extern const Key KEY_F10;


/////////////////////////////////////////////////////////////////////////

void terminal_init();
void terminal_cleanup();
Key terminal_key();
Vector2i terminal_size();

void sleep(float seconds);

/////////////////////////////////////////////////////////////////////////

struct Vector2i {
    int x;
    int y;

    Vector2i() : x(0), y(0) {}
    Vector2i(int x, int y) : x(x), y(y) {}
};

// Global operator forward declarations for Vector2i
Vector2i operator+(const Vector2i& a, const Vector2i& b);
Vector2i& operator+=(Vector2i& a, const Vector2i& b);
Vector2i operator-(const Vector2i& a, const Vector2i& b);
Vector2i& operator-=(Vector2i& a, const Vector2i& b);
Vector2i operator*(const Vector2i& a, const Vector2i& b);
Vector2i operator*(const Vector2i& a, int b);
Vector2i& operator*=(Vector2i& a, const Vector2i& b);
Vector2i operator/(const Vector2i& a, int b);
Vector2i operator/(const Vector2i& a, const Vector2i& b);
Vector2i& operator/=(Vector2i& a, const Vector2i& b);
bool operator==(const Vector2i& a, const Vector2i& b);
bool operator!=(const Vector2i& a, const Vector2i& b);

///////////////////////////////////////////////////////////////////////

/* Inclusive on both edges */
struct Rect {
    Vector2i    min;
    Vector2i    max;
};

/* Calculate the intersection of two rectangles */
Rect rect_intersect(const Rect& a, const Rect& b);

///////////////////////////////////////////////////////////////////////

/* ANSI color with each channel on the range 0-5 */
struct Color3i {
    int r;
    int g;
    int b;

    Color3i() : r(0), g(0), b(0) {}
    Color3i(int r, int g, int b) : r(r), g(g), b(b) {}
};

extern const Color3i WHITE;
extern const Color3i RED;
extern const Color3i GREEN;
extern const Color3i BLUE;
extern const Color3i BLACK;
extern const Color3i CYAN;
extern const Color3i MAGENTA;
extern const Color3i YELLOW;

// Global operator forward declarations for Color3i
Color3i operator+(const Color3i& a, const Color3i& b);
Color3i& operator+=(Color3i& a, const Color3i& b);
Color3i operator-(const Color3i& a, const Color3i& b);
Color3i& operator-=(Color3i& a, const Color3i& b);
Color3i operator*(const Color3i& a, const Color3i& b);
Color3i operator*(const Color3i& a, int b);
Color3i& operator*=(Color3i& a, const Color3i& b);
Color3i operator/(const Color3i& a, int b);
Color3i operator/(const Color3i& a, const Color3i& b);
Color3i& operator/=(Color3i& a, const Color3i& b);
bool operator==(const Color3i& a, const Color3i& b);
bool operator!=(const Color3i& a, const Color3i& b);

////////////////////////////////////////////////////////////////////////

struct Pixel {
    Color3i      fg;
    Character    ch;
    Color3i      bg;
    
    Pixel() : fg(), ch(0), bg() {}
    Pixel(Character c) : fg(), ch(c), bg() {}
    Pixel(Color3i f, Character c, Color3i b = BLACK) : fg(f), ch(c), bg(b) {}
    Pixel(Color3i b) : fg(WHITE), ch(0), bg(b) {}
};


struct Image {
    Vector2i    size;

    /* Row-major */
    std::vector<Pixel> data;

    /* Clipping region stack used for drawing operations. There is always at least one clipping region,
       which is the full image. */
    std::vector<Rect> clip;

    Image() {}
    Image(Vector2i size, Pixel value = Pixel(' '));
};

/* Clears the image and the clipping region stack back to the full image */
void image_resize(Image& img, Vector2i new_size);
void image_clear(Image& img, Pixel value);

/* Obeys the current clipping region */ 
void image_set(Image& img, Vector2i pix, Pixel val);

/* Ignores the current clipping region */
Pixel image_get(const Image& img, Vector2i pix);
void image_display(Image& img);

/* Pops the top of the clipping region stack, if it has more than 1 region*/
void image_pop_clip(Image& img);

/* Pushes the intersection of the current clipping region and the specified region. */
void image_push_intersect_clip(Image& img, Rect clip);

/* Blits src into dst, clipping to each and treating the specified character as transparent. 
   If overwrite_bg is true, the bg color of src is copied to dst, otherwise the bg color
   of each dst pixel is preserved. */
void image_blit(Image& dst, Vector2i dst_corner, const Image& src, Vector2i src_corner, Vector2i size, bool overwrite_bg = false, Character transparent = '\0');

/* Prints a string of characters into the image, starting at the specified corner. 
   Newlines or hitting word_wrap characters from corner.x will cause the text to wrap
   down to the next y line, looking up to 10 characters backwards to find a breaking
   character (space, newline, or punctuation) at which to break the current line. Obeys the current
   image clipping region. Returns the number of lines written. */
int image_print(Image& img, Vector2i corner, const String& str, Color3i fg = WHITE, Color3i bg = BLACK, bool overwrite_bg = false, int word_wrap = INT_MAX);

#ifdef _MSC_VER
// https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/kbhit?view=msvc-170
#include <conio.h>

#else
int _kbhit();

#endif
