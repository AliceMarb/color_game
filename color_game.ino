#include <Adafruit_TCS34725softi2c.h>
#include <Adafruit_NeoPixel.h>
#include "color_game.h"

state CURRENT_STATE;
// FSM Variables
// Has each square had a matching block placed in it?
boolean all_blocks_placed;
// Number of squares that have had a matching block placed in them
int num_blocks_placed;

// used for testing
boolean using_sensors = true;

void setup() {
  Serial.begin(9600);
  Serial.println("Color View Test");
  initialize_system(using_sensors);
  // Initialize FSM variables
  CURRENT_STATE = (state) sWAIT_FOR_BLOCK;
  all_blocks_placed = false;
  num_blocks_placed = 0;
//  if (using_sensors) {
//    test_sensors();
//  } else {
//    test_leds();
//    //    test_music_player();
//  }
}


void loop() {
    update_inputs();
    CURRENT_STATE = update_fsm(CURRENT_STATE, millis(), all_blocks_placed, num_blocks_placed);
    delay(10);
}

state update_fsm(state cur_state, long mils, boolean all_blocks_placed, int num_blocks_placed) {
  state next_state;
  switch (cur_state) {
    case sWAIT_FOR_BLOCK:
      Serial.println("STARTING WAIT FOR BLOCK");
      Serial.print("Num blocks placed: ");
      Serial.println(num_blocks_placed);
      Serial.print("All blocks placed? ");
      Serial.println(all_blocks_placed);
      if (!all_blocks_placed && sizeof(new_matches) >= 1) {
        num_blocks_placed += (int) sizeof(new_matches);
        play_matches();
        play_non_matches();
        display_board();
        reset_block_info();
        next_state = sWAIT_FOR_BLOCK;
      } else if (all_blocks_placed) {
        next_state = sGAME_FINISHED;
        music_playing = true;
        display_board();
        //        give some time to see final lights before setting them low
        delay(1000);
        play_finished_music();
      } else {
        next_state = sWAIT_FOR_BLOCK;
      }
      break;
    case sGAME_FINISHED:
      Serial.println("\n\n\nGAME FINISHED\n\n\n");
      if (music_playing) {
        Serial.println("\n\n Music still playing, stay in game over");
        next_state = sGAME_FINISHED;
      } else {
        // when the finished seqeuence ends, reset the board
        Serial.println("\n\nMusic is finished, restart");
        reset_board();
        reset_block_info();
        display_board();
        Serial.print("Num blocks placed: ");
        Serial.println(num_blocks_placed);
        next_state = sWAIT_FOR_BLOCK;
        Serial.println("GOING INTO WAITING STATE");
      }
      break;
  }
  return next_state;
}
