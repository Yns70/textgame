#include "textgame.h"
#include <stdlib.h>
#include <cassert>

const Color3i WHITE(5, 5, 5);
const Color3i RED(5, 0, 0);
const Color3i GREEN(0, 5, 0);
const Color3i BLUE(0, 0, 5);
const Color3i BLACK(0, 0, 0);
const Color3i CYAN(0, 5, 5);
const Color3i MAGENTA(5, 0, 5);
const Color3i YELLOW(5, 5, 0);

// Have to define these before ncurses makes them into macros
const Key KEY_NONE = 0;
const Key KEY_ESCAPE = 27;
const Key KEY_TAB = 8;
const Key KEY_ENTER = 13;
const Key KEY_DELETE = 0512;
const Key KEY_DOWN = 0402;
const Key KEY_UP = 0403;
const Key KEY_LEFT = 0404;
const Key KEY_RIGHT = 0405;
const Key KEY_HOME = 0406;
const Key KEY_BACKSPACE = 0407;
const Key KEY_F0 = 0410;
const Key KEY_F1 = 0411;
const Key KEY_F2 = 0412;
const Key KEY_F3 = 0413;
const Key KEY_F4 = 0414;
const Key KEY_F5 = 0415;
const Key KEY_F6 = 0416;
const Key KEY_F7 = 0417;
const Key KEY_F8 = 0420;
const Key KEY_F9 = 0421;
const Key KEY_F10 = 0422;

#ifdef _MSC_VER

#include <windows.h>
#include <stdio.h>
#include <conio.h>

Vector2i terminal_size() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hConsole == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error getting console handle.\n");
    }

    if (! GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        fprintf(stderr, "Error getting console screen buffer info.\n");
    }

    return Vector2i(csbi.srWindow.Right - csbi.srWindow.Left + 1,
                    csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
}

void terminal_cleanup() {
    // Reset the terminal
    printf("\x1b[0m\n");
}

Key terminal_key() {
    return _kbhit();
}

#else

#include <stdio.h>
#include <ncurses.h>
#include <sys/ioctl.h>
#include <unistd.h>

void terminal_init() {
    // ncurses setup
    initscr();
    cbreak();
    noecho();

    // Allow KEY_n
    keypad(stdscr, TRUE);
    
    // Make getch() non-blocking
    nodelay(stdscr, TRUE);
    
    // Hide the cursor
    curs_set(0);
}


Key terminal_key() {
    // ncurses
    const char c = getch();
    return (c == ERR) ? '\0' : c;
}


void terminal_cleanup() {
    // ncurses
    endwin();
    
    // Reset the terminal
    printf("\x1b[0m\n");
}


#if 0
/*
   Linux (POSIX) implementation of _kbhit().
   Morgan McGuire, morgan@casual-effects.com
   https://github.com/morgan3d/misc/blob/main/kbhit/kbhit.cpp
*/
static char _kbhit() {
    static const int STDIN = 0;
    static bool initialized = false;

    if (! initialized) {
        // Use termios to turn off line buffering
        termios term;
        tcgetattr(STDIN, &term);
        term.c_lflag &= ~ICANON;
        tcsetattr(STDIN, TCSANOW, &term);
        setbuf(stdin, NULL);
        initialized = true;
    }

    int bytesWaiting;
    ioctl(STDIN, FIONREAD, &bytesWaiting);
    return bytesWaiting;
}
#endif


Vector2i terminal_size() {
    // Do not use curses, which needs an entire signalling mechanism
    // to detect terminal size changes
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        return Vector2i(w.ws_col, w.ws_row);
    } else {
        return Vector2i(0, 0);
    }
}

#endif // _MSC_VER

/////////////////////////////////////////////////////////////////////////////////////////
Vector2i operator+(const Vector2i& a, const Vector2i& b) {
    return Vector2i(a.x + b.x, a.y + b.y);
}

