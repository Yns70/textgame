#include "textgame.h"
#include <stdlib.h>
#include <cassert>
#include <chrono>
#include <thread>
#include <cstdarg>
#include <codecvt>
#include <locale>
#include <queue>

const Color3 WHITE(1.0f, 1.0f, 1.0f);
const Color3 GRAY(0.5f, 0.5f, 0.5f);
const Color3 RED(1.0f, 0.0f, 0.0f);
const Color3 GREEN(0.0f, 1.0f, 0.0f);
const Color3 BLUE(0.0f, 0.0f, 1.0f);
const Color3 BLACK(0.0f, 0.0f, 0.0f);
const Color3 CYAN(0.0f, 1.0f, 1.0f);
const Color3 MAGENTA(1.0f, 0.0f, 1.0f);
const Color3 YELLOW(1.0f, 1.0f, 0.0f);
const Color3 PINK(1.0f, 0.4f, 0.7f);

Color3 hsv_to_color3(float h, float s, float v) {
    // Clamp inputs to valid ranges
    h = fmod(h, 1.0f);
    if (h < 0.0f) h += 1.0f;
    s = clamp(s, 0.0f, 1.0f);
    v = clamp(v, 0.0f, 1.0f);
    
    float c = v * s; // Chroma
    float h_sector = h * 6.0f; // Convert to 0-6 range for sectors
    float x = c * (1.0f - fabs(fmod(h_sector, 2.0f) - 1.0f));
    float m = v - c;
    
    float r, g, b;
    
    if (h_sector < 1.0f) {
        r = c; g = x; b = 0.0f;
    } else if (h_sector < 2.0f) {
        r = x; g = c; b = 0.0f;
    } else if (h_sector < 3.0f) {
        r = 0.0f; g = c; b = x;
    } else if (h_sector < 4.0f) {
        r = 0.0f; g = x; b = c;
    } else if (h_sector < 5.0f) {
        r = x; g = 0.0f; b = c;
    } else {
        r = c; g = 0.0f; b = x;
    }
    
    return Color3(r + m, g + m, b + m);
}

String format(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    
    // First, determine the required buffer size
    va_list args_copy;
    va_copy(args_copy, args);
    int size = vsnprintf(nullptr, 0, fmt, args_copy);
    va_end(args_copy);
    
    if (size < 0) {
        va_end(args);
        return String(); // Return empty string on error
    }
    
    // Allocate buffer and format the string
    std::vector<char> buffer(size + 1);
    vsnprintf(buffer.data(), buffer.size(), fmt, args);
    va_end(args);
    
    // Convert UTF-8 to UTF-32
    std::string utf8_str(buffer.data());
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    
    try {
        return converter.from_bytes(utf8_str);
    } catch (const std::range_error&) {
        // Fallback: convert character by character for invalid UTF-8
        String result;
        for (char c : utf8_str) {
            result.push_back(static_cast<char32_t>(static_cast<unsigned char>(c)));
        }
        return result;
    }
}

// Have to define these before ncurses makes them into macros
const Key KEY_NONE = 0;
const Key KEY_ESCAPE = 27;
const Key KEY_TAB = 9;
const Key KEY_ENTER = 13;
const Key KEY_DELETE = 0512;
const Key KEY_DOWN = 0402;
const Key KEY_UP = 0403;
const Key KEY_LEFT = 0404;
const Key KEY_RIGHT = 0405;
const Key KEY_HOME = 0406;
const Key KEY_END = 0550;
const Key KEY_BACKSPACE = 0407;
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

void sleep(float seconds) {
    std::this_thread::sleep_for(std::chrono::microseconds(long(seconds * 1e6f)));
}

static Mouse mouse;
static std::queue<Key> key_queue;

/* Process the OS event queue, updating mouse and key_queue */
static void process_event_queue();


/* Get the next keystroke in the keyboard queue. Keep reading until this returns '\0' */
Key terminal_read_keyboard() {
    process_event_queue();
    if (key_queue.empty()) {
        return KEY_NONE;
    } else {
        const Key k = key_queue.front();
        key_queue.pop();
        return k;
    }
}


/* Get the latest state of the mouse */
Mouse terminal_read_mouse() {
    process_event_queue();
    return mouse;
}


#ifdef _MSC_VER

#define NOMINMAX
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


static void set_cursor_visibility(BOOL v) {
    // Hide the cursor
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(out, &cursorInfo);
    cursorInfo.bVisible = v;
    SetConsoleCursorInfo(out, &cursorInfo);
}

static HANDLE hStdInput = 0;

