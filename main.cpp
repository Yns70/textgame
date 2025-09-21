#include "textgame.h"
#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>

Image image;

int main(const int argc, const char* argv[]) {

    terminal_init();
    
    Key ch = KEY_NONE;
    while (ch != KEY_ESCAPE) {
        ch = terminal_key();
        
        image_resize(image, terminal_size());
        
        for (int x = 1; x < image.size.x - 1; ++x) {
            image_set(image, Vector2i(x, 0), Pixel(RED, u'═'));
            image_set(image, Vector2i(x, image.size.y - 1), Pixel(RED, u'═'));
        }
        
        image_set(image, Vector2i(0, 0), Pixel(RED, u'╔'));
        image_set(image, Vector2i(0, image.size.y - 1), Pixel(RED, u'╚'));
        
        image_set(image, Vector2i(image.size.x - 1, 0), Pixel(RED, u'╗'));
        image_set(image, Vector2i(image.size.x - 1, image.size.y - 1), Pixel(RED, u'╝'));
        
        for (int y = 1; y < image.size.y - 1; ++y) {
            image_set(image, Vector2i(0, y), Pixel(RED, u'║'));
            image_set(image, Vector2i(image.size.x - 1, y), Pixel(RED, u'║'));
        }
        
        image_display(image);
        
        // 1/60 second
        usleep(16667);
    }

    terminal_cleanup();
    return 0;
}
