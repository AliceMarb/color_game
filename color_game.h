/*
 * Type (enum and struct) definitions for state
 */
typedef enum {
  sWAIT_FOR_BLOCK = 1,
  sGAME_FINISHED = 2,
} state;

/*
 * Type for returning new_matches and non_matches
 */
typedef struct {
  uint16_t red;
  uint16_t green;
  uint16_t blue;
} sensor_colors;

/*
 * For testing
 */
const int num_sensors = 6;
const int total_blocks = 6;
void test_sensors();
void test_leds();
void test_music_player();
 
/*
 * INPUT VARIABLES
 */
// Array of 6 integers, where each integer is a value 0-2, representing if the square on the board is red (0), green (1), or blue (2)
int colors[num_sensors];
// Array of 6 integers, where each integer is either 0 or 1, representing if the square on the board has had a matching block placed in it.
int status_tracker[num_sensors];
// Is the game end music playing
boolean music_playing;
// Array of integers, where each integer is the index of a square that has a newly matching block (as of current loop)
int new_matches[num_sensors] = {};
int new_match_size = 0;
// Array of integers, where each integer is the index of a square that has a non-matching block (not necessarily new)
int non_matches[num_sensors] = {};
int non_match_size = 0;


/*
 * Setup function: initialize input variables, set up LED pins and sensor pins
 */
void initialize_system();

/*
 * Read in inputs from sensors and update corresponding variables
 */
void update_inputs();

/*
 * Helper function definitions for functions used in the FSM
 */
void randomize_colors();
void reset_board();
void reset_block_info();
void display_board();
void play_matches();
void play_non_matches();
void play_finished_music();

state update_fsm(state cur_state, long mils, boolean all_blocks_placed, int num_blocks_placed);