void terminal_init() {
    // Enable UTF-8 output
    SetConsoleOutputCP(65001);
    set_cursor_visibility(FALSE);

    hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    // ENABLE_EXTENDED_FLAGS disables "quick edit" mode, which is needed for the mouse
    SetConsoleMode(hStdInput, ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS);
}


void terminal_cleanup() {
    set_cursor_visibility(TRUE);

    // Reset the terminal
    printf("\x1b[0m\n");
}


void process_event_queue() {
    static const int N = 32;
    INPUT_RECORD event_array[N];
    DWORD num_events = 0;

    GetNumberOfConsoleInputEvents(hStdInput, &num_events);
    while (num_events > 0) {
        ReadConsoleInput(hStdInput, event_array, N, &num_events);

        for (DWORD e = 0; e < num_events; ++e) {
            // https://learn.microsoft.com/en-us/windows/console/input-record-str
            const INPUT_RECORD& event = event_array[e];

            if (event.EventType & MOUSE_EVENT) {
                // https://learn.microsoft.com/en-us/windows/console/mouse-event-record-str
                mouse.position.x = event.Event.MouseEvent.dwMousePosition.X;
                mouse.position.y = event.Event.MouseEvent.dwMousePosition.Y;
                mouse.button = 0;
                mouse.button |= (event.Event.MouseEvent.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) ? 1 : 0;
                mouse.button |= (event.Event.MouseEvent.dwButtonState & RIGHTMOST_BUTTON_PRESSED) ? 2 : 0;
            } else if (event.EventType & KEY_EVENT) {
                // https://learn.microsoft.com/en-us/windows/console/key-event-record-str
                if (event.Event.KeyEvent.bKeyDown) {
                    Key k = event.Event.KeyEvent.uChar.AsciiChar;
                    if (k == 0) {
                        switch (event.Event.KeyEvent.wVirtualKeyCode) {
                        case VK_BACK:
                            k = KEY_BACKSPACE;
                            break;

                        case VK_RETURN:
                            k = KEY_ENTER;
                            break;

                        case VK_TAB:
                            k = KEY_TAB;
                            break;
                        case VK_HOME:
                            k = KEY_HOME;
                            break;

                        case VK_UP:
                            k = KEY_UP;
                            break;

                        case VK_DOWN:
                            k = KEY_DOWN;
                            break;

                        case VK_LEFT:
                            k = KEY_LEFT;
                            break;

                        case VK_RIGHT:
                            k = KEY_RIGHT;
                            break;

                        case VK_DELETE:
                            k = KEY_DELETE;
                            break;

                        case VK_F1:
                            k = KEY_F1;
                            break;

                        case VK_F2:
                            k = KEY_F2;
                            break;

                        case VK_F3:
                            k = KEY_F3;
                            break;

                        case VK_F4:
                            k = KEY_F4;
                            break;

                        case VK_F5:
                            k = KEY_F5;
                            break;

                        case VK_F6:
                            k = KEY_F6;
                            break;

                        case VK_F7:
                            k = KEY_F7;
                            break;

                        case VK_F8:
                            k = KEY_F8;
                            break;

                        case VK_F9:
                            k = KEY_F9;
                            break;

                        case VK_F10:
                            k = KEY_F10;
                            break; 
                        }
                    }

                    if (k) {
                        key_queue.push(k);
                    }
                }
            }
        }

        GetNumberOfConsoleInputEvents(hStdInput, &num_events);
    };

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

    // Make the mouse responsive (disable click delay)
    mouseinterval(0);
    
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);

    // Enable mouse *movement* tracking at the terminal level.  This
    // causes the mouse to send fake keystrokes. iTerm2 doesn't reliably
    // send movement events, however--it seems to need a few focus events
    // before it starts streaming them.
    printf("\033[?1003h\033[2K\n");
}


void process_event_queue() {
    // getch() is modified to be non-blocking by
    // curses.
    Key k = getch();

    while (k != ERR) {
        if (k == KEY_MOUSE) {
            MEVENT event;

            if (getmouse(&event) == OK) {

                mouse.position.x = event.x;
                mouse.position.y = event.y;

                if (event.bstate & BUTTON1_PRESSED) {
                    mouse.button |= 0x1;
                }

                if (event.bstate & BUTTON1_RELEASED) {
                    mouse.button &= 0xFF - 0x1;
                }

                if (event.bstate & BUTTON2_PRESSED) {
                    mouse.button |= 0x2;
                }

                if (event.bstate & BUTTON2_RELEASED) {
                    mouse.button &= 0xFF - 0x2;
                }

                if (event.bstate & BUTTON3_PRESSED) {
                    mouse.button |= 0x4;
                }

                if (event.bstate & BUTTON3_RELEASED) {
                    mouse.button &= 0xFF - 0x4;
                }

                if (event.bstate & BUTTON4_PRESSED) {
                    mouse.button |= 0x8;
                }

                if (event.bstate & BUTTON4_RELEASED) {
                    mouse.button &= 0xFF - 0x8;
                }

                if (event.bstate & BUTTON5_PRESSED) {
                    mouse.button |= 0x10;
                }

                if (event.bstate & BUTTON5_RELEASED) {
                    mouse.button &= 0xFF - 0x10;
                }
            }
        } else {
            key_queue.push(k);
        }

        k = getch();
    }
}


