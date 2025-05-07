// pre fing with i2c
#include "ti_msp_dl_config.h"
#include <stdio.h>
#include <ti/driverlib/dl_i2c.h>

#define DISPLAY_RESOLUTION 60 // how much time in ms one LED row takes up. Analogous to Approach Rate in osu (inversel so)
#define WINDOW_OFFSET 4 // this is where the perfect window begins, measured from the bottom of the board
#define FORESIGHT_DISTANCE DISPLAY_RESOLUTION*29 // 29 leds are >0
#define HINDSIGHT_DISTANCE DISPLAY_RESOLUTION*3 // 3 leds are <0 w.r.t realtime
#define MAX_HIT_OBJS 266 // maximum numebr of allowable hitobjects in song. (max across all songs rn is ETA at 266)
#define PRINT_PERIOD 20 // Min period in ms before screen can refresh
#define ACCELEROMETER_LOCKOUT 200 // how many accelerometer inputs are ignored after a large input is detected
#define SENSITIVITY 35000 // how sensitive various parameters are
#define DELAY (3276) // N+1 is period, this should be about a half second
#define TOGGLE_SONG_DELAY 3500000
#define INTER_SONG_DELAY 6000000 // lowk pointless now
#define GLOBAL_OFFSET -1170 //experimentally measured offset parameter to account for external sound chip
uint8_t offTxPacket[] =  {0x92, 0x49, 0x24, 0x92, 0x49, 0x24, 0x92, 0x49, 0x24};
// long ones for double buffer shit

#define LED_COUNT 256
#define PACKET_SIZE 9
#define NUM_OF_SONGS 3 // hardcoding this for demo
uint8_t buffer_switch = 0;
uint8_t txPacket0_ready_flag = 0;
uint8_t txPacket1_ready_flag = 0;

uint8_t txPacket0[LED_COUNT*PACKET_SIZE];
uint8_t txPacket1[LED_COUNT*PACKET_SIZE];

uint8_t row_storage[32];

void initTxPacket0(void) {
    for (int i = 0; i < LED_COUNT; i++) {
        for (int j = 0; j < PACKET_SIZE; j++) {
            txPacket0[i * PACKET_SIZE + j] = offTxPacket[j];
        }
    }
    txPacket0_ready_flag = 0;
}

void initTxPacket1(void) {
    for (int i = 0; i < LED_COUNT; i++) {
        for (int j = 0; j < PACKET_SIZE; j++) {
            txPacket1[i * PACKET_SIZE + j] = offTxPacket[j];
        }
    }
    txPacket1_ready_flag = 0;
}

int time_ms = 0; // this used to be uint16. explains a lot
uint8_t onTxPacket[] =   {0x92, 0x4D, 0xA6, 0x92, 0x49, 0x24, 0x92, 0x4D, 0xA6};

// common TxPackets for beetsaber implementation
uint8_t greenTxPacket[] = {0x92,0x49,0xb4,0x92,0x49,0x24,0x92,0x49,0x24};  // 000600
uint8_t redTxPacket[] = {0x92,0x49,0x24,0x92,0x49,0xb4,0x92,0x49,0x24};    // 060000
uint8_t blueTxPacket[] = {0x92,0x49,0x24,0x92,0x49,0x24,0x92,0x49,0xb4};   // 000006
uint8_t yellowTxPacket[] = {0x92,0x49,0xb4,0x92,0x49,0xb4,0x92,0x49,0x24}; // 060600
uint8_t purpleTxPacket[] = {0x92,0x49,0x24,0x92,0x49,0xb4,0x92,0x49,0xb4}; // 060006

uint8_t dimGreenTxPacket[] = {0x92,0x49,0x26,0x92,0x49,0x24,0x92,0x49,0x24};
uint8_t dimYellowTxPacket[] = {0x92,0x49,0x26,0x92,0x49,0x26,0x92,0x49,0x24};
uint8_t dimRedTxPacket[] = {0x92,0x49,0x24,0x92,0x49,0x26,0x92,0x49,0x24};
uint8_t dimBlueTxPacket[] = {0x92,0x49,0x24,0x92,0x49,0x24,0x92,0x49,0x26};
uint8_t dimGreyTxPacket[] = {0x92,0x49,0x36,0x92,0x49,0x36,0x92,0x49,0x36};

uint8_t *flashTxPacket = offTxPacket;

struct Song { // note that, in the actual game, you wont have much room to print
  // You can trigger playback though?
  const char *artist;
  const char *song_name;
  const char *path_to_song;
  //char diffname[MAX_DIFFNAME_LEN]; unneeded
  //unsigned int song_length; // not obtainable from osu metadata, p sure...
  unsigned int preview_time; // time in ms when the song preview starts
  unsigned short num_notes;
  unsigned int hit_objs[MAX_HIT_OBJS];
}; // size is variable. Realistically, no larger than 100B per song tho (everything besides hit objs)
// for the hit objs: up to 512 objects, each one 4 bytes (int + flags) => 2kB. Total: ~2.1kB/song max

struct Song song2_duvet = {
  .artist = "Boa",
  .song_name = "Duvet",
  .path_to_song = "/Users/henry/Downloads/327-final-proj/oszs/charts/duvet/audio.mp3",//"../..oszs/charts/duvet/audio.mp3",
  .preview_time = 0, // zeroed out since the preview starts really late
  .num_notes = 78,
  .hit_objs = {0x08000776, 0x04000bd1, 0x04000d10, 0x48000eee, 0x480013f0, 0x080017bd, 0x44001e07, 0x04001feb, 0x4400212e, 0x44002312, 0x08002592, 0x44002959, 0x48002a9c, 0x04002f9c, 0x0400317d, 0x0800335f, 0x48003863, 0x44003c26, 0x48003d67, 0x48003fe9, 0x0400464b, 0x4800478d, 0x44004b53, 0x08004dd7, 0x44005235, 0x44005375, 0x08005554, 0x44005a67, 0x48005ba9, 0x480061f3, 0x080065b4, 0x04006a0b, 0x48006bfa, 0x08006fc9, 0x480074de, 0x440078a7, 0x080079e9, 0x0400801d, 0x040082a1, 0x08008525, 0x480088eb, 0x04008f36, 0x48009078, 0x4800943e, 0x08009804, 0x08009a89, 0x4400a088, 0x2000a2f0, 0x0400b734, 0x0800b9b6, 0x4800bd79, 0x4400c279, 0x4400c3ba, 0x4800c63b, 0x4400cb4a, 0x6000cdcc, 0x0800e97b, 0x0400f0f5, 0x4400f38e, 0x4800f4d0, 0x4400f896, 0x0800f9d8, 0x04010022, 0x040102a7, 0x080103e9, 0x480108f1, 0x08010cb7, 0x4801107e, 0x480116c8, 0x44011d13, 0x48011e55, 0x0801221b, 0x04012866, 0x44012aea, 0x48012c26, 0x0801311c, 0x080134e2, 0x600138a8}
  // human playable version below
  //.hit_objs = {0x08000776, 0x04000d10, 0x480013f0, 0x080017bd, 0x44001e07, 0x4400212e, 0x08002592, 0x44002959, 0x04002f9c, 0x0400317d, 0x0800335f, 0x48003863, 0x48003d67, 0x48003fe9, 0x4800478d, 0x08004dd7, 0x44005235, 0x44005375, 0x08005554, 0x44005a67, 0x48005ba9, 0x480061f3, 0x080065b4, 0x04006a0b, 0x48006bfa, 0x08006fc9, 0x480074de, 0x440078a7, 0x080079e9, 0x0400801d, 0x040082a1, 0x08008525, 0x480088eb, 0x04008f36, 0x48009078, 0x4800943e, 0x08009804, 0x08009a89, 0x4400a088, 0x2000a2f0, 0x0400b734, 0x0800b9b6, 0x4800bd79, 0x4400c279, 0x4400c3ba, 0x4800c63b, 0x4400cb4a, 0x6000cdcc, 0x0800e97b, 0x0400f0f5, 0x4400f38e, 0x4800f4d0, 0x4400f896, 0x0800f9d8, 0x04010022, 0x040102a7, 0x080103e9, 0x480108f1, 0x08010cb7, 0x4801107e, 0x480116c8, 0x44011d13, 0x48011e55, 0x0801221b, 0x04012866, 0x44012aea, 0x48012c26, 0x0801311c, 0x080134e2, 0x600138a8}

};

