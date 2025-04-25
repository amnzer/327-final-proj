#include <stdio.h>
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include <unistd.h>
/*
Beet Saber Proof-of-Concept
*/

// 1. Define Key Objects
// a. The song class
struct Song { // note that, in the actual game, you wont have much room to print
  // You can trigger playback though?
  const char *artist;
  const char *song_name;
  const char *path_to_song;
  //char diffname[MAX_DIFFNAME_LEN]; unneeded
  //unsigned int song_length; // not obtainable from osu metadata, p sure...
  unsigned int preview_time; // time in ms when the song preview starts
}; // size is variable. Realistically, no larger than 100B per song tho (everything besides hit objs)
// for the hit objs: up to 512 objects, each one 4 bytes (int + flags) => 2kB. Total: ~2.1kB/song max

struct Song song1_eta = {
    .artist ="NewJeans",
    .song_name = "ETA",
    .path_to_song = "/Users/donu/Desktop/S25/ELEC 327/327-final-proj/oszs/charts/ETA/audio.mp3",//"../../oszs/charts/ETA/audio.mp3",
    .preview_time = 49205
  }; 

struct Song song2_duvet = {
  .artist = "Boa",
  .song_name = "Duvet",
  .path_to_song = "/Users/donu/Desktop/S25/ELEC 327/327-final-proj/oszs/charts/duvet/audio.mp3",//"../..oszs/charts/duvet/audio.mp3",
  .preview_time = 0 // zeroed out since the preview starts really late
};
struct Song song3_killing_my_love = {
  .artist = "Leslie Parrish",
  .song_name = "Killing My Love",
  .path_to_song = "/Users/donu/Desktop/S25/ELEC 327/327-final-proj/oszs/charts/killing-my-love/audio.mp3",//"../..oszs/charts/duvet/audio.mp3",
  .preview_time = 76263
};
// initialize all of these things
char *artist;
char *song_name;
char *path_to_song;
unsigned int song_length;


// b. The game states

enum Game_States {
  GAME_START,
  SONG_SELECT,
  IN_GAME,
  REPORT_SCREEN,
  EXIT_GAME
};

enum Game_States game_state = GAME_START; // initialize game state
// c. Special Strings
// Welcome message. Sources: 
// https://patorjk.com/software/taag/
// https://tomeko.net/online_tools/cpp_text_escape.php?
char welcome_msg[] = "  _      __    __                     __         ___          __    ____     __             __\n | | /| / /__ / /______  __ _  ___   / /____    / _ )___ ___ / /_  / __/__ _/ /  ___ ____  / /\n | |/ |/ / -_) / __/ _ \\/  ' \\/ -_) / __/ _ \\  / _  / -_) -_) __/ _\\ \\/ _ `/ _ \\/ -_) __/ /_/ \n |__/|__/\\__/_/\\__/\\___/_/_/_/\\__/  \\__/\\___/ /____/\\__/\\__/\\__/ /___/\\_,_/_.__/\\__/_/   (_) \n\n";

// d. Misc Variables
char user_input = '\0'; // tracks user input
int prev_carousel_idx = 0; 
int carousel_idx = 0;
// e. Helper functions for strings
void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n') { } // read until you hit the end
}
// 2. Key Functions
// a. Playback
unsigned short song_initialized = 0; //true if song is playing

