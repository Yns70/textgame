#include "textgame.h"
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>


int main(const int argc, const char* argv[]) {

    terminal_init();

    // Create the map
    Image map;
    image_resize(map, terminal_size());

    for (int x = 1; x < map.size.x - 1; ++x) {
        image_set(map, Vector2i(x, 0), Pixel(RED, u'═'));
        image_set(map, Vector2i(x, map.size.y - 1), Pixel(RED, u'═'));
    }
    
    image_set(map, Vector2i(0, 0), Pixel(RED, u'╔'));
    image_set(map, Vector2i(0, map.size.y - 1), Pixel(RED, u'╚'));
    
    image_set(map, Vector2i(map.size.x - 1, 0), Pixel(RED, u'╗'));
    image_set(map, Vector2i(map.size.x - 1, map.size.y - 1), Pixel(RED, u'╝'));
    
    for (int y = 1; y < map.size.y - 1; ++y) {
        image_set(map, Vector2i(0, y), Pixel(RED, u'║'));
        image_set(map, Vector2i(map.size.x - 1, y), Pixel(RED, u'║'));
    }
    
    // Some random trees
    for (int i = 0; i < 100; ++i) {
        const int x = rand() % (map.size.x - 2) + 1;
        const int y = rand() % (map.size.y - 2) + 1;
        image_set(map, Vector2i(x, y), Pixel(GREEN, u'♣'));
    }

    // Create framebuffer
    Image framebuffer;
    image_resize(framebuffer, terminal_size());
    
    // Player position
    Vector2i player_pos(map.size.x / 2, map.size.y / 2);

    Key ch = KEY_NONE;
    while (ch != KEY_ESCAPE) {
        ch = terminal_key();
        
        // Handle player movement
        Vector2i new_pos = player_pos;
        if (ch == 'w' || ch == 'W') {
            --new_pos.y;
        } else if (ch == 's' || ch == 'S') {
            ++new_pos.y;
        } else if (ch == 'a' || ch == 'A') {
            --new_pos.x;
        } else if (ch == 'd' || ch == 'D') {
            ++new_pos.x;
        }
        
        // Check if new position is valid (space character)
        Pixel target_pixel = image_get(map, new_pos);
        if (target_pixel.ch == u' ') {
            player_pos = new_pos;
        }
        
        // Blit map to framebuffer
        image_blit(framebuffer, Vector2i(0, 0), map, Vector2i(0, 0), map.size, true);
        
        // Draw player
        image_set(framebuffer, player_pos, Pixel(YELLOW, u'@'));
        
        image_display(framebuffer);
        
        // 1/60 second
        usleep(16667);
    }

    terminal_cleanup();
    return 0;
}
