// TODO: Update the pins to the actual pins on the board
#define SDA_PIN_1 12
#define SCL_PIN_1 13
#define LED_PIN_1 5
#define SDA_PIN_2 9
#define SCL_PIN_2 8
#define LED_PIN_2 10

#define SDA_PIN_3 23 //opposite of colors!
#define SCL_PIN_3 22
#define LED_PIN_3 14

#define SDA_PIN_4 3
#define SCL_PIN_4 2
#define LED_PIN_4 2

#define SDA_PIN_5 51
#define SCL_PIN_5 50
#define LED_PIN_5 22

#define SDA_PIN_6 46
#define SCL_PIN_6 47
#define LED_PIN_6 18

/*
   Defining how each color sounds
*/
#define MUSIC_PIN A9
#define RED_FREQ 1046 // C6
#define GREEN_FREQ 1174 // D6
#define BLUE_FREQ 1318 // E6
#define NON_MATCH_FREQ 400

//neopixel ring pin
#define NEOPIXEL_PIN 11
#define HIGH_INTENSITY 255
#define LOW_INTENSITY 5
#define LIGHT_THRESHOLD 3000

/*
   Internal helper variables
*/
Adafruit_NeoPixel strip = Adafruit_NeoPixel(100, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

Adafruit_TCS34725softi2c tcs1 = Adafruit_TCS34725softi2c(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X, SDA_PIN_1, SCL_PIN_1);
Adafruit_TCS34725softi2c tcs2 = Adafruit_TCS34725softi2c(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X, SDA_PIN_2, SCL_PIN_2);
Adafruit_TCS34725softi2c tcs3 = Adafruit_TCS34725softi2c(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X, SDA_PIN_3, SCL_PIN_3);
Adafruit_TCS34725softi2c tcs4 = Adafruit_TCS34725softi2c(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X, SDA_PIN_4, SCL_PIN_4);
Adafruit_TCS34725softi2c tcs5 = Adafruit_TCS34725softi2c(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X, SDA_PIN_5, SCL_PIN_5);
Adafruit_TCS34725softi2c tcs6 = Adafruit_TCS34725softi2c(TCS34725_INTEGRATIONTIME_700MS, TCS34725_GAIN_1X, SDA_PIN_6, SCL_PIN_6);

Adafruit_TCS34725softi2c all_sensors[] = { tcs1, tcs2, tcs3, tcs4, tcs5, tcs6 };
Adafruit_TCS34725softi2c sensors[num_sensors];

int led_pins[6] = { LED_PIN_1, LED_PIN_2, LED_PIN_3, LED_PIN_4, LED_PIN_5, LED_PIN_6 };

int color_to_freq[3] = { RED_FREQ, GREEN_FREQ, BLUE_FREQ };

// Array of 6 integers, where each integer is either 0 or 1, representing if the square on the board has had any block placed into it in the past loop.
int placed_block_status[num_sensors];
// Array of 6 integers, where each integer is a value -1, 0, 1, or 2, representing the color of each of the blocks placed on the board in the past loop. If no block has been placed in the past loop, the value is -1.
int placed_block_color[num_sensors];

/*
 * Tunes
  
*/
// preset colors for Mary Had A Little Lamb
int mary_lamb_tune[6] = { 2, 1, 0, 1, 2, 0 };

/*
   Initialize system: set the input variable values, initialize all TCS
*/
void initialize_system(boolean testing_sensors) {
  randomSeed(analogRead(6));
  //  Serial.println("pins");
  initialize_leds();
  if (testing_sensors) {
    Serial.print("size of new matches: ");
    Serial.println(new_match_size);
    Serial.println("Initializing system");
    Serial.print("num sensors ");
    Serial.println(num_sensors);
    // set input variables
    reset_board();
    for (int i = 0; i < num_sensors; i++) {
      sensors[i] = all_sensors[i];
    }
    music_playing = false;
    initialize_sensors();
    display_board();
  }
}

void initialize_leds() {
  strip.begin();
  strip.show();
}

void initialize_sensors() {
  for (int i = 0; i < num_sensors; i++) {
    // check if sensor is connected
    Adafruit_TCS34725softi2c tcs = sensors[i];
    if (tcs.begin()) {
      Serial.print("Found sensor ");
      Serial.println(i + 1);
    } else {
      Serial.print("Can't find sensor ");
      Serial.println(i + 1);
      while (1);
    }
  }
}

sensor_colors read_sensor_colors(Adafruit_TCS34725softi2c tcs) {
  uint16_t clear, red, green, blue;
  tcs.setInterrupt(false); // turn on LED on tcs sensor
  tcs.getRawData(&red, &green, &blue, &clear);
  tcs.setInterrupt(true); // turn off LED on tcs sensor
  Serial.print("C:\t"); Serial.print(clear);                        //print color values
  Serial.print("\tR:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.println(blue);
  sensor_colors s;
  s.red = red;
  s.green = green;
  s.blue = blue;
  return s;
}
/*
  Check for placed blocks and color of said blocks
  Only need to check for blocks where the status_tracker is 0 -- "matched" blocks can't become unmatched
*/
void update_inputs() {
  Serial.println("\n\n\nUpdating inputs");
  Serial.print("Number of blocks correctly placed: ");
  Serial.println(num_blocks_placed);
  for (int i = 0; i < num_sensors; i++) {
    // skip checking sensors which have already been matched
    if (status_tracker[i] == 1) {
      placed_block_status[i] = 1;
      continue;
    }
    sensor_colors sensor_results = read_sensor_colors(sensors[i]);
    uint16_t red = sensor_results.red; uint16_t blue = sensor_results.blue; uint16_t green = sensor_results.green;

    // check if a block is placed by the sensor
    Serial.print("Is a block at sensor ");
    Serial.print(i + 1);
    Serial.print("? ");
    int placed_status = get_block_status(red, green, blue);
    placed_block_status[i] = placed_status;
    if (placed_status) {
      placed_block_color[i] = get_block_color(red, green, blue);
      Serial.print("Color of block: ");
      Serial.println(color_int_to_color_string(placed_block_color[i]));
      if (colors[i] == placed_block_color[i] ) {
        //  the color of the block and LED match
        // update the status tracker since this is a new match!
        status_tracker[i] = 1;
        new_matches[new_match_size] = i;
        new_match_size++;
        num_blocks_placed++;
      } else {
        non_matches[non_match_size] = i;
        non_match_size++;
      }
    } else {
      // no block here, so to avoid confusion set color to -1
      placed_block_color[i] = -1;
    }
    if (total_blocks == num_blocks_placed) {
      Serial.println("ALL BLOCKS PLACED");
      Serial.print("Total blocks: ");
      Serial.println(total_blocks);
      Serial.print("Number of blocks correctly placed: ");
      Serial.println(num_blocks_placed);
      all_blocks_placed = true;
    }
    Serial.println("\n\n");
  }
  //  Serial.println("Placed blocks");
  //  for (int i = 0; i < num_sensors; i++) {
  //    Serial.println(placed_block_status[i]);
  //  }
  //  Serial.println("Placed colors");
  //  for (int i = 0; i < num_sensors; i++) {
  //    Serial.println(placed_block_color[i]);
  //  }
  Serial.println("New matches");
  for (int i = 0; i < new_match_size; i++) {
    Serial.print("Sensor ");
    Serial.print(new_matches[i] + 1);
    Serial.print(", ");
  }
  Serial.print("Number of new matches: ");
  Serial.println(new_match_size);
  Serial.println("\n\n");
  Serial.println("Non matches");
  for (int i = 0; i < non_match_size; i++) {
    Serial.println(non_matches[i]);
  }
}


/*
   Get ready for a new round of the game: reset the board variables to random colors and LEDs to low
*/
void reset_board() {
  for (int i = 0; i < num_sensors; i++) {
    status_tracker[i] = 0;
    colors[i] = mary_lamb_tune[i];
  }
  // there can only be exactly 2 of each color
//  int num_red = 0; int num_green = 0; int num_blue = 0;
//  for (int i = 0; i < num_sensors; i++) {
//    int random_color = random(3);
//    while (random_color == 0 && num_red == 2 ||
//           random_color == 1 && num_green == 2 ||
//           random_color == 2 && num_blue == 2) {
//      random_color = random(3);
//    }
//    if (random_color == 0) {
//      num_red++;
//    } else if (random_color == 1) {
//      num_green++;
//    } else {
//      num_blue++;
//    }
//    colors[i] = random_color;
//    status_tracker[i] = 0;
//  }
  all_blocks_placed = false;
  num_blocks_placed = 0;
}
/*
   Resets the placed block arrays and the match arrays for the new loop (placed_block_status, placed_block_color, new_matches, non_matches)
*/
void reset_block_info() {
  new_match_size = 0;
  non_match_size = 0;
  // reset arrays
  for (int i = 0; i < num_sensors; i++) {
    new_matches[i] = 0;
    non_matches[i] = 0;
    placed_block_status[i] = 0;
    placed_block_color[i] = 0;
  }
}
/*
   Set each LED on the board to the corresponding color in colors, and the corresponding brightness depending on if the status_tracker value is 0 or 1.
*/
void display_board() {
  for (int i = 0; i < num_sensors; i++) {
    Serial.print("display LED color: ");
    Serial.print(color_int_to_color_string(colors[i]));
    Serial.print(" on pin ");
    Serial.print(led_pins[i]);
    if (status_tracker[i] == 1) {
      // set color to bright because a match has been found
      Serial.println(" at high intensity.");
      write_color_to_neopixel(colors[i], led_pins[i], HIGH_INTENSITY);
      //      write_color_to_rgb_led(colors[i], led_pins[i][0], led_pins[i][1], led_pins[i][2], 80);
    } else {
      Serial.println(" at low intensity.");
      //      write_color_to_rgb_led(colors[i], led_pins[i][0], led_pins[i][1], led_pins[i][2], 20);
      write_color_to_neopixel(colors[i], led_pins[i], LOW_INTENSITY);
    }
    //    strip.show();
  }
}

/*
   Check the TCS readings to predict which color the color sensor detects.
   Returns: 0, 1, or 2, depending on if the red_reading, green_reading or blue_reading is highest
   red = 0, green = 1, blue = 2
*/
int get_block_color(uint16_t red, uint16_t green, uint16_t blue) {
  if (red > green && red > blue) {
    // red detected
    return 0;
  } else if (green > blue && green > red) {
    // green detected
    return 1;
  } else if (blue > green && blue > red) {
    // blue detected
    return 2;
  } else {
    // some colors are equal, unclear if red, green, or blue...
    // return red as preliminary solution
    Serial.println("NOT SURE WHICH COLOR READ!");
    return 0;
  }
}

/*
    Check to see if the TCS readings are high enough that a block has been placed in the square next to the color sensor. Important because we only want to check for matches / play tones when we know that a block is present.
    Returns: true if we think a block has been placed, false we think the square is empty.
*/
boolean get_block_status(uint16_t red, uint16_t green, uint16_t blue) {
  if (red > LIGHT_THRESHOLD && green > LIGHT_THRESHOLD && blue > LIGHT_THRESHOLD) {
    //  if (red > 1000 && green > 1000 && blue > 1000) {
    Serial.println("YES ");
    //    Serial.print("\tR:\t"); Serial.print(red);
    //    Serial.print("\tG:\t"); Serial.print(green);
    //    Serial.print("\tB:\t"); Serial.println(blue);
    return true;
  } else {
    Serial.println("NO ");
    //    Serial.print("\tR:\t"); Serial.print(red);
    //    Serial.print("\tG:\t"); Serial.print(green);
    //    Serial.print("\tB:\t"); Serial.println(blue);
    return false;
  }
}


void test_music_player() {
  while (true) {
    Serial.println("Playing non match tone.");
    tone(MUSIC_PIN, NON_MATCH_FREQ, 500);
    delay(1000);
    Serial.println("Playing red tone.");
    tone(MUSIC_PIN, RED_FREQ, 500);
    delay(1000);
    Serial.println("Playing green tone.");
    tone(MUSIC_PIN, GREEN_FREQ, 500);
    delay(1000);
    Serial.println("Playing blue tone.");
    tone(MUSIC_PIN, BLUE_FREQ, 500);
    delay(1000);
  }
}
/*
   Play the corresponding match tone for each new match from this loop.
*/
void play_matches() {
  Serial.println("Play matches");
  Serial.print("Number of new matches: ");
  Serial.println(new_match_size);
  for (int i = 0; i < new_match_size; i++) {
    Serial.print("Sensor with new match: ");
    Serial.print(new_matches[i]);
    Serial.print(", ");
    Serial.print("Color of new match: ");
    Serial.println(color_int_to_color_string(colors[new_matches[i]]));
    play_match_music(colors[new_matches[i]]);
  }
}
/*
   Play the not match music once if there is at least one non-match.
*/
void play_non_matches() {
  if (non_match_size > 0) {
    Serial.println("Playing non matches (more than 0)");
    play_non_match_music();
  }
}

/*
   Play the corresponding tone for each square on the board.
*/
void play_finished_music() {
  // set all lights to low
  for (int i = 0; i < num_sensors; i++) {
    status_tracker[i] = 0;
  }
  display_board();
  delay(2000);
  Serial.println("PLAYING FINISHED MUSIC");
  for (int i = 0; i < num_sensors; i++) {
    Serial.print("Color of LED being played: ");
    Serial.println(color_int_to_color_string(colors[i]));
    // light up the light being played
    //    write_color_to_rgb_led(colors[i], led_pins[i][0], led_pins[i][1], led_pins[i][2], 80);
    write_color_to_neopixel(colors[i], led_pins[i], HIGH_INTENSITY);
    play_match_music(colors[i]);
    delay(1000);
  }
  music_playing = false;
  delay(4000);
}

/*
   Play a tone corresponding to the color (red, green and blue each have a unique tone)
*/
void play_match_music(int color_int) {
  tone(MUSIC_PIN, color_to_freq[color_int], 1000);
}

/*
   Play an “angry” tone to tell the user that the block does not match
*/
void play_non_match_music() {
  tone(MUSIC_PIN, NON_MATCH_FREQ, 1000);
}

void test_sensors() {
  Serial.println("Sensor Test");
  initialize_sensors();
  while (true) {
    for (int i = 0; i < num_sensors; i++) {
      Serial.print("Testing sensor ");
      Serial.println(i + 1);
      sensor_colors sensor_results = read_sensor_colors(sensors[i]);
      uint16_t red = sensor_results.red; uint16_t blue = sensor_results.blue; uint16_t green = sensor_results.green;
      int detected_color = get_block_color(red, green, blue);
      Serial.print(color_int_to_color_string(detected_color));
      Serial.println(" detected");
      Serial.println("\n");
    }
    delay(5000);
    Serial.println("\n\n\n\n");
  }
}

void test_leds() {
  Serial.println("LEDs Test");
  while (true) {
    int color_int = random(3);
    Serial.print("All LEDs should be ");
    Serial.print(color_int_to_color_string(color_int));
    Serial.println(".");
    Serial.println("All LEDs should have LESS BRIGHT light.");
    for (int i = 0; i < num_sensors; i++) {
      Serial.print("LED Pin: ");
      Serial.println(led_pins[i]);
      write_color_to_neopixel(color_int, led_pins[i], LOW_INTENSITY);
    }
    delay(5000);
    Serial.println("All LEDs should have BRIGHTER light.");
    for (int i = 0; i < num_sensors; i++) {
      Serial.print("LED Pin: ");
      Serial.println(led_pins[i]);
      write_color_to_neopixel(color_int, led_pins[i], HIGH_INTENSITY);
    }
    Serial.println("\n\n\n\n\n");
    delay(5000);
  }
}

void write_color_to_neopixel(int color_int, int led_pin, int intensity) {
  if (color_int == 0) {
    strip.setPixelColor(led_pin, strip.Color(intensity, 0, 0));
  } else if (color_int == 1) {
    strip.setPixelColor(led_pin, strip.Color(0, intensity, 0));
  } else {
    strip.setPixelColor(led_pin, strip.Color(0, 0, intensity));
  }
  strip.show();
}

void write_color_to_rgb_led(int color_int, int redpin, int greenpin, int bluepin, int intensity) {
  //  Serial.print("Writing ");
  //  Serial.println(color_int_to_color_string(color_int));
  if (color_int == 0) {
    analogWrite(redpin, intensity);
    analogWrite(greenpin, 0);
    analogWrite(bluepin, 0);
  } else if (color_int == 1) {
    analogWrite(greenpin, intensity);
    analogWrite(redpin, 0);
    analogWrite(bluepin, 0);
  } else {
    analogWrite(bluepin, intensity);
    analogWrite(redpin, 0);
    analogWrite(greenpin, 0);
  }
}

String color_int_to_color_string(int color_int) {
  if (color_int == 0) {
    return "RED";
  } else if (color_int == 1) {
    return "GREEN";
  } else if (color_int == 2) {
    return "BLUE";
  } else {
    return "INVALID COLOR";
  }
}