void terminal_cleanup() {
    // Disable mouse movement tracking (and then erase this line)
    printf("\033[?1000l\033[2K\n");
    
    // Reset the terminal
    printf("\x1b[0m\033[2K\n");

    // Shut down ncurses
    endwin();
}


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

//////////////////////////////////////////////////////////////////////////////////

// Global operator implementations for Color3
Color3 operator+(const Color3& a, const Color3& b) {
    return Color3(a.r + b.r, a.g + b.g, a.b + b.b);
}

Color3& operator+=(Color3& a, const Color3& b) {
    a.r += b.r;
    a.g += b.g;
    a.b += b.b;
    return a;
}

Color3 operator-(const Color3& a, const Color3& b) {
    return Color3(a.r - b.r, a.g - b.g, a.b - b.b);
}

Color3& operator-=(Color3& a, const Color3& b) {
    a.r -= b.r;
    a.g -= b.g;
    a.b -= b.b;
    return a;
}

Color3 operator*(const Color3& a, const Color3& b) {
    return Color3(a.r * b.r, a.g * b.g, a.b * b.b);
}

Color3 operator*(const Color3& a, float b) {
    return Color3(a.r * b, a.g * b, a.b * b);
}

Color3& operator*=(Color3& a, const Color3& b) {
    a.r *= b.r;
    a.g *= b.g;
    a.b *= b.b;
    return a;
}

Color3 operator/(const Color3& a, float b) {
    return Color3(a.r / b, a.g / b, a.b / b);
}

Color3 operator/(const Color3& a, const Color3& b) {
    return Color3(a.r / b.r, a.g / b.g, a.b / b.b);
}

Color3& operator/=(Color3& a, const Color3& b) {
    a.r /= b.r;
    a.g /= b.g;
    a.b /= b.b;
    return a;
}