unsigned short conditional_uninit(ma_sound *ma_sownd){
  if (song_initialized){
    ma_sound_stop(ma_sownd);
    ma_sound_uninit(ma_sownd);
    printf("Stopped and uninit\n");
    usleep(360000); // !impt: delay in us btwn songs. Without this you get artifacting.
    song_initialized = 0;
    return 0; 
  }
  return 1; // if this is true, already uninit
}
void load_song(int idx, struct Song song_array[], ma_engine *ma_enjine, ma_sound *ma_sownd){
  // clear old song
  int double_uninit = conditional_uninit(ma_sownd);
  // get metadata
  const char *songname = song_array[idx].song_name;
  const char *artist = song_array[idx].artist;
  unsigned int prevyew_time = song_array[idx].preview_time;
  const char *path = song_array[idx].path_to_song;
  // play the song
  // Load sound from file (but don’t auto-start)
  if (ma_sound_init_from_file(ma_enjine, path, 0, NULL, NULL,ma_sownd) != MA_SUCCESS){
    printf("oop!!\n");
    return;
  }
  // start playback at preview time
  // // part 1: algebra
  float time_to_go_to = prevyew_time/1000.0; // preview time is in ms
  ma_uint64 sr = ma_engine_get_sample_rate(ma_enjine); // sample rate. calculated per song?
  ma_uint64 frameToSeek = (ma_uint64)(time_to_go_to * sr);
  // // Part a: Start playback
  if (ma_sound_seek_to_pcm_frame(ma_sownd, frameToSeek) != MA_SUCCESS) {
    printf("Couldn't find preview time... !\n");
  }
  usleep(80000);
  printf("Now playing: %s by %s\n", songname, artist);
  // if all is well...
  ma_sound_start(ma_sownd);
  printf("Song initted\n");
  song_initialized=1;
  printf("Sound started yippee\n");
}
// The Game
int main(){
  // Declare song list
  struct Song songlist[] = {song1_eta, song2_duvet, song3_killing_my_love};
  int songlist_len = sizeof(songlist)/sizeof(songlist[0]);
  // Set up miniaudio
  ma_sound sound;
  ma_engine engine;
  ma_result result;

  result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    return result;  // Failed to initialize the engine.
  }
  // Main game loop
  while (1){
    /*Phase 1: Game start
        User is greeted with a welcome message.
        User can proceed to SONG_SELECT, or EXIT_GAME.
    */
    if (game_state == GAME_START){
      printf("%s",welcome_msg);
      printf("Enter e to continue! Or, enter x to exit...\n");
      while (1){ // while loop about responding to a specific button
        // get a key
        user_input = getchar();
        clear_input_buffer();
        //
        if(user_input == 'e'){
          game_state = SONG_SELECT;
          printf("Going to song select...\n\n");
          load_song(0,songlist,&engine, &sound); // should be random, not zero
          break;
        }
        else if (user_input == 'x'){
          game_state = EXIT_GAME;
          break;
        }
        else{
          printf("Not e, and not x...\n");
        }
      }
    }
    /* Game State 2:  Song Select
          User is given a carousel of songs to pick from. User can hover over any song
          User hears the song, starting at a highlighted section, and sees the name of the song.
    */
    // How this might work on the MSP:
    // 1. Fetch random start song
    // // Keep track of millisecond time since song start, then get left 3 bits
    // // Use an LFSR and XOR it with a floating number (e.g. TIMA0->VALUE)
    // 2. Play random start song
    // // Find out if you can play starting at a specific time in ms using the audio chip
    if (game_state == SONG_SELECT){
      printf("Welcome to song select! Enter A and D to scroll through songs. Enter space to confirm\n");
      while (1){ // Carousel
        // get a key
        printf("Carousel idx: %d\n",carousel_idx);
        user_input = getchar();
        clear_input_buffer();
        // If key was A, go left
        if (user_input=='a'){
          // clear song
          // go left
          carousel_idx--; 
          // wrap around to end if needed
          if (carousel_idx<0){
            carousel_idx = songlist_len-1; 
          }
          // play song 
          load_song(carousel_idx,songlist,&engine, &sound); 
        }
        // If key was D, go right
        else if (user_input=='d'){
          // go right
          carousel_idx++;
          // wrap around to start if needed
          if (carousel_idx==songlist_len){ 
            carousel_idx=0;
          }
          // play song 
          load_song(carousel_idx,songlist,&engine, &sound); 
        }
        // If key was space, confirm selection
        else if (user_input==' '){ // confirm
          // clear song
          conditional_uninit(&sound);
          game_state = IN_GAME;
          break;
        }
        // If none of the above
        else{
          printf("Invalid input bruh\n");
        }
      }
    }
    /* State 3: Gameplay (the meat)

    */
    if (game_state == IN_GAME){
      printf("Welcome to the game!\n");
      break;
    }
    // Game State 5: Exit Game
    if (game_state == EXIT_GAME){
      printf("Thanks for playing!\n\n");
      break;
    }
    else{
      printf("No mans land...\n\n");
      break;
    }
  }
  // clear sound and engine
  conditional_uninit(&sound);
  ma_engine_uninit(&engine);
  // goodbye
  return 0;
}