struct Song song4_reality_surf = {
  .artist = "Bladee",
  .song_name = "Reality Surf",
  .path_to_song = "/Users/henry/Downloads/327-final-proj/oszs/charts/reality-surf/audio.mp3",
  .preview_time = 0,
  .num_notes = 193,
  .hit_objs = {0x0400030c, 0x040005fa, 0x040008e8, 0x04000ec4, 0x040011b2, 0x040014a0, 0x04001a7c, 0x04001d6a, 0x04002058, 0x04002634, 0x04002922, 0x04002c10, 0x040031ec, 0x040034da, 0x04003651, 0x040037c8, 0x04003ab6, 0x04003da4, 0x04004092, 0x04004209, 0x04004380, 0x0400466e, 0x0400495c, 0x04004dc1, 0x04004f38, 0x040050af, 0x04005226, 0x04005514, 0x04005979, 0x04005af0, 0x04005dde, 0x04005f55, 0x040060cc, 0x04006531, 0x040066a8, 0x0400681f, 0x04006996, 0x04006c84, 0x04006f72, 0x040070e9, 0x04007260, 0x0400754e, 0x040076c5, 0x0400783c, 0x04007b2a, 0x04007ca1, 0x04007e18, 0x04008106, 0x0400827d, 0x040083f4, 0x040089d0, 0x04008b47, 0x04008fac, 0x0400929a, 0x04009588, 0x040096ff, 0x04009876, 0x04009b64, 0x04009e52, 0x0400a140, 0x0400a2b7, 0x0400a42e, 0x0400a5a5, 0x0400a71c, 0x0400acf8, 0x0400ae6f, 0x0400afe6, 0x0400b2d4, 0x0400b739, 0x0400b8b0, 0x0400ba27, 0x0400bb9e, 0x0400bd15, 0x0400be8c, 0x0400c17a, 0x0400c2f1, 0x0400c756, 0x0400c8cd, 0x0400cbbb, 0x0400cd32, 0x0400cea9, 0x0400d020, 0x0400d30e, 0x0400d485, 0x0400d5fc, 0x0400d8ea, 0x0400dbd8, 0x0400dd4f, 0x0400dec6, 0x0400e03d, 0x0400e1b4, 0x0400e4a2, 0x0400e619, 0x0400e790, 0x0400e907, 0x0400ea7e, 0x0400f05a, 0x0400f348, 0x0400f4bf, 0x0400f636, 0x0400f7ad, 0x0400f924, 0x0400fc12, 0x0400ff00, 0x04010077, 0x040101ee, 0x04010365, 0x040104dc, 0x040107ca, 0x04010ab8, 0x04010c2f, 0x04010da6, 0x04010f1d, 0x04011094, 0x04011382, 0x040114f9, 0x0401195e, 0x04011c4c, 0x040120b1, 0x04012228, 0x04012516, 0x0401268d, 0x04012804, 0x04012de0, 0x04012f57, 0x040130ce, 0x04013245, 0x040133bc, 0x040136aa, 0x04013998, 0x04013b0f, 0x04013c86, 0x04013dfd, 0x04013f74, 0x04014262, 0x040143d9, 0x04014550, 0x040146c7, 0x0401483e, 0x04014b2c, 0x04014e1a, 0x04014f91, 0x04015108, 0x040156e4, 0x040159d2, 0x04015b49, 0x04015cc0, 0x04015fae, 0x0401629c, 0x0401658a, 0x04016878, 0x04016b66, 0x04016e54, 0x04017430, 0x04017a0c, 0x04017cfa, 0x04017fe8, 0x0401815f, 0x040182d6, 0x040185c4, 0x040188b2, 0x04018ba0, 0x04018d17, 0x04018e8e, 0x0401917c, 0x04019758, 0x040198cf, 0x04019a46, 0x04019d34, 0x0401a022, 0x0401a310, 0x0401a487, 0x0401a5fe, 0x0401a775, 0x0401a8ec, 0x0401abda, 0x0401ad51, 0x0401b1b6, 0x0401b4a4, 0x0401b792, 0x0401b9c4, 0x0401bd6e, 0x0401c05c, 0x0401c34a, 0x0401c4c1, 0x0401c926, 0x0401cc14, 0x0401cf02, 0x0401d134, 0x0401d4de, 0x0401d7cc, 0x0401daba, 0x0401dc31}
};

struct Song song5_stupid_horse = {
  .artist = "100 gecs",
  .song_name = "Stupid Horse",
  .path_to_song = "/Users/henry/Downloads/327-final-proj/oszs/charts/stupid-horse/audio.mp3",
  .preview_time = 29778,
  .num_notes = 182,
  .hit_objs = {0x04001410, 0x080014c2, 0x040016da, 0x0800183f, 0x48001c6e, 0x04001e86, 0x04001f39, 0x4800209e, 0x48002368, 0x48002798, 0x440029af, 0x04002a62, 0x08002bc7, 0x48002e91, 0x480032c1, 0x040034d8, 0x0400358b, 0x480036f0, 0x080039ba, 0x08003dea, 0x04004002, 0x440040b4, 0x4400437e, 0x040044e4, 0x44004596, 0x48004649, 0x44004913, 0x04004a79, 0x04004b2b, 0x48004bde, 0x08004ea8, 0x480052d7, 0x4800543c, 0x48005706, 0x440059d1, 0x44005b36, 0x04005c9b, 0x44005e00, 0x48005f65, 0x0400617d, 0x08006230, 0x080064fa, 0x48006a8e, 0x48006bf4, 0x48006d59, 0x44006f70, 0x08007023, 0x08007452, 0x480075b8, 0x040079e7, 0x08007b4c, 0x08007f7c, 0x080080e1, 0x04008510, 0x08008675, 0x48008aa5, 0x48008c0a, 0x04009039, 0x4800919e, 0x080095ce, 0x48009733, 0x04009b62, 0x48009cc8, 0x0800a0f7, 0x0800a25c, 0x0400a68c, 0x0800a7f1, 0x4800ac20, 0x0800ad85, 0x0400b1b5, 0x4800b31a, 0x0800b749, 0x0800b8ae, 0x4400bcde, 0x4800be43, 0x0800c272, 0x0800c3d8, 0x0800c807, 0x0400dcf4, 0x4800dda7, 0x0800dfbe, 0x4400e3ee, 0x4800e553, 0x0400e982, 0x4400eae8, 0x4400edb2, 0x4400f07c, 0x4800f346, 0x2000f776, 0x4401013a, 0x4401029f, 0x44010352, 0x48010404, 0x040106ce, 0x04010833, 0x440108e6, 0x08010998, 0x08010c63, 0x08010dc8, 0x48010f2d, 0x08011092, 0x080111f8, 0x4801135d, 0x44011627, 0x0801178c, 0x080118f1, 0x48011a56, 0x08011bbc, 0x08011d21, 0x08011e86, 0x44012150, 0x480122b5, 0x440124cd, 0x48012580, 0x0801284a, 0x44012a62, 0x08012b14, 0x08012dde, 0x44012ff6, 0x480130a9, 0x48013373, 0x0401358b, 0x4801363d, 0x44013908, 0x44013a6d, 0x44013bd2, 0x44013d37, 0x48013e9c, 0x040140b4, 0x48014166, 0x08014431, 0x480149c5, 0x08014b2a, 0x08014c90, 0x04014ea7, 0x48014f5a, 0x48015389, 0x480154ee, 0x0401591e, 0x08015a83, 0x08015eb2, 0x48016018, 0x04016447, 0x480165ac, 0x080169dc, 0x48016b41, 0x04016f70, 0x480170d5, 0x04017bfe, 0x44017d64, 0x44017e16, 0x08017ec9, 0x04018193, 0x040182f9, 0x440183ab, 0x4801845e, 0x48018728, 0x48018b57, 0x08018cbc, 0x48018f86, 0x44019251, 0x040193b6, 0x0401951b, 0x44019680, 0x080197e5, 0x440199fd, 0x48019ab0, 0x08019d7a, 0x4801a30e, 0x0801a474, 0x4801a5d9, 0x0401a7f0, 0x0801a8a3, 0x0801acd2, 0x0801ae38, 0x0401b267, 0x4801b3cc, 0x4801bef5, 0x0801c325, 0x4801c48a, 0x4401c8b9, 0x4801ca1e}
};

