/*
 * MIT License
 *
 * Copyright (c) 2022 <#author_name#>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef MORSE_FACE_H_
#define MORSE_FACE_H_

#include "movement.h"

typedef enum {
    waiting,
    set_wpm,
    set_fwpm,
    set_tone,
    running,
} morse_mode_t;

typedef struct {
    // Anything you need to keep track of, put it here!
    uint8_t wpm;
    uint8_t fwpm;
    uint8_t tone;
    uint8_t t_dit;
    uint8_t t_fdit;
    morse_mode_t mode;
} morse_state_t;

void _morse_face_dit(uint8_t count, void *context);
void _morse_face_dah(uint8_t count, void *context);
void _morse_face_codenum(uint16_t code, void *context);
void _morse_face_char(char ch, void *context);
void _morse_face_message(char *msg, void *context);

uint8_t _morse_face_tdit(uint8_t wpm);
uint8_t _morse_face_tfdit(uint8_t wpm, uint8_t fwpm);

void morse_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr);
void morse_face_activate(movement_settings_t *settings, void *context);
bool morse_face_loop(movement_event_t event, movement_settings_t *settings, void *context);
void morse_face_resign(movement_settings_t *settings, void *context);

#define morse_face ((const watch_face_t){ \
    morse_face_setup, \
    morse_face_activate, \
    morse_face_loop, \
    morse_face_resign, \
    NULL, \
})

#endif // MORSE_FACE_H_