Vector2i& operator+=(Vector2i& a, const Vector2i& b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

Vector2i operator-(const Vector2i& a, const Vector2i& b) {
    return Vector2i(a.x - b.x, a.y - b.y);
}

Vector2i& operator-=(Vector2i& a, const Vector2i& b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

Vector2i operator*(const Vector2i& a, const Vector2i& b) {
    return Vector2i(a.x * b.x, a.y * b.y);
}

Vector2i operator*(const Vector2i& a, int b) {
    return Vector2i(a.x * b, a.y * b);
}

Vector2i& operator*=(Vector2i& a, const Vector2i& b) {
    a.x *= b.x;
    a.y *= b.y;
    return a;
}

Vector2i operator/(const Vector2i& a, int b) {
    return Vector2i(a.x / b, a.y / b);
}

Vector2i operator/(const Vector2i& a, const Vector2i& b) {
    return Vector2i(a.x / b.x, a.y / b.y);
}

Vector2i& operator/=(Vector2i& a, const Vector2i& b) {
    a.x /= b.x;
    a.y /= b.y;
    return a;
}

bool operator==(const Vector2i& a, const Vector2i& b) {
    return a.x == b.x && a.y == b.y;
}

bool operator!=(const Vector2i& a, const Vector2i& b) {
    return !(a == b);
}

// Global operator implementations for Color3i
Color3i operator+(const Color3i& a, const Color3i& b) {
    return Color3i(a.r + b.r, a.g + b.g, a.b + b.b);
}

Color3i& operator+=(Color3i& a, const Color3i& b) {
    a.r += b.r;
    a.g += b.g;
    a.b += b.b;
    return a;
}

Color3i operator-(const Color3i& a, const Color3i& b) {
    return Color3i(a.r - b.r, a.g - b.g, a.b - b.b);
}

Color3i& operator-=(Color3i& a, const Color3i& b) {
    a.r -= b.r;
    a.g -= b.g;
    a.b -= b.b;
    return a;
}

Color3i operator*(const Color3i& a, const Color3i& b) {
    return Color3i(a.r * b.r, a.g * b.g, a.b * b.b);
}

Color3i operator*(const Color3i& a, int b) {
    return Color3i(a.r * b, a.g * b, a.b * b);
}

Color3i& operator*=(Color3i& a, const Color3i& b) {
    a.r *= b.r;
    a.g *= b.g;
    a.b *= b.b;
    return a;
}

/* Integer division */
Color3i operator/(const Color3i& a, int b) {
    return Color3i(a.r / b, a.g / b, a.b / b);
}

/* Integer division */
Color3i operator/(const Color3i& a, const Color3i& b) {
    return Color3i(a.r / b.r, a.g / b.g, a.b / b.b);
}

/* Integer division */
Color3i& operator/=(Color3i& a, const Color3i& b) {
    a.r /= b.r;
    a.g /= b.g;
    a.b /= b.b;
    return a;
}

bool operator==(const Color3i& a, const Color3i& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

bool operator!=(const Color3i& a, const Color3i& b) {
    return !(a == b);
}

//////////////////////////////////////////////////////////////////////////////////

Image::Image(Vector2i size, Pixel value) {
    image_resize(*this, size);
}

void image_resize(Image& f, Vector2i new_size) {
    f.size = new_size;
    f.data.resize(f.size.x * f.size.y);
    f.clip.clear();
    f.clip.push_back({Vector2i(0, 0), f.size});
    image_clear(f, ' ');
}


void image_clear(Image& f, Pixel value) {
    const int n = f.size.x * f.size.y;
    Pixel* p = f.data.data();
    for (int i = 0; i < n; ++i, ++p) {
        *p = value;
    }
}


void image_set(Image& f, Vector2i pix, Pixel val) {
    assert(!f.clip.empty());
    
    if (val.ch != '\0') {
        // Check if pixel is within current clipping region
        const Rect& clip = f.clip.back();
        if (pix.x >= clip.min.x && pix.x <= clip.max.x &&
            pix.y >= clip.min.y && pix.y <= clip.max.y) {
            f.data[pix.x + pix.y * f.size.x] = val;
        }
    }
}


Pixel image_get(const Image& f, Vector2i pix) {
    if ((pix.x >= 0) &&
        (pix.y >= 0) &&
        (pix.x < f.size.x) &&
        (pix.y < f.size.y)) {
        return f.data[pix.x + pix.y * f.size.x];
    } else {
        return Pixel('\0');
    }
}


/**
 Converts a single 32-bit Unicode code point (char32_t) to a UTF-8 sequence.
 
 This function handles any valid Unicode code point in the range U+0000 to U+10FFFF.
 Invalid code points (above U+10FFFF or in the surrogate range U+D800 to U+DFFF)
 are converted to the UTF-8 representation of the replacement character, U+FFFD ().
 
 - c32 The input char32_t character (32-bit Unicode code point).
 - buffer A pointer to a character array where the UTF-8 sequence
   will be written. This buffer must be at least 4 bytes to handle the longest
   possible UTF-8 sequence.

 Returns the number of bytes written to the buffer.
*/
static int char32_to_utf8(char32_t c32, char* buffer) {
    // Invalid code points: above U+10FFFF or in surrogate range U+D800 to U+DFFF
    // Replace with U+FFFD, which is EF BF BD in UTF-8
    if (c32 > 0x10FFFF || (c32 >= 0xD800 && c32 <= 0xDFFF)) {
        buffer[0] = static_cast<char>(0xEF);
        buffer[1] = static_cast<char>(0xBF);
        buffer[2] = static_cast<char>(0xBD);
        return 3;
    }
    // 1-byte sequence for U+0000 to U+007F (ASCII)
    // Format: 0xxxxxxx
    else if (c32 < 0x80) {
        buffer[0] = static_cast<char>(c32);
        return 1;
    }
    // 2-byte sequence for U+0080 to U+07FF
    // Format: 110yyyyy 10xxxxxx
    else if (c32 < 0x800) {
        buffer[0] = static_cast<char>(0xC0 | (c32 >> 6));
        buffer[1] = static_cast<char>(0x80 | (c32 & 0x3F));
        return 2;
    }
    // 3-byte sequence for U+0800 to U+FFFF
    // Format: 1110zzzz 10yyyyyy 10xxxxxx
    else if (c32 < 0x10000) {
        buffer[0] = static_cast<char>(0xE0 | (c32 >> 12));
        buffer[1] = static_cast<char>(0x80 | ((c32 >> 6) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | (c32 & 0x3F));
        return 3;
    }
    // 4-byte sequence for U+10000 to U+10FFFF
    // Format: 11110www 10zzzzzz 10yyyyyy 10xxxxxx
    else {
        buffer[0] = static_cast<char>(0xF0 | (c32 >> 18));
        buffer[1] = static_cast<char>(0x80 | ((c32 >> 12) & 0x3F));
        buffer[2] = static_cast<char>(0x80 | ((c32 >> 6) & 0x3F));
        buffer[3] = static_cast<char>(0x80 | (c32 & 0x3F));
        return 4;
    }
}

/* Calculate the intersection of two rectangles */
Rect rect_intersect(const Rect& a, const Rect& b) {
    Rect result;
    result.min.x = std::max(a.min.x, b.min.x);
    result.min.y = std::max(a.min.y, b.min.y);
    result.max.x = std::min(a.max.x, b.max.x);
    result.max.y = std::min(a.max.y, b.max.y);
    
    // Ensure intersection is valid (min <= max)
    if (result.min.x > result.max.x) {
        result.max.x = result.min.x;
    }
    if (result.min.y > result.max.y) {
        result.max.y = result.min.y;
    }
    
    return result;
}

/*  https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit */
static int color3i_to_ansi(Color3i color) {
    return 16 + clamp(color.r, 0, 5) * 36 + clamp(color.g, 0, 5) * 6 + clamp(color.b, 0, 5);
}


void image_display(Image& f) {
    // Get terminal dimensions for clipping
    Vector2i term_size = terminal_size();
    
    // Calculate the actual drawing area (intersection of image and terminal)
    Vector2i draw_size;
    draw_size.x = (f.size.x < term_size.x) ? f.size.x : term_size.x;
    draw_size.y = (f.size.y < term_size.y) ? f.size.y : term_size.y;
    
    // Skip if nothing to draw
    if (draw_size.x <= 0 || draw_size.y <= 0) {
        return;
    }

    // Avoid per-frame allocation
    static char* buffer = nullptr;
    static size_t buffer_size = 0;

    // Allocate a buffer large enough for the clipped image when converted to UTF-8.
    // Each pixel requires: 
    //     Up to 4 bytes for the character
    //     11 bytes for the fg color
    //     11 bytes for the bg color
    //   -----
    //     26 bytes per pixel
    //
    // Plus up to 12 bytes for cursor positioning per line
    // One byte is required for the null terminator on the entire string
    size_t required_size = (26 * draw_size.x + 12) * draw_size.y + 1;

    if (buffer_size < required_size) {
        free(buffer);
        buffer_size = required_size;
        buffer = (char*)calloc(required_size, sizeof(char));
    }
    
    char* b = buffer;
    
    // Draw only the visible portion of the image
    for (int y = 0; y < draw_size.y; ++y) {
        // Position cursor at start of line
        b += snprintf(b, required_size - (b - buffer), "\033[%d;1H", y + 1);
        
        const Pixel* line_start = f.data.data() + y * f.size.x;
        for (int x = 0; x < draw_size.x; ++x) {
            const Pixel* p = line_start + x;
            
            b += snprintf(b, required_size - (b - buffer), "\033[38;5;%dm\033[48;5;%dm",
                   color3i_to_ansi(p->fg),
                   color3i_to_ansi(p->bg));
            
            b += char32_to_utf8(p->ch, b);
        }
    }
    
    // Null terminate the entire string
    *b = '\0';

    // Write the clipped content
    printf("%s", buffer);
    
    #ifndef _MSC_VER
    // Let curses know we've updated the screen
    refresh();
    #endif
    
}


void image_push_intersect_clip(Image& img, Rect clip) {
    assert(!img.clip.empty());
    
    // Push the intersection of current clipping region and new clip
    img.clip.push_back(rect_intersect(img.clip.back(), clip));
}


void image_pop_clip(Image& img) {
    // Only pop if there's more than one region (preserve the base region)
    if (img.clip.size() > 1) {
        img.clip.pop_back();
    }
}


void image_blit
(Image& dst,
 Vector2i dst_corner,
 const Image& src,
 Vector2i src_corner,
 Vector2i size,
 bool overwrite_bg,
 Character transparent) {
    assert(!dst.clip.empty());
    
    for (int y = 0; y < size.y; ++y) {
        for (int x = 0; x < size.x; ++x) {
            Vector2i src_pos = src_corner + Vector2i(x, y);
            Vector2i dst_pos = dst_corner + Vector2i(x, y);
            
            // Get source pixel (bounds checking handled by image_get)
            Pixel src_pixel = image_get(src, src_pos);
            
            // Skip transparent pixels
            if (src_pixel.ch == transparent) {
                continue;
            }
            
            // Prepare destination pixel
            Pixel dst_pixel = src_pixel;
            if (!overwrite_bg) {
                // Preserve destination background color
                Pixel existing = image_get(dst, dst_pos);
                dst_pixel.bg = existing.bg;
            }
            
            // Set pixel (clipping handled by image_set)
            image_set(dst, dst_pos, dst_pixel);
        }
    }
}
