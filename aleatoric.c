//simon parker
#include <stdlib.h>
#include <stdio.h>
#include <portaudio.h>
#include <math.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define SAMPLE_RATE 48000
#define SAMPLE_BITS 16
#define MAX_INT16 32767
#define AMPLITUDE 0.25
#define NUM_CHANNELS 1

float duration;

bool base_flag = false;

char song_structures[3][6] = {{0,0,1,1,2,2}, {0,1,0,1,2,3}, {0,1,2,3,3,3}};

int line_structures[10][4] = { //positive major, negative minor
  {1,4,-2,5},
  {1,-6,-2,5},
  {1,-3,4,-4},
  {1,5,-2,5},
  {1,-6,4,5},
  {4,1,-6,4},
  {1,5,6,-1},
  {1,4,-4,1},
  {4,5,1,1},
  {-6,4,1,5},
};

int line_choices[4] = {-1, -1, -1, -1}; //index 0 is the choice for A, 1 for B, etc. stored values are indices into line_structures


float base_keys_hz[13] = { 
  220.,
  233.08,
  246.94,
  261.63,
  277.18,
  293.66,
  311.13,
  329.63,
  349.23,
  369.99,
  392.00,
  415.30,
  440.
};

//adds arr2 to arr1 and stores the result in arr3
void add_arr(int16_t arr1[], int16_t arr2[], int16_t arr3[], int num){
  for (int i = 0; i < num; i++){
    arr3[i] = arr1[i] + arr2[i];
  }
}

//takes a starting note in [0,12] and returns the delta'th note in that major scale
float major_scale_transform(float freq, int delta, int debug){ 
  //major scale in the form w-w-h-w-w-w-h
  if (debug){
    printf("DEBUG2: mst w freq %lf, delta %d\n", freq, delta);
  }
  for (int i = 0; i < delta; i++){
    freq *= pow(2., (1. / 12.));
  }
 return freq;
}
  

//returns a hertz value
//80% chooses 1 of 3 notes in this chord
//20% chooses 1 of 7 notes in the major scale, including notes in the chord bc i am too lazy to remove
float note(int key, int chord, int debug){
  if (debug){
    printf("DEBUG: note() with key = %d, chord = %d\n", key, chord);
  }
  float freq = base_keys_hz[key];
  int choice = rand() % 5;
  if (debug){
    printf("DEBUG: choice = %d\n", choice);
  }
  if (choice != 0){
    int chord_notes[3];
    int chord_offsets[7] ={0, 2, 4, 5, 7, 9, 11};
    chord_notes[0] = key + chord_offsets[(abs(chord) - 1) % 7]; //in the range [key, key + 11]
    chord_notes[1] = key + chord_offsets[(abs(chord) + 1) % 7];
    chord_notes[2] = key + chord_offsets[(abs(chord) + 3) % 7];
    if (chord < 0){
      chord_notes[1] -= 1; //flat 3rd for minor
      if (chord_notes[1] < key){
        chord_notes[1] += 12; //looping back up if flattening 3rd puts us below base key
      }
    }
    if (debug){
      printf("DEBUG: chord notes are ");
      for (int i = 0; i < 3; i++){
        printf("%d ", chord_notes[i]);
      }
      printf("\n");
    }
    int chord_choice = chord_notes[rand() % 3];
    return major_scale_transform(freq, chord_choice - key, 0);
  }
  else{
    float scale[7] = {major_scale_transform(freq, 0, 0),
                    major_scale_transform(freq, 1, 0),
                    major_scale_transform(freq, 2, 0),
                    major_scale_transform(freq, 3, 0),
                    major_scale_transform(freq, 4, 0),
                    major_scale_transform(freq, 5, 0),
                    major_scale_transform(freq, 6, 0)
                  };  
    return scale[rand() % 7];
  }
}


int16_t sawtooth(float t, float freq) {
    float phase = t * freq;
    return (phase - floor(phase)) * MAX_INT16 * AMPLITUDE; 
}

//fills the array with samples of a sawtooth at the given frequency
void fill_samples(int16_t arr[], int size, float freq){
  for (int i = 0; i < size; i++){
    arr[i] = sawtooth((float)i / (float)SAMPLE_RATE, freq);
  }
}