char *artist;
char *song_name;
char *path_to_song;
unsigned int song_length;


// c. The game states
enum Game_States {
  GAME_START,
  SONG_SELECT,
  IN_GAME,
  REPORT_SCREEN,
  EXIT_GAME
};

char get_note_channel(unsigned int note){
  return (note&1<<30)==(1<<30);
}

enum Game_States game_state = GAME_START;

uint8_t *txPacket;

int transmissionComplete = 0; // flag to indicate complete transmission
int idx = 0; // index of packet being transmitted
int repeat = 0; // counts from 0 to n_repeats
int n_repeats = 256; // number of LEDs

int num_cycles = 0;
int shift = 0;

int message_len = sizeof(onTxPacket) / sizeof(onTxPacket[0]);

int db_message_len = sizeof(txPacket1) / sizeof(txPacket1[0]);

int accel_magnitude_global=0;
int x_global;
int y_global;
int z_global;
unsigned int transmissions = 0;
unsigned int debounce_limit = 0;

int input_channel=0;

int song_length_ms=0;

int accel_counter = 0;
int accel_input(){
    if ((accel_counter > 20) && (accel_magnitude_global>32000)){
        accel_counter = 0;
        return 1;
    }else{
        return 0;
    }
}
int carousel_idx_prev = 0;
int carousel_idx = 0;
int debounce_lim = 0;

int directional_accel_input(int numsongs){
    int absx, absy, absz;
    absx = x_global<0? -x_global: x_global;
    absy = y_global<0? -y_global: y_global;
    absz = z_global<0? -z_global: z_global;

    if ((accel_counter > 60)){// !impt: accel_counter is hardcoded. this is debounce tolerance
        if ((y_global>19000)&&(absx<4000)&&(absz<4000)){
            carousel_idx++;
             if (carousel_idx >= numsongs){
                 carousel_idx = 0;
             }
             printf("Right swipe. Carousel value: %d. Y value: %d \n",carousel_idx, y_global);
             accel_counter = 0;
             return 1;
        }
        if ((y_global<-19000)&&(absx<4000)&&(absz<4000)){
            carousel_idx--;
            if (carousel_idx < 0){
              carousel_idx = numsongs-1;
            }
            printf("Left swipe. Y value: %d\n",y_global);
            accel_counter = 0;
            return -1;
        }
        if ((x_global>19000)&&(absy<4000)&&(absz<4000)){ // confirmation
            accel_counter = 0;
            return 2;
        }
    }
    return 0;
}
int directional_inpt;
/*
int accel_input_directional(int cutoff, int numsongs){
    if ((accel_counter > 20) && (accel_magnitude_global>cutoff)){
            accel_counter = 0;
            if (x_global<-600){ // i thiiiink this means swipe right (if the green LED is down on the accelerometer)
                carousel_idx++;
                if (carousel_idx >= numsongs){
                    carousel_idx = 0;
                }
                printf("Right swipe. Carousel value: %d\n",carousel_idx);
            }
            // mirror
            else if (x_global<-6000){ // i thiiiink this means swipe left
                carousel_idx--;
                if (carousel_idx < 0){
                    carousel_idx = numsongs-1;
                }
                printf("Left swipe. Carousel value: %d\n",carousel_idx);
            }
            return 1;
        }else{
            return 0;
        }
}
*/
// screens (start)
uint8_t* idle_screen(int index_in){
    uint8_t tempTxPacket[]= {0x92, 0x49, 0x24, 0x92, 0x49, 0x24, 0x92, 0x49, 0x24};
    int index_mod = index_in % 16;

    // grey border
    if((index_in>=0 && index_in<=8)||(index_in<=255 && index_in>=248)){
        return dimGreyTxPacket;
    }
    if((index_mod==0)||(index_mod==15)||(index_mod==7)||(index_mod==8)){
        return dimGreyTxPacket;
    }

    //blue smiley
    if((index_in==114)||(index_in==117)||(index_in==122)||(index_in==125)||(index_in==130)||(index_in==133)||(index_in==144)||(index_in==151)||(index_in==153)||(index_in==158)||(index_in==162)||(index_in==163)||(index_in==164)||(index_in==165)){
        return blueTxPacket;
    }

    //default
    return offTxPacket;
}

uint8_t* one_screen(int index_in) {
    int index_mod = index_in % 16;

    // grey border
    if ((index_in >= 0 && index_in <= 8) || (index_in >= 248 && index_in <= 255))
        return dimGreyTxPacket;

    if ((index_mod == 0) || (index_mod == 15) || (index_mod == 7) || (index_mod == 8))
        return dimGreyTxPacket;

    // blue positions
    const int blue_indices[] = {51,52,58,61,66,69,75,76,115,116,122,130,131,133,139,140,141};
    const int num_blue_indices = sizeof(blue_indices) / sizeof(blue_indices[0]);

    for (int i = 0; i < num_blue_indices; i++) {
        if (index_in == blue_indices[i]) {
            return blueTxPacket;
        }
    }

    const int red_indices[] = {11,12,13,21,27,28,34,42,43,44,75,76,82,85,90,91,93,98,99,101,106,109,147,149,153,154,155,156,157,163,165,169,170,171,172,173,179,181};
    const int num_red_indices = sizeof(red_indices) /sizeof(red_indices[0]);

    for (int i = 0; i < num_red_indices; i++) {
        if (index_in == red_indices[i]) {
            return redTxPacket;
        }
    }

    const int purple_indices[] = {195,203,204,211,213,220,227,236,243};
    const int num_purple_indices = sizeof(purple_indices) /sizeof(purple_indices[0]);

    for (int i = 0; i < num_purple_indices; i++) {
        if (index_in == purple_indices[i]) {
            return purpleTxPacket;
        }
    }
    // default
    return offTxPacket;
}

uint8_t* two_screen(int index_in) {
    int index_mod = index_in % 16;

    // grey border
    if ((index_in >= 0 && index_in <= 8) || (index_in >= 248 && index_in <= 255))
        return dimGreyTxPacket;

    if ((index_mod == 0) || (index_mod == 15) || (index_mod == 7) || (index_mod == 8))
        return dimGreyTxPacket;

    // blue positions
    const int blue_indices[] = {51,52,58,61,66,69,75,76,115,116,122,130,131,133,139,140,141};
    const int num_blue_indices = sizeof(blue_indices) / sizeof(blue_indices[0]);

    for (int i = 0; i < num_blue_indices; i++) {
        if (index_in == blue_indices[i]) {
            return blueTxPacket;
        }
    }

    const int red_indices[] = {11,12,13,21,27,28,34,42,43,44,75,76,82,85,90,91,93,98,99,101,106,109,147,149,153,154,155,156,157,163,165,169,170,171,172,173,179,181};
    const int num_red_indices = sizeof(red_indices) /sizeof(red_indices[0]);

    for (int i = 0; i < num_red_indices; i++) {
        if (index_in == red_indices[i]) {
            return redTxPacket;
        }
    }

    const int purple_indices[] = {195,196,202,205,210,221,227,235,242,243,244,245};
    const int num_purple_indices = sizeof(purple_indices) /sizeof(purple_indices[0]);

    for (int i = 0; i < num_purple_indices; i++) {
        if (index_in == purple_indices[i]) {
            return purpleTxPacket;
        }
    }
    // default
    return offTxPacket;
}



uint8_t* three_screen(int index_in) {
    int index_mod = index_in % 16;

    // grey border
    if ((index_in >= 0 && index_in <= 8) || (index_in >= 248 && index_in <= 255))
        return dimGreyTxPacket;

    if ((index_mod == 0) || (index_mod == 15) || (index_mod == 7) || (index_mod == 8))
        return dimGreyTxPacket;

    // blue positions
    const int blue_indices[] = {51,52,58,61,66,69,75,76,115,116,122,130,131,133,139,140,141};
    const int num_blue_indices = sizeof(blue_indices) / sizeof(blue_indices[0]);

    for (int i = 0; i < num_blue_indices; i++) {
        if (index_in == blue_indices[i]) {
            return blueTxPacket;
        }
    }

    const int red_indices[] = {11,12,13,21,27,28,34,42,43,44,75,76,82,85,90,91,93,98,99,101,106,109,147,149,153,154,155,156,157,163,165,169,170,171,172,173,179,181};
    const int num_red_indices = sizeof(red_indices) /sizeof(red_indices[0]);

    for (int i = 0; i < num_red_indices; i++) {
        if (index_in == red_indices[i]) {
            return redTxPacket;
        }
    }

    const int purple_indices[] = {195,196,197,205,210,218,219,220,226,237,243,244,245};
    const int num_purple_indices = sizeof(purple_indices) /sizeof(purple_indices[0]);

    for (int i = 0; i < num_purple_indices; i++) {
        if (index_in == purple_indices[i]) {
            return purpleTxPacket;
        }
    }
    // default
    return offTxPacket;
}