bool operator==(const Color3& a, const Color3& b) {
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

bool operator!=(const Color3& a, const Color3& b) {
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
    f.clip.push_back({Vector2i(0, 0), f.size - Vector2i(1, 1)});
    image_clear(f, ' ');
}


void image_clear(Image& f, Pixel value) {
    const int n = f.size.x * f.size.y;
    Pixel* p = f.data.data();
    for (int i = 0; i < n; ++i, ++p) {
        *p = value;
    }
}


void image_set(Image& f, Vector2i pix, Pixel val, bool overwrite_bg) {
    assert(!f.clip.empty());
    
    if (val.ch != '\0') {
        // Check if pixel is within current clipping region
        const Rect& clip = f.clip.back();
        if ((pix.x >= clip.min.x) && (pix.x <= clip.max.x) &&
            (pix.y >= clip.min.y) && (pix.y <= clip.max.y)) {
            
            if (! overwrite_bg) {
                // Preserve existing background color
                val.bg = f.data[pix.x + pix.y * f.size.x].bg;
            }
            
            f.data[pix.x + pix.y * f.size.x] = val;
        }
    }
}

void image_set_bg(Image& f, Vector2i pix, Color3 bg) {
    assert(!f.clip.empty());
    
    // Check if pixel is within current clipping region
    const Rect& clip = f.clip.back();
    if (pix.x >= clip.min.x && pix.x <= clip.max.x &&
        pix.y >= clip.min.y && pix.y <= clip.max.y) {
        
        // Only modify the background color, preserve ch and fg
        Pixel& existing = f.data[pix.x + pix.y * f.size.x];
        existing.bg = bg;
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


/* https://en.wikipedia.org/wiki/ANSI_escape_code#24-bit */
static uint8_t unorm_to_uint8(float value) {
    return static_cast<uint8_t>(round(std::max(0.0f, std::min(1.0f, value)) * 255.0f));
}

void image_display(Image& f) {
    // Get terminal dimensions for clipping
    const Vector2i term_size = terminal_size();
    
    // Calculate the actual drawing area (intersection of image and terminal)
    Vector2i draw_size;
    draw_size.x = std::min(f.size.x, term_size.x);
    draw_size.y = std::min(f.size.y, term_size.y);
    
    // Skip if nothing to draw
    if (draw_size.x <= 0 || draw_size.y <= 0) {
        return;
    }

    // Avoid per-frame allocation by caching the buffer
    static char* buffer = nullptr;
    static size_t buffer_size = 0;

    // Allocate a buffer large enough for the clipped image when converted to UTF-8.
    // Each pixel requires: 
    //     Up to 4 bytes for the character
    //     18 bytes for the fg color
    //     18 bytes for the bg color
    //   -----
    //     40 bytes per pixel
    //
    // Plus up to 12 bytes for cursor positioning per line
    // One byte is required for the null terminator on the entire string
    size_t required_size = (40 * draw_size.x + 12) * draw_size.y + 1;

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
            
            b += snprintf(b, required_size - (b - buffer), "\033[38;2;%d;%d;%dm\033[48;2;%d;%d;%dm",
                   unorm_to_uint8(p->fg.r),
                   unorm_to_uint8(p->fg.g),
                   unorm_to_uint8(p->fg.b),
                   unorm_to_uint8(p->bg.r),
                   unorm_to_uint8(p->bg.g),
                   unorm_to_uint8(p->bg.b));
            
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
            
            // Set pixel (clipping handled by image_set)
            image_set(dst, dst_pos, src_pixel, overwrite_bg);
        }
    }
}


int image_print(Image& img, Vector2i corner, const String& str, Color3 fg, Color3 bg, bool overwrite_bg, int word_wrap) {
    assert(!img.clip.empty());
    
    Vector2i pos = corner;
    int lines_written = 1; // Start with 1 since we're on the first line
    
    const int max_lookback = std::min(word_wrap / 2, 10);
    for (size_t i = 0; i < str.length(); ++i) {
        Character ch = str[i];
            
        // Handle explicit newlines
        if (ch == U'\n') {
            pos.x = corner.x;
            ++pos.y;
            ++lines_written;
            continue;
        }

        // Look ahead to see if we need to wrap within the next max_lookback characters
        bool need_wrap = false;
        size_t wrap_pos = i;
        
        // Quick check: could the remaining string possibly exceed the wrap boundary?
        size_t remaining_chars = str.length() - i;
        int chars_until_wrap = (corner.x + word_wrap) - pos.x;
        if (remaining_chars > static_cast<size_t>(chars_until_wrap)) {
            // Look ahead to find where we would exceed the boundary
            int temp_x = pos.x;
            for (size_t j = i; j < str.length() && j < i + max_lookback; ++j) {
                if (str[j] == U'\n') break; // Stop at explicit newlines
                if (temp_x >= corner.x + word_wrap) {
                    need_wrap = true;
                    wrap_pos = j;
                    break;
                }
                ++temp_x;
            }

            if (need_wrap) {
                // Look backwards from wrap_pos to find a good break point
                size_t break_pos = wrap_pos;
                int lookback = 0;
                bool found_break = false;
                
                while ((lookback < max_lookback) && (break_pos > i) && !found_break) {
                    Character prev_ch = str[break_pos - 1];
                    if (prev_ch == U' ' || prev_ch == U'\n' || prev_ch == U'-' || 
                        prev_ch == U'.' || prev_ch == U','  || prev_ch == U';' || 
                        prev_ch == U':' || prev_ch == U'!'  || prev_ch == U'?') {
                        // Found a good break point. Break right after this character
                        found_break = true;
                        break; // Exit the search loop
                    }
                    --break_pos;
                    ++lookback;
                }
                
                // We need to process characters up to the break point first
                if (found_break) {
                    // Process characters from current position up to and including the punctuation
                    for (size_t j = i; j < break_pos; ++j) {
                        image_set(img, pos, Pixel(fg, str[j], bg), overwrite_bg);
                        ++pos.x;
                    }
                    
                    // Now handle the line break
                    pos.x = corner.x;
                    ++pos.y;
                    ++lines_written;
                    
                    // Skip spaces after the punctuation for the new line
                    i = break_pos;
                    while (i < str.length() && str[i] == U' ') {
                        ++i;
                    }
                    --i; // Will be incremented by loop
                } else {
                    // Force break case - back up one character and break
                    --i; // Back up so we can reprocess this character on the new line
                    pos.x = corner.x;
                    ++pos.y;
                    ++lines_written;
                }
                continue; // Skip the normal character processing
            }
        }

        // Place the character (clipping handled by image_set)
        image_set(img, pos, Pixel(fg, ch, bg), overwrite_bg);
        ++pos.x;
    }
    
    return lines_written;
}
