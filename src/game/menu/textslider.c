#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>

#include "game/menu/textslider.h"
#include "game/menu/widget.h"
#include "audio/sound.h"
#include "utils/log.h"
#include "utils/compat.h"

typedef struct {
    char *text;
    font *font;
    int ticks;
    int dir;
    int pos_;
    int *pos;
    int has_off;
    int positions;

    void *userdata;
    textslider_slide_cb slide;
} textslider;

void textslider_render(component *c) {
    textslider *tb = widget_get_obj(c);
    char buf[100];
    int chars;
    int width;
    int xoff;
    sprintf(buf, "%s ", tb->text);
    chars = strlen(buf);
    if (tb->has_off && *tb->pos == 0) {
        buf[chars] = 'O';
        buf[chars+1] = 'F';
        buf[chars+2] = 'F';
        buf[chars+3] = '\0';
    } else {
        for(int i = 0; i < tb->positions; i++) {
            if (i+1 > *tb->pos) {
                buf[chars+i] = '|';
            } else {
                buf[chars+i] = 127;
            }
        }
        // null terminator
        buf[chars+tb->positions] = '\0';
    }
    chars = strlen(buf);
    width = chars*tb->font->w;
    xoff = (c->w - width)/2;
    if(component_is_selected(c)) {
        int t = tb->ticks / 2;
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(80 - t, 220 - t*2, 80 - t, 255));
    } else if (component_is_disabled(c)) {
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(121, 121, 121, 255));
    } else {
        font_render(tb->font, buf, c->x + xoff, c->y, color_create(0, 121, 0, 255));
    }
}

int textslider_action(component *c, int action) {
    textslider *tb = widget_get_obj(c);
    if (action == ACT_KICK || action == ACT_PUNCH || action == ACT_RIGHT) {
        (*tb->pos)++;
        if (*tb->pos > tb->positions) {
            *tb->pos = tb->positions;
        } else {
            // Play menu sound
            sound_play(20, 0.5f, 0.5f, 2.0f);
        }
        if(tb->slide) {
            tb->slide(c, tb->userdata, *tb->pos);
        }
        return 0;
    } else  if(action == ACT_LEFT) {
        (*tb->pos)--;
        if (tb->has_off && *tb->pos < 0) {
            *tb->pos = 0;
        } else if (*tb->pos < 1) {
            *tb->pos = 1;
        } else {
            // Play menu sound
            sound_play(20, 0.5f, -0.5f, 2.0f);
        }
        if(tb->slide) {
            tb->slide(c, tb->userdata, *tb->pos);
        }
        return 0;
    }
    return 1;
}

void textslider_tick(component *c) {
    textslider *tb = widget_get_obj(c);
    if(!tb->dir) {
        tb->ticks++;
    } else {
        tb->ticks--;
    }
    if(tb->ticks > 120) {
        tb->dir = 1;
    }
    if(tb->ticks == 0) {
        tb->dir = 0;
    }
}

void textslider_free(component *c) {
    textslider *tb = widget_get_obj(c);
    free(tb->text);
    free(tb);
}

component* textslider_create(font *font, const char *text, unsigned int positions, int has_off, textslider_slide_cb cb, void *userdata) {
    component *c = widget_create();

    textslider *tb = malloc(sizeof(textslider));
    memset(tb, 0, sizeof(textslider));
    tb->text = strdup(text);
    tb->font = font;
    tb->ticks = 0;
    tb->dir = 0;
    tb->pos_ = 1;
    tb->pos = &tb->pos_;
    tb->has_off = has_off;
    tb->positions = positions;
    tb->userdata = userdata;
    tb->slide = cb;
    widget_set_obj(c, tb);

    widget_set_render_cb(c, textslider_render);
    widget_set_action_cb(c, textslider_action);
    widget_set_tick_cb(c, textslider_tick);
    widget_set_free_cb(c, textslider_free);
    return c;
}

component* textslider_create_bind(font *font, const char *text, unsigned int positions, int has_off, textslider_slide_cb cb, void *userdata, int *bind) {
    component *c = textslider_create(font, text, positions, has_off, cb, userdata);
    textslider *ts = widget_get_obj(c);
    ts->pos = (bind) ? bind : &ts->pos_;
    return c;
}