uint8_t* four_screen(int index_in){
    int index_mod = index_in % 16;
    // grey border
    if((index_in>=0 && index_in<=8)||(index_in>256 && index_in<255)){
        return dimGreyTxPacket;
    }
    if((index_mod==0)||(index_mod==15)||(index_mod==7)||(index_mod==8)){
        return dimGreyTxPacket;
    }
    //four
    if ((index_mod==106)||(index_mod==109)||(index_mod==114)||(index_mod==117)||(index_mod==122)||(index_mod==125)||(index_mod==130)||(index_mod==133)||(index_mod==138)||(index_mod==139)||(index_mod==140)||(index_mod==141)||(index_mod==146)||(index_mod==157)||(index_mod==162)){
        return blueTxPacket;
    }
    // default
    return offTxPacket;

}

uint8_t* carousel_screen(int index_to_show, int index_in){
    int num = index_to_show+1;
    if ((index_to_show<0)||(index_to_show>2)){
        printf("Input index should be greater than 3. what.\n");
        return offTxPacket;
    }
    switch (num){
        case 1:
            return one_screen(index_in);
            break;
        case 2:
            return two_screen(index_in);
            break;
        case 3:
            //printf("three\n");
            return three_screen(index_in);
            break;
        default:
            printf("Something went wrong with carousel screen calculation\n");
            return offTxPacket;
            break;
    }
}

uint8_t* report_screen(int index_in){
    int index_mod = index_in % 16;
    if((index_mod==2)||(index_mod==5)||(index_mod==10)||(index_mod==13)){
        return dimGreyTxPacket;
    }
    return offTxPacket;
}


// screens (end)

uint8_t* draw_note(int index_in, int column, int y_val, char color){
    // column tells us left (0) or right (1)
    // y_val tells us dist from top of board, i.e. 0 is top 31 is bottom
    int index_mod = index_in%16;
    //printf("index_in:%d",index_in);
    if((index_in>=(y_val*8)) && (index_in<=((y_val+1)*8))){ // if we're in the correct column
        if (column==0){
            if (y_val%2==0){ // if we're in an even row...
                if ((index_mod==5) || (index_mod==6) || (index_mod==7)) // set the left side to color
                    return dimGreyTxPacket;
            }else{
                if ((index_mod==8) || (index_mod==9) || (index_mod==10)) // if we're in an odd row color these ones
                    return dimGreyTxPacket;
            }
        } else if (column==1){

            if (y_val%2==0){ // if we're in an even row...
                if ((index_mod==0) || (index_mod==1) || (index_mod==2))
                    return dimGreyTxPacket;
            }else{
                if ((index_mod==13) || (index_mod==14) || (index_mod==15)) // if we're in an odd row color these ones
                    return dimGreyTxPacket;
            }

        }
    }
    return offTxPacket;
}

uint8_t* tx_comparison_small(uint8_t *txPacket1, uint8_t *txPacket2){
    if(txPacket1==offTxPacket){
        return txPacket2;
    }else if(txPacket2==offTxPacket){
        return txPacket1;
    }

    if(((txPacket1==dimRedTxPacket)||(txPacket1==dimGreenTxPacket)||(txPacket1==dimBlueTxPacket)) && ((txPacket2!=dimRedTxPacket)&&(txPacket2!=dimGreenTxPacket)&&(txPacket2!=dimBlueTxPacket))){
        return txPacket2;
    } else if (((txPacket2==dimRedTxPacket)||(txPacket2==dimGreenTxPacket)||(txPacket2==dimBlueTxPacket)) && ((txPacket1!=dimRedTxPacket)&&(txPacket1!=dimGreenTxPacket)&&(txPacket1!=dimBlueTxPacket))){
        return txPacket1;
    }

}
/*
uint8_t* tx_comparison_big(uint8_t *txPacket1, uint8_t *txPacket2){
    uint8_t tempTxPacket1[LED_COUNT*PACKET_SIZE];
    for (int i = 0; i < LED_COUNT; i++) {
            uint8_t* packet = pick_lights_db(i);  // Only call once per LED
            for (int j = 0; j < PACKET_SIZE; j++) {
                txPacket0[i * PACKET_SIZE + j] = packet[j];
            }
        }

}

*/

uint8_t* regular_screen(int index_in){
    int index_mod = index_in%16;
    // test to change color based on accel
    /*
    uint8_t *tempTxPacket;
    //char color;
    //if ((accel_magnitude_global>30000) &&(accel_counter>40)){
        //printf("accel input is 1\n");
      //  printf("Accel just triggered\n");
        //accel_counter = 0;
        //tempTxPacket = greenTxPacket;
    //}
    //else
      //  tempTxPacket = dimRedTxPacket;
     */
    // two green lines down the middle...
    if ((index_mod==4) || (index_mod==3) || (index_mod==11) || (index_mod==12))
        return flashTxPacket;
    else
        txPacket = offTxPacket;

    // correctness indicators at the bottom (very hardcoded...)
    if ((index_mod==4) || (index_mod==3) || (index_mod==11) || (index_mod==12)){
        if((index_in<=255)&&(index_in>=248)){
            return redTxPacket;
        }
        if((index_in<=247)&&(index_in>=240)){
            return greenTxPacket;
        }
        if((index_in<=239)&&(index_in>=224)){
            return blueTxPacket;
        }
        if((index_in<=223)&&(index_in>=216)){
            return greenTxPacket;
        }
        if((index_in<=215)&&(index_in>=208)){
            return redTxPacket;
        }
    }else if ((index_mod!=4) && (index_mod!=3) && (index_mod!=11) && (index_mod!=12)){
        if((index_in<=255)&&(index_in>=248)){
            return dimRedTxPacket;
        }
        if((index_in<=247)&&(index_in>=240)){
            return dimGreenTxPacket;
        }
        if((index_in<=239)&&(index_in>=224)){
            return dimBlueTxPacket;
        }
        if((index_in<=223)&&(index_in>=216)){
            return dimGreenTxPacket;
        }
        if((index_in<=215)&&(index_in>=208)){
            return dimRedTxPacket;
        }
    }

    // instead of default case being off packet, just make it the notes.
    return offTxPacket;
}

void realtime_feedback(char latest_hit_note){
   // clear until end of screen
  if (latest_hit_note == 4){
    flashTxPacket = blueTxPacket;
  }
  if (latest_hit_note == 2){
    flashTxPacket = greenTxPacket;
  }
  if (latest_hit_note == 1){
    flashTxPacket = redTxPacket;
  }
  if (latest_hit_note == 0){
    flashTxPacket = dimRedTxPacket;
  }
}

// ts is critical but will not look like that
//uint32 get_song_time_ms()
uint32_t song_time_ms = 0; // important: can we get from adafruit instead of this bs

unsigned short forward_index = 0; // index of the next note that will enter the hit window
unsigned short backward_index = 0; // index of the next note that will leave the hit window
unsigned short window_idx; // for iterating through window
short row; // for grid iteration
int forward_time; // time corresponding to forward_idx
int backward_time; // time corresponding to backward_idx
short change = 1; // if the board state should change
unsigned short song_note_cnt;// number of notes in the song
unsigned int *hitobjs;
struct Song *song;
// 3.a.2. Performance Variables
short combo=0;
short maxcombo=0;
short score=0;
short perfects=0;
short goods=0;
short oks=0;
short misses=0;
float accuracy;
char last_hit_note = 127;

unsigned int has_been_hit(unsigned int hitobj){
  return (hitobj & (1<<31)) == 1<<31; // see game_readme_technical
}
unsigned int get_note_time_ms(unsigned int hitobj){
  return (hitobj&0b1111111111111111111); // 19 ones. see game_readme_technical
}