void fill_measure(int16_t measure[], int key, int chord, int size){
  int16_t melody[size];
  int16_t base[size];
  for (int i = 0; i < 8; i++){
    fill_samples(melody + (i * (size / 8)), size / 8, note(key, chord, 0));
  }
  if (base_flag){
    fill_samples(base, size, base_keys_hz[key + abs(chord) - 1] / 4.); 
  }
  //todo: harmony, drums
  add_arr(melody, base, measure, size);
}

//COPIED FROM PREVIOUS ASSIGNMENT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//writes num_bytes of an int to a file pointed to by fp, in little endian
void write_LE(FILE *fp, int num, int num_bytes){
  char byte;
  for (int i = 0; i < num_bytes; i++){
    byte = (num >> 8*i) & 0xff;
    fwrite(&byte, 1, 1, fp);
  }
}

//writes a string to the file pointed to by f
void write_str(FILE *fp, char *str){
  fwrite(str, strlen(str), 1, fp);
}

//writes a wav header to fp
void write_header(FILE *fp){
  write_str(fp, "RIFF");
  write_LE(fp, (44 + (NUM_CHANNELS*SAMPLE_RATE*duration*SAMPLE_BITS/8)) - 8, 4);
  write_str(fp, "WAVE");
  write_str(fp, "fmt ");
  write_LE(fp, 16, 4);
  write_LE(fp, 1, 2);
  write_LE(fp, NUM_CHANNELS, 2);
  write_LE(fp, SAMPLE_RATE, 4);
  write_LE(fp, SAMPLE_RATE*NUM_CHANNELS*SAMPLE_BITS/8, 4); //Byte rate
  write_LE(fp, NUM_CHANNELS*SAMPLE_BITS/8, 2); //Bytes per frame
  write_LE(fp, SAMPLE_BITS, 2);
  write_str(fp, "data");
  write_LE(fp,NUM_CHANNELS*SAMPLE_RATE*duration*SAMPLE_BITS/8, 4);
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char *argv[]){
  struct option long_opts[] = {
    {"output", required_argument, NULL, 'o'},
    {"seed", required_argument, NULL, 's'},
    {"base", no_argument, NULL, 'b'},
    { NULL, 0, NULL, 0 }
  };
  bool output = false;
  int opt;
  int seed = 1;
  char *output_file;
  while ((opt = getopt_long(argc, argv, "o:s:b", long_opts, NULL)) != -1) {
    switch (opt) {
      case 'o':
        output = true;
        output_file = optarg;
        break;
      case 's':
        seed = atoi(optarg); //todo, make sure optarg is a valid int. in the meantime, be good
        break;
      case 'b':
        base_flag = true;
        break;
      case '?':
        return 1;
    }
  }
  //song setup ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  srand(seed);
  int tempo = (rand() % 81) + 80;
  int song_choice = rand() % 3;
  int done = false;
  line_choices[0] = rand() % 10; //A
  while (!done){ //B
    line_choices[1] = rand() % 10;
    if (line_choices[1] != line_choices[0]){
      done = true;
    }
  }
  done = false;
  while (!done){ //C
    line_choices[2] = rand() % 10;
    if (line_choices[2] != line_choices[0] && line_choices[2] != line_choices[1]){
      done = true;
    }
  }
  done = false;
  while (!done){ //D
    line_choices[3] = rand() % 10;
    if (line_choices[3] != line_choices[0] && line_choices[3] != line_choices[1] && line_choices[3] != line_choices[2]){
      done = true;
    }
  }

  int key_choice = rand() % 13;
  
  int samples_per_measure = (int)SAMPLE_RATE * 60 * pow(tempo, -1) * 4; //eigth notes
  duration = (float)(24 * samples_per_measure) / (float)SAMPLE_RATE; 
  //6 lines * 4 measures per line = 24 measures per song
  int16_t measure[samples_per_measure];
  printf("tempo: %d, samples per measure: %d\n",tempo, samples_per_measure);
  //setup end ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (output){
    FILE *fp = fopen(output_file, "wb");
    if (fp == NULL) {
      perror("fopen");
      return 1;
    }
    write_header(fp);
    for (int i = 0; i < 6; i++){ //line
      int line = song_structures[song_choice][i];
      for (int j = 0; j < 4; j++){ //chord
        int chord = line_structures[line_choices[line]][j]; 
        fill_measure(measure, key_choice, chord, samples_per_measure);
        for (int k = 0; k < samples_per_measure; k++){
          write_LE(fp, measure[k], 2);
        }
      }
    }
  }
  else{
    printf("normal mode\n");
  }
}
