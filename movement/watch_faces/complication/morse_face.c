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

#include <stdlib.h>
#include <string.h>
#include "morse_face.h"

#define MORSE_TONE BUZZER_NOTE_C7

char message[] = "KD2TDZ";

const uint16_t CODES[] = {6,17,21,9,2,20,11,16,4,30,13,18,7,5,15,22,27,10,8,3,12,24,14,25,29,19,
                          63,62,60,56,48,32,33,35,39,47};

void _morse_face_dit(uint8_t count, void *context){
  morse_state_t *state = (morse_state_t *)context;
  for(int i=count;i>0;i--){
    watch_buzzer_play_note(MORSE_TONE, state->t_dit);
    watch_buzzer_play_note(BUZZER_NOTE_REST, state->t_dit);
  }
}

void _morse_face_dah(uint8_t count, void *context){
  morse_state_t *state = (morse_state_t *)context;
  for(int i=count;i>0;i--){
    watch_buzzer_play_note(MORSE_TONE, state->t_dit*3);
    watch_buzzer_play_note(BUZZER_NOTE_REST, state->t_dit);
  }
}

void _morse_face_codenum(uint16_t code, void *context){
  morse_state_t *state = (morse_state_t *)context;
  while(code > 1) {
    if(code % 2)
      _morse_face_dah(1, context);
    else
      _morse_face_dit(1, context);
    code /= 2;
  }
  watch_buzzer_play_note(BUZZER_NOTE_REST, state->t_fdit*3 - state->t_dit);
}

void _morse_face_char(char ch, void *context){
  //A..Z0..9
  if (ch > 'Z') ch -= 32;
  if (ch > 'Z') {
    _morse_face_codenum(256, context);
  } else if (ch < 'A'){
    if (ch >= '0' && ch <= '9')
      _morse_face_codenum(CODES[26 + (ch - '0')], context);
    else
      _morse_face_codenum(256, context);
  } else {
    _morse_face_codenum(CODES[ch - 'A'], context);
  }
}

void _morse_face_message(char *msg, void *context){
  morse_state_t *state = (morse_state_t *)context;
  uint8_t index = 0;
  char ch;
  while((ch = msg[index++])){
    if(ch == ' '){
      watch_buzzer_play_note(BUZZER_NOTE_REST, state->t_fdit*7 - state->t_dit);
    } else {
      _morse_face_char(ch, context);
    }
  }
}

uint8_t _morse_face_tdit(uint8_t wpm){
  //return 1000 * 60 / (50 * wpm);
  return 1200 / wpm;
}

uint8_t _morse_face_tfdit(uint8_t wpm, uint8_t fwpm){
  return 1000 * (300 * wpm - 186 * fwpm) / (95 * wpm * fwpm);
}


void morse_face_setup(movement_settings_t *settings, uint8_t watch_face_index, void ** context_ptr) {
    (void) settings;
    (void) watch_face_index;

    if (*context_ptr == NULL) {
        *context_ptr = malloc(sizeof(morse_state_t));
        memset(*context_ptr, 0, sizeof(morse_state_t));
        // Do any one-time tasks in here; the inside of this conditional happens only at boot.
        morse_state_t *state = (morse_state_t *)*context_ptr;
        state->mode = waiting;
        state->wpm = state->fwpm = 20;
        //calculate dit timings based on wpm and fwpm
        state->t_dit  = _morse_face_tdit(state->wpm);
        state->t_fdit = _morse_face_tfdit(state->wpm,state->fwpm);
    }
    // Do any pin or peripheral setup here; this will be called whenever the watch wakes from deep sleep.
}

void morse_face_activate(movement_settings_t *settings, void *context) {
    (void) settings;
    morse_state_t *state = (morse_state_t *)context;

    // Handle any tasks related to your watch face coming on screen.
    state->mode = waiting;
}

bool morse_face_loop(movement_event_t event, movement_settings_t *settings, void *context) {

    (void) settings;

    morse_state_t *state = (morse_state_t *)context;
    char buf[11];

    switch (event.event_type) {
        case EVENT_ACTIVATE:
            // Show your initial UI here.
          sprintf(buf, "MO        ");
          break;
        case EVENT_TICK:
            // If needed, update your display here.
            switch(state->mode){
              case waiting:
                sprintf(buf, "MO        ");
                break;
              case set_wpm:
                sprintf(buf, "W     % 2u  ",state->wpm);
                break;
              case set_fwpm:
                sprintf(buf, "F     % 2u  ",state->fwpm);
                break;
              case set_tone:
                sprintf(buf, "To    C7  ");
                break;
            }
            watch_display_string(buf, 0);

            break;
        case EVENT_MODE_BUTTON_UP:
            // You shouldn't need to change this case; Mode almost always moves to the next watch face.
            movement_move_to_next_face();
            break;
        case EVENT_LIGHT_BUTTON_UP:
            // movement_illuminate_led();
            switch(state->mode) {
              case waiting:
              case set_fwpm:
                state->mode++;
                break;
              case set_wpm:
                state->mode++;
                if(state->fwpm > state->wpm)
                  state->fwpm = state->wpm;
                break;
              case set_tone:
                state->mode = waiting;
                break;
            }
            break;
        case EVENT_ALARM_BUTTON_UP:
            switch(state->mode){
              case waiting:
                _morse_face_message(message, context);
                break;
              case set_wpm:
                state->wpm++;
                if(state->wpm > 20) 
                  state->wpm = 5;
                state->t_dit  = _morse_face_tdit(state->wpm);
                state->t_fdit  = _morse_face_tfdit(state->wpm,state->fwpm);
                break;
              case set_fwpm:
                state->fwpm++;
                if(state->fwpm > state->wpm)
                  state->fwpm = 5;
                state->t_fdit  = _morse_face_tfdit(state->wpm,state->fwpm);
                break;

            }
        case EVENT_TIMEOUT:
            // Your watch face will receive this event after a period of inactivity. If it makes sense to resign,
            // you may uncomment this line to move back to the first watch face in the list:
            // movement_move_to_face(0);
            break;
        case EVENT_LOW_ENERGY_UPDATE:
            // If you did not resign in EVENT_TIMEOUT, you can use this event to update the display once a minute.
            // Avoid displaying fast-updating values like seconds, since the display won't update again for 60 seconds.
            // You should also consider starting the tick animation, to show the wearer that this is sleep mode:
            // watch_start_tick_animation(500);
            break;
        default:
            break;
    }

    // return true if the watch can enter standby mode. If you are PWM'ing an LED or buzzing the buzzer here,
    // you should return false since the PWM driver does not operate in standby mode.
    return true;
}

void morse_face_resign(movement_settings_t *settings, void *context) {
    (void) settings;
    (void) context;

    // handle any cleanup before your watch face goes off-screen.
}