//char pressed; // stores if a keypress was registered from some key (msut define key later)
// 3.c. Calculating metrics
float measure_accuracy(unsigned short song_note_cnt){
  // measures accuracy [0,1]
  // perfects are worth 100, goods 50, oks 25, miss 0
  return (perfects+0.5*goods+0.25*oks)/(song_note_cnt);
}
short measure_score(float accuracy, int maxcombo){
  float total_notes = perfects+goods+oks+misses;
  return (short) (40*(maxcombo/total_notes)+60*accuracy); // 40 from combo, 60 from acc out of 100. god knows nobody getting 100 tho
}

short first_valid_note_idx(short backward_idx, short forward_idx, unsigned int hitobjs[],char channel){
  // find the index of the first valid note. if none, return 1.
  short idx = backward_idx;
  while (idx<forward_idx){
    if (!has_been_hit(hitobjs[idx]) && (get_note_channel(hitobjs[idx])==channel)){
      return idx;
    }
    idx++;
  }
  return -1;
}
short valid_idx;
short timediff;

char perf_good_ok(short timediff){
  if (timediff<=DISPLAY_RESOLUTION){
    return 100; // perfect
  }
  if (timediff<=DISPLAY_RESOLUTION*2){
    return 50; // good
  }
  if (timediff<=DISPLAY_RESOLUTION*3){
    return 25; // ok
  }
  return 0; // shouldn't happen at all
}


char hit_result; // 100, 50, 25, 0
char get_hit_channel(int y_value){ // will need to change this since it uses keyboard inputs
  if (y_value<-17000){
    return 0;
  }
  if (y_value>17000){
    return 1;
  }
  // we shouldn't get here...
  return 2;
}
void mark_note_as_hit(unsigned int hitobjs[],short idx){
  hitobjs[idx] = hitobjs[idx] | 0x1<<31; // set 31st bit to 1
}

int cnum = 0;

uint8_t* compare_multiple_notes(int index_in){
    uint8_t* tempTxPacket = offTxPacket;
    for (int i = 0; i<cnum; i++){
        // need to replace 2nd arg with some kind of getcolumn function
        tempTxPacket = tx_comparison_small(tempTxPacket,draw_note(index_in, get_note_channel(hitobjs[backward_index+i]), row_storage[i], 0));
    }
    return tx_comparison_small(tempTxPacket,regular_screen(index_in));
}




// double buffer
uint8_t* pick_lights_db(int index_in) {
    switch(game_state){
        case GAME_START:
            return idle_screen(index_in);
            break;
        case SONG_SELECT:
            //regular_screen(index_in);
            return carousel_screen(carousel_idx,index_in);
            break;
        case IN_GAME:
            //printf("time_ms mod 32: %d. (time_ms/1920)%32: %d \n",(time_ms%32),(time_ms/60)%32);
            //return draw_note(index_in, 0, (time_ms/DISPLAY_RESOLUTION)%32,0);
            return compare_multiple_notes(index_in); // !impt: changed to %32*60 = 1920
        case REPORT_SCREEN:
            return report_screen(index_in);
        default:
            break;
    }
}



void writeTxPacket0(){
    txPacket0_ready_flag = 0;
    for (int i = 0; i < LED_COUNT; i++) {
        uint8_t* packet = pick_lights_db(i);  // Only call once per LED
        for (int j = 0; j < PACKET_SIZE; j++) {
            txPacket0[i * PACKET_SIZE + j] = packet[j];
        }
    }
    txPacket0_ready_flag = 1;
}

void writeTxPacket1(){
    txPacket1_ready_flag = 0;
    for (int i = 0; i < LED_COUNT; i++) {
        uint8_t* packet = pick_lights_db(i);  // Same fix
        for (int j = 0; j < PACKET_SIZE; j++) {
            txPacket1[i * PACKET_SIZE + j] = packet[j];
        }
    }
    txPacket1_ready_flag = 1;
}

void handle_delay(int max_cycles){
    num_cycles = (num_cycles<max_cycles) ? num_cycles + 1:0;
    if (num_cycles == (max_cycles-1))
        shift = (shift < 31) ? shift + 1 : 0;
}

/* Maximum size of TX packet */
#define I2C_TX_MAX_PACKET_SIZE (16)

/* Number of bytes to send to target device */
#define I2C_TX_PACKET_SIZE (16)

/* Maximum size of RX packet */
#define I2C_RX_MAX_PACKET_SIZE (16)

/* Number of bytes to received from target */
#define I2C_RX_PACKET_SIZE (16)

/* I2C Target address */
#define I2C_TARGET_ADDRESS (0x19)

/* Data sent to the Target */
// clone (not needed?)
uint8_t gTxPacket[I2C_TX_MAX_PACKET_SIZE] = {0x00, 0x01, 0x02, 0x03, 0x04,
    0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};

/* Counters for TX length and bytes sent */
uint32_t gTxLen, gTxCount;

/* Data received from Target */
uint8_t gRxPacket[I2C_RX_MAX_PACKET_SIZE];
/* Counters for TX length and bytes sent */
uint32_t gRxLen, gRxCount;
/* Indicates status of I2C */
enum I2cControllerStatus {
    I2C_STATUS_IDLE = 0,
    I2C_STATUS_TX_STARTED,
    I2C_STATUS_TX_INPROGRESS,
    I2C_STATUS_TX_COMPLETE,
    I2C_STATUS_RX_STARTED,
    I2C_STATUS_RX_INPROGRESS,
    I2C_STATUS_RX_COMPLETE,
    I2C_STATUS_ERROR,
} gI2cControllerStatus;

// from datasheet
#define LSM303AGR_ACCEL_ADDR  0x19

// Register addresses (from datasheet)
#define CTRL_REG1_A           0x20
#define OUT_X_L_A             0x28  // Must OR with 0x80 for auto-increment
#define OUT_X_H_A             0x29
int sampleindex = 0;
// Power stuff
char left_or_right = 1; // selects whether left or right GPIO should be turned on
void switch_power(){
    if (left_or_right==0){
        DL_GPIO_clearPins(PWR_PORT, PWR_PIN_0_PIN);
        DL_GPIO_setPins(PWR_PORT, PWR_PIN_1_PIN);
        left_or_right=1;
    }
    else{
        DL_GPIO_clearPins(PWR_PORT, PWR_PIN_1_PIN);
        DL_GPIO_setPins(PWR_PORT, PWR_PIN_0_PIN);
        left_or_right=0;
    }
    //!impt: printf("New: %d\n",left_or_right);
}

void accel_init() {
    gTxPacket[0] = CTRL_REG1_A;  // Register address
    gTxPacket[1] = 0x57;         // 0b01010111 = 100Hz, all axes enabled, normal mode
    gTxLen = 2;
    // clone
    gTxCount = DL_I2C_fillControllerTXFIFO(I2C_INST, gTxPacket, gTxLen);
    // clone
    DL_I2C_disableInterrupt(I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
    // clone
    gI2cControllerStatus = I2C_STATUS_TX_STARTED;

    //printf("Starting transfer\n");
    // clone
    DL_I2C_startControllerTransfer(I2C_INST, LSM303AGR_ACCEL_ADDR,
                                   DL_I2C_CONTROLLER_DIRECTION_TX, gTxLen);
    // clone
    while (gI2cControllerStatus != I2C_STATUS_TX_COMPLETE &&
           gI2cControllerStatus != I2C_STATUS_ERROR)
        __WFE();

}
int overall_magnitude(int16_t x, int16_t y, int16_t z){
    // sum of abs values
    int16_t x1,y1,z1;
    x1 = x<0? -x:x;
    y1 = y<0? -y:y;
    z1 = z<0? -z:z;
    return x1+y1+z1;
}

// for playing sounds
int prev_song_idx = 0;
void play_song(int songidx){
     // want lag only on the start of clearPins
       if (songidx == 0){
           DL_GPIO_clearPins(SONGS_PORT, SONGS_ONE_PIN);
           delay_cycles(TOGGLE_SONG_DELAY);
           DL_GPIO_setPins(SONGS_PORT, SONGS_ONE_PIN);
       }
       if (songidx ==1){
          DL_GPIO_clearPins(SONGS_PORT, SONGS_TWO_PIN);
          delay_cycles(TOGGLE_SONG_DELAY);
          DL_GPIO_setPins(SONGS_PORT, SONGS_TWO_PIN);
      }
       if (songidx ==2){
         DL_GPIO_clearPins(SONGS_PORT, SONGS_THREE_PIN);
         delay_cycles(TOGGLE_SONG_DELAY);
         DL_GPIO_setPins(SONGS_PORT, SONGS_THREE_PIN);
     }
       prev_song_idx=songidx;

}
void pause_song(int songidx){
     // want pretty high start lag and end lag
    if (songidx!=-1){
       delay_cycles(INTER_SONG_DELAY);
       if (songidx == 0){
           DL_GPIO_clearPins(SONGS_PORT, SONGS_ONE_PIN);
           delay_cycles(TOGGLE_SONG_DELAY);
           DL_GPIO_setPins(SONGS_PORT, SONGS_ONE_PIN);
       }
       if (songidx ==1){
          DL_GPIO_clearPins(SONGS_PORT, SONGS_TWO_PIN);
          delay_cycles(TOGGLE_SONG_DELAY);
          DL_GPIO_setPins(SONGS_PORT, SONGS_TWO_PIN);
       }
       if (songidx ==2){
         DL_GPIO_clearPins(SONGS_PORT, SONGS_THREE_PIN);
         delay_cycles(TOGGLE_SONG_DELAY);
         DL_GPIO_setPins(SONGS_PORT, SONGS_THREE_PIN);
       }
       delay_cycles(5*TOGGLE_SONG_DELAY); // hgih end lag

    }
}
void change_to_new_song(char oldsongidx, char newsongidx){ // should only use prev_idx and current_idx as inputs to this
    // first, clear old song
    pause_song(oldsongidx); // error handling happens in pause song. kind of not good practice...
    // then, set new song
    play_song(newsongidx);
    // set indexes accordingly
}


//!impt: for debouncing
int next_allowable_trigger; // sets lockout period

// for timer switching
char switch_flag = 1;

void increment_if_possible(uint32_t tima_val){
       if (tima_val<327){ // !impt
           if (switch_flag==1){
               time_ms+=20; //!impt: this assumes the repeat period is 4ms. so does the 65 number
               switch_flag=0;
           }
       }
       switch_flag = tima_val>=327 ? 1: switch_flag;
}



int main(void)
{
    // hello...
    initTxPacket0();
    initTxPacket1();


    // initialization + configuration of timer module
    SYSCFG_DL_TIMER_1_init();
    DL_TimerG_setLoadValue(TIMER_1_INST, 0xFFFFFFFF);  // Optional, for full-range
    DL_TimerG_setTimerCount(TIMER_1_INST, 0);     // Reset counter to 0
    DL_TimerG_startCounter(TIMER_1_INST);         // Start counting


    //troubleshooting timer module
    //bool isRunning = DL_TimerG_isRunning(TIMER_1_INST);
    //bool clken = DL_TimerG_isClockEnabled(TIMER_1_INST);
    //uint32_t load_value = DL_TimerG_getLoadValue(TIMER_1_INST);
    //printf("Load_value: %d, isRunning: %d, clken: %d\n",
           //load_value, isRunning, clken);
    // Start the timer
    //DL_TimerG_startCounter(TIMER_1_INST);
    //clken = DL_TimerG_isClockEnabled(TIMER_1_INST);
    //uint32_t ticks = TIMG0->COUNTERREGS.CTR;
    //uint32_t elapsed_ms = (ticks1 * 1000) / 32768;


    struct Song songlist[] = {song2_duvet, song4_reality_surf, song5_stupid_horse};
    int songlist_len = sizeof(songlist)/sizeof(songlist[0]);

    txPacket = offTxPacket;

    SYSCFG_DL_init();

    // clone
    NVIC_EnableIRQ(I2C_INST_INT_IRQN);
    //DL_I2C_enableController(I2C_0_INST);
    // check
    printf("Forcing interrupt\n");
    NVIC_SetPendingIRQ(I2C_INST_INT_IRQN);

    /* Set LED to indicate start of transfer */
    //DL_GPIO_setPins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);

    DL_SYSCTL_disableSleepOnExit();
    //DL_GPIO_clearPins(PWR_PORT, PWR_PIN_0_PIN);
    //DL_GPIO_setPins(PWR_PORT, PWR_PIN_0_PIN);

    // clone
    gI2cControllerStatus = I2C_STATUS_IDLE;

    gTxLen = I2C_TX_PACKET_SIZE;
    /*
     * Fill the FIFO
     *  The FIFO is 8-bytes deep, and this function will return number
     *  of bytes written to FIFO */
    accel_init();
    //printf("Initialized accel\n");

    // clone
    gTxCount = DL_I2C_fillControllerTXFIFO(I2C_INST, &gTxPacket[0], gTxLen);
    /* Enable TXFIFO trigger interrupt if there are more bytes to send */

    // clone
    if (gTxCount < gTxLen) {
        DL_I2C_enableInterrupt(
            I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
    } else {
        DL_I2C_disableInterrupt(
            I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
    }

    /*
     * Send the packet to the controller.
     * This function will send Start + Stop automatically.
     */
    // clone
    gI2cControllerStatus = I2C_STATUS_TX_STARTED;
    // clone
    while (!(
        DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE))
        ;
    DL_I2C_startControllerTransfer(
        I2C_INST, I2C_TARGET_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_TX, gTxLen);

    //printf("Values in 0x19:\n ");
    /* Wait until the Controller sends all bytes */
    // clone
    while ((gI2cControllerStatus != I2C_STATUS_TX_COMPLETE) &&
           (gI2cControllerStatus != I2C_STATUS_ERROR)) {
        __WFE();
    }
    // clone
    while (DL_I2C_getControllerStatus(I2C_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;

    // clone
    /* Trap if there was an error */
    delay_cycles(2000);
    if (DL_I2C_getControllerStatus(I2C_INST) &
        DL_I2C_CONTROLLER_STATUS_ERROR) {
        /* LED will remain high if there is an error */
        __BKPT(0);
    }
    //printf("Accelerometer could be initialized\n");
    // clone
    while (!(
        DL_I2C_getControllerStatus(I2C_INST) & DL_I2C_CONTROLLER_STATUS_IDLE))
        ;

    /* Add delay between transfers */
    delay_cycles(1000);

    /* Send a read request to Target */
    gRxLen               = I2C_RX_PACKET_SIZE;
    gRxCount             = 0;

    gI2cControllerStatus = I2C_STATUS_RX_STARTED;
    // clone
    DL_I2C_startControllerTransfer(
        I2C_INST, I2C_TARGET_ADDRESS, DL_I2C_CONTROLLER_DIRECTION_RX, gRxLen);
    //printf("Received bytes\n");
    /* Wait for all bytes to be received in interrupt */
    while (gI2cControllerStatus != I2C_STATUS_RX_COMPLETE) {
        __WFE();
    }
    // clone
    while (DL_I2C_getControllerStatus(I2C_INST) &
           DL_I2C_CONTROLLER_STATUS_BUSY_BUS)
        ;
    /* If write and read were successful, toggle LED */
    int16_t accel_data_l[6];
    int16_t accel_data_r[6];
    int16_t x;
    int16_t y;
    int16_t z;
    // !impt: will have to probably switch off of delay cycles as the means to poll accelerometer
    while (1) {



        uint32_t ticks = TIMG0->COUNTERREGS.CTR;
        //time_ms = (ticks * 1000) / 32768;
        increment_if_possible(ticks);
        //printf("time_ms:%d\n",time_ms);
        accel_counter = accel_counter<10000000 ? accel_counter+1: 10000; // should be good for many hours
        //switch_power();

        //delay_cycles(50000);
        switch (game_state){
        case GAME_START:
            if (accel_input()==1){
                game_state = SONG_SELECT;
                play_song(carousel_idx);
                printf("We went to song select. Now playing song at idx 0 i hope\n");

            }
            break;
        case SONG_SELECT:
            // !impt: this code assumes that nothing is playing before the song starts. this must be the case
            directional_inpt = directional_accel_input(songlist_len);
            if ((directional_inpt==1)||(directional_inpt==-1)){
                change_to_new_song(prev_song_idx,carousel_idx);
            }
            // what happens here?
            /*
             * 1. There is a default song that plays
             * 2. Can press buttons to move left or right in the carousel (or swipe accelerometer)
             *
             * 3. Every time a button press is registered, trigger indefinite playback for that song
             * 4. Press a confirm button (or swipe down) to select a song
             *
             * Incoming problems:
             *  1. debounce for button press
             */
            //!impt: drag down hard to
            if (directional_inpt==2){
                //printf("Went to game. Song picked : %d\n", carousel_idx);
                game_state = IN_GAME;
                forward_index = 0; // index of the next note that will enter the hit window
                backward_index = 0; // index of the next note that will leave the hit window

                song = &songlist[carousel_idx];
                hitobjs = song->hit_objs;
                song_note_cnt = song->num_notes;

                song_length_ms = get_note_time_ms(hitobjs[song_note_cnt-1]);
                //printf("song_length_ms: %d\n", song_length_ms);
                forward_time = get_note_time_ms(hitobjs[forward_index]);
                //printf("hitobj %x, forward_time %d", hitobjs[forward_index], forward_time);
                backward_time = get_note_time_ms(hitobjs[backward_index]);
                // here you stop and restart the song!!!
                pause_song(carousel_idx);
                play_song(carousel_idx);
                time_ms=GLOBAL_OFFSET;


            }

             break;
        case IN_GAME:
            printf("timems %d\n",time_ms);
            //carousel_idx = 1;


            //printf("num notes %d\n",song_note_cnt);
            //printf("First num: %x\n",hitobjs[0]);

            // initialize/reset game variables
            /*
            forward_index = 0; // index of the next note that will enter the hit window
            backward_index = 0; // index of the next note that will leave the hit window
            forward_time = get_note_time_ms(hitobjs[forward_index]);
            backward_time = get_note_time_ms(hitobjs[backward_index]);
            */

            //printf("First num: %x, num notes: %d, forward_idx: %d \n",hitobjs[0], song_note_cnt, forward_index);
            //!impt:
            //printf("Forward time %d, forward index %d, song number %d, time_ms %d\n",forward_time, forward_index, carousel_idx, time_ms);
            // 2a/5 check if a note just entered the window

            //leave if we've gone longer than the song!
            if(time_ms>= song_length_ms){
                game_state = REPORT_SCREEN;
            }

            if (FORESIGHT_DISTANCE+time_ms>forward_time){
                //printf("triggered\n");
                forward_index +=1;
                if (forward_index == song_note_cnt){
                    forward_time = 0x0FFFFFFF; // set to ~inf; there is no next note
                }
                else{
                    forward_time = get_note_time_ms(hitobjs[forward_index]);
                    //printf("updated %d\n", forward_time);
                }
            //printf("Time: %d | Window %d | Next time: %d |Idx: %d|\n",time_ms,FORESIGHT_DISTANCE,forward_time,forward_index);
            //printf("Forward advance at %d ms with %d ms window. Next time: %d\n. Idx: %d",time_ms,FORESIGHT_DISTANCE,forward_time,forward_index);
            }
            // 2b/5 check if a note just exited the window
            if (backward_time+HINDSIGHT_DISTANCE<time_ms){
                // this means something just left.
                if (has_been_hit(hitobjs[backward_index])==0){ // if hasn't been hit
                    combo = 0;
                    misses+=1;
                    last_hit_note = 0;
                    // no need for real time acc. but if you wanted it:
                    // - reset accuracy
                    // - reset combo
                    // - modify score
                }
                backward_index+=1;
                change = 1;
                if (backward_index == song_note_cnt){
                    backward_time = 0x0FFFFFFF; // set to ~inf; there is no next note
                }
                else{ // !impt: instead of get_note_time_ms which calls miniaudio functions, ...
                    backward_time = get_note_time_ms(hitobjs[backward_index]);
                }
                //printf("Time: %d | Window -%d | Next time: %d |Idx: %d|\n",time_ms,HINDSIGHT_DISTANCE,backward_time,backward_index);
            }
            for (window_idx=backward_index; window_idx<forward_index; window_idx++){
                // first, get the row you're in (note that below implementation freezes time_ms)
                if (!has_been_hit(hitobjs[window_idx])){
                  row = (time_ms+FORESIGHT_DISTANCE-get_note_time_ms(hitobjs[window_idx]))/DISPLAY_RESOLUTION;
                  //draw_note(row,1,get_note_channel(hitobjs[window_idx]));
                  row_storage[window_idx-backward_index] = row;
                  cnum = forward_index-backward_index;
                  //printf("cnum %d\n",cnum);
                }
                        //printf("Row: %d",row);
                        // gng ur not checking if ntoe vanishes
            }

            realtime_feedback(last_hit_note);

            if (accel_input()==1){ // i.e if someone shook controller
                //printf("made it in, ");
                int hitx = x_global;
                int hity = y_global;
                int hitz = z_global;
                //printf("hit X:%d , Y:%d, Z:%d \n", hitx, hity, hitz);
              if (0){ // need to link this to GPIO button input hopefully...
                // see results
                game_state = REPORT_SCREEN;
                // three things to do here...
                // turn off sound
                // terminal back to normal
                // clear grid and such
                //printf("\nRage quit?\n");
                break;
              }
              input_channel = get_hit_channel(hity);
              //printf("input channel: %d", input_channel);
              if (input_channel == 0 || input_channel ==1){ // left or right
                // timing calculation
                // time_ms = get_song_time_ms(&sound);
                valid_idx = first_valid_note_idx(backward_index,forward_index,hitobjs,input_channel);
                if (valid_idx>-1){ // only do anything  if a note being hit happens at a valid time
                  // calculate perfect/good/ok
                  timediff = get_note_time_ms(hitobjs[valid_idx])-time_ms; // could this be chronically delayed?
                  timediff = timediff>-1? timediff: -timediff;// abs value
                  hit_result = perf_good_ok(timediff);
                  // modify score results
                  if (hit_result == 100) {
                    perfects+=1;
                    last_hit_note = 4;
                    //printf("Perfect");
                  }
                  if (hit_result == 50) {
                    goods+=1;
                    last_hit_note = 2;
                    //printf("Good");
                  }
                  if (hit_result == 25) {
                    last_hit_note = 1;
                    oks+=1;
                    //printf("Ok");
                  }
                  // hit too early
                  if (hit_result == 0){
                    last_hit_note = 0;
                    combo = 0;
                    misses+=1; // actually whatever just handle the entire input who caress who cares
                    //printf("Hit result was 0. Note time %d, note %d, hit time %d, index %d \n",get_note_time_ms(hitobjs[valid_idx]),hitobjs[valid_idx],time_ms,valid_idx);
                  //break;
                  }
                  // constant tasks
                  combo+=1;
                  maxcombo = combo>maxcombo ? combo: maxcombo;
                  // mark note as hit
                  mark_note_as_hit(hitobjs,valid_idx);
                }
              }
            }

            break;
            case REPORT_SCREEN:
            break;
        }

        // do pwm and transmit LED data thru SPI!

        if (buffer_switch == 0) {
            writeTxPacket1();  // Prepares the *next* buffer
        } else {
            writeTxPacket0();
        }
        transmissionComplete = 0;
        idx = 1;
        repeat = 0;
        NVIC_ClearPendingIRQ(SPI_0_INST_INT_IRQN);
        NVIC_EnableIRQ(SPI_0_INST_INT_IRQN);
        DL_SPI_transmitData8(SPI_0_INST, *txPacket);
        while (!transmissionComplete)
            __WFI();
        delay_cycles(2000);

        //handle_delay(2);
        /*
        DL_TimerA_setCaptureCompareValue(PWM_0_INST, 4000, DL_TIMER_CC_0_INDEX);
        transmissionComplete = 0;
        idx = 1;
        repeat = 0;
        NVIC_ClearPendingIRQ(SPI_0_INST_INT_IRQN);
        NVIC_EnableIRQ(SPI_0_INST_INT_IRQN);
        DL_SPI_transmitData8(SPI_0_INST, *txPacket);

        while (!transmissionComplete)
            __WFI();

        delay_cycles(2000);
        */

        // accelerometer
        // Request starting at OUT_X_L_A with auto-increment
        // clone
        gTxPacket[0] = OUT_X_L_A | 0x80;
        gTxLen = 1;
        // clone
        gTxCount = DL_I2C_fillControllerTXFIFO(I2C_INST, gTxPacket, gTxLen);
        // clone
        gI2cControllerStatus = I2C_STATUS_TX_STARTED;
        // clone
        DL_I2C_startControllerTransfer(I2C_INST, LSM303AGR_ACCEL_ADDR,
                                       DL_I2C_CONTROLLER_DIRECTION_TX, gTxLen);

        delay_cycles(5000); // need a delay here. poll vs interrupt...
        // clone
        while (gI2cControllerStatus != I2C_STATUS_TX_COMPLETE &&
               gI2cControllerStatus != I2C_STATUS_ERROR)
            __WFE();

        // Read 2 bytes: OUT_X_L_A and OUT_X_H_A
        gRxLen = 6;
        gRxCount = 0;

        // clone
        gI2cControllerStatus = I2C_STATUS_RX_STARTED;
        // clone
        DL_I2C_startControllerTransfer(I2C_INST, LSM303AGR_ACCEL_ADDR,
                                       DL_I2C_CONTROLLER_DIRECTION_RX, gRxLen);
        // clone
        while (gI2cControllerStatus != I2C_STATUS_RX_COMPLETE)
            __WFE();
        //printf("Writing accel data\n");
        for (int i = 0; i < 6; i++) {

            if(left_or_right==0){
                accel_data_l[i] = gRxPacket[i];
            }
            if(left_or_right==1){
                accel_data_r[i] = gRxPacket[i];
            }

        }

        if(left_or_right==0){
            x = (int16_t)((accel_data_l[1] << 8) | accel_data_l[0]);
            y = (int16_t)((accel_data_l[3] << 8) | accel_data_l[2]);
            z = (int16_t)((accel_data_l[5] << 8) | accel_data_l[4]);
            //printf("Left data: X %d, Y %d, Z %d\n",x,y,z);
        }

        if(left_or_right==1){
            x = (int16_t)((accel_data_r[1] << 8) | accel_data_r[0]);
            y = (int16_t)((accel_data_r[3] << 8) | accel_data_r[2]);
            z = (int16_t)((accel_data_r[5] << 8) | accel_data_r[4]);

            //printf("Right data: X %d, Y %d, Z %d\n",x,y,z);
        }
        //printf("X %d, Y %d, Z %d\n", x,y,z);
        accel_magnitude_global = overall_magnitude(x,y,z);
        //printf("Absval: %d\n", accel_magnitude_global);
        transmissions++; // total # of times accel data was read
        //printf("X: %d, Y: %d, Z: %d\n", x,y,z);
        //printf("Absval, (x,y), indx: %d, (%d, %d), %d \n\n", overall_magnitude(x,y,z), x,y,accel_counter);

        x_global = x; // global scoped
        y_global = y;
        z_global = z;
        //printf("Absval zero: %d\n", overall_magnitude(x0,y0,z0));
        // Combine the two bytes (little-endian)
        //int16_t x_raw = (int16_t)((gRxPacket[1] << 8) | gRxPacket[0]);
        //x_accel_data[sample_index++] = x_raw;
        //printf("X[%d] = %d\n", sample_index, x_raw);

        //printf("elapsed ms: %d \n elapsed ticks: %d \n",elapsed_ms,ticks);

        delay_cycles(4000);  // ~10ms delay @16MHz for 200Hz sampling

        // trying to get this shit working
        //TIMG0->COUNTERREGS.LOAD = DELAY; // This will load as soon as timer is enabled
        //TIMG0->COUNTERREGS.CTR = DELAY; // caleb added this during class
        //TIMG0->COUNTERREGS.CTRCTL |= (GPTIMER_CTRCTL_EN_ENABLED);
        //#pragma nounroll
        //while (1){
        //    __WFI();
        //}

    }

}

void I2C_INST_IRQHandler(void)
{
    //printf("IRQ triggered, for 1\n");
    switch (DL_I2C_getPendingInterrupt(I2C_INST)) {
        case DL_I2C_IIDX_CONTROLLER_RX_DONE:
            //printf("One, RX done\n");
            gI2cControllerStatus = I2C_STATUS_RX_COMPLETE;
            break;
        case DL_I2C_IIDX_CONTROLLER_TX_DONE:
            //printf("One, TX done\n");
            DL_I2C_disableInterrupt(
                I2C_INST, DL_I2C_INTERRUPT_CONTROLLER_TXFIFO_TRIGGER);
            gI2cControllerStatus = I2C_STATUS_TX_COMPLETE;
            break;
        case DL_I2C_IIDX_CONTROLLER_RXFIFO_TRIGGER:
            gI2cControllerStatus = I2C_STATUS_RX_INPROGRESS;
            /* Receive all bytes from target */
            while (DL_I2C_isControllerRXFIFOEmpty(I2C_INST) != true) {
                if (gRxCount < gRxLen) {
                    gRxPacket[gRxCount++] =
                        DL_I2C_receiveControllerData(I2C_INST);
                } else {
                    /* Ignore and remove from FIFO if the buffer is full */
                    DL_I2C_receiveControllerData(I2C_INST);
                }
            }
            //printf("Triggered\n");
            //memcpy(accelData, gRxPacket, gRxLen);
            break;
        case DL_I2C_IIDX_CONTROLLER_TXFIFO_TRIGGER:
            gI2cControllerStatus = I2C_STATUS_TX_INPROGRESS;
            /* Fill TX FIFO with next bytes to send */
            if (gTxCount < gTxLen) {
                gTxCount += DL_I2C_fillControllerTXFIFO(
                    I2C_INST, &gTxPacket[gTxCount], gTxLen - gTxCount);
            }

            break;
            /* Not used for this example */
        case DL_I2C_IIDX_CONTROLLER_ARBITRATION_LOST:
        case DL_I2C_IIDX_CONTROLLER_NACK:
            if ((gI2cControllerStatus == I2C_STATUS_RX_STARTED) ||
                (gI2cControllerStatus == I2C_STATUS_TX_STARTED)) {
                /* NACK interrupt if I2C Target is disconnected */
                gI2cControllerStatus = I2C_STATUS_ERROR;
            }
        case DL_I2C_IIDX_CONTROLLER_RXFIFO_FULL:
        case DL_I2C_IIDX_CONTROLLER_TXFIFO_EMPTY:
        case DL_I2C_IIDX_CONTROLLER_START:
        case DL_I2C_IIDX_CONTROLLER_STOP:
        case DL_I2C_IIDX_CONTROLLER_EVENT1_DMA_DONE:
        case DL_I2C_IIDX_CONTROLLER_EVENT2_DMA_DONE:
        default:
            break;
    }
}


// double buffer ISR
void SPI_0_INST_IRQHandler(void)
{
    switch (DL_SPI_getPendingInterrupt(SPI_0_INST)) {
        case DL_SPI_IIDX_TX:
            if(buffer_switch==0){ // does txpacket 0 or 1 need to be sent
                DL_SPI_transmitData8(SPI_0_INST, txPacket0[idx]);
            } else {
                DL_SPI_transmitData8(SPI_0_INST, txPacket1[idx]);
            }
            idx++;
            if (idx == db_message_len) {
                //printf("\nidx = message len. Values: txpacket1flag = %d, txpacket0flag = %d, buffer switch: %d\n",txPacket1_ready_flag,txPacket0_ready_flag,buffer_switch);
                idx = 0;


                if (buffer_switch==0){
                    buffer_switch = 1;
                }else if(buffer_switch==1){
                    buffer_switch = 0;
                }
                //repeat = repeat + 1;
                //pick_lights(repeat);
                // select lights based on repeat in [0,255]
                //if (repeat == n_repeats)
                transmissionComplete = 1;
                NVIC_DisableIRQ(SPI_0_INST_INT_IRQN);
            }
            break;
        default:
            break;
    }
}



// old ISR

/*(
void SPI_0_INST_IRQHandler(void)
{
    switch (DL_SPI_getPendingInterrupt(SPI_0_INST)) {
        case DL_SPI_IIDX_TX:
            DL_SPI_transmitData8(SPI_0_INST, txPacket[idx]);
            idx++;
            if (idx == message_len) {
                idx = 0;
                repeat = repeat + 1;
                pick_lights(repeat);
                // select lights based on repeat in [0,255]
                if (repeat == n_repeats) {
                    transmissionComplete = 1;
                    NVIC_DisableIRQ(SPI_0_INST_INT_IRQN);
                }
            }
            break;
        default:
            break;
    }
}
*/




void TIMG0_IRQHandler(void)
{
    switch (TIMG0->CPU_INT.IIDX) {
        case GPTIMER_CPU_INT_IIDX_STAT_Z: // Counted down to zero event.
            // If we wanted to execute code in the ISR, it would go here.
            break;
        default:
            break;
    }
}



