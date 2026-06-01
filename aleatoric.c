//simon parker
#include <stdlib.h>
#include <stdio.h>
#include <portaudio.h>
#include <math.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>

#define SAMPLES_PER_SEC 48000
#define SAMPLE_BITS 16


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


//adds arr2 to arr1 and stores the result in arr1
void add_arr(int16_t arr1[], int16_t arr2[], int num){
  for (int i = 0; i < num; i++){
    arr1[i] += arr2[i];
  }
}

int main(int argc, char *argv[]){
  struct option long_opts[] = {
    {"output", required_argument, NULL, 'o'},
    {"seed", required_argument, NULL, 's'},
    { NULL, 0, NULL, 0 }
  };
  bool output = false;
  int opt;
  int seed = 1;
  char *output_file;
  while ((opt = getopt_long(argc, argv, "o:s:", long_opts, NULL)) != -1) {
    switch (opt) {
      case 'o':
        output = true;
        output_file = optarg;
        break;
      case 's':
        seed = atoi(optarg); //todo, make sure optarg is a valid int. in the meantime, be good
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
  printf("here is the song, measure by measure, 8 notes per measure, in hz:\n");
  printf("structure: ");
  for (int i = 0; i < 6; i++){
    printf("%d", song_structures[song_choice][i]);
  }
  printf("\n");
  for (int i = 0; i < 4; i++){
    printf("line %d is chords ", i);
    for (int j = 0; j < 4; j++){
      printf("%d ", line_structures[line_choices[i]][j]);
    }
    printf("\n");
  }
  printf("key = %d\n", key_choice);
  for (int i = 0; i < 6; i++){
    int line = song_structures[song_choice][i];
    printf("line: %d\n", line);
    for (int j = 0; j < 4; j++){
      int chord = line_structures[line_choices[line]][j];
      printf("\tchord: %d\n\t\t", chord);
      for (int k = 0; k < 8; k++){
        printf("%lf,  ", note(key_choice, chord, 0));
      }
      printf("\n");
    }
  }

  
  //setup end ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (output){
    printf("the output file is %s\n", output_file);
  }
  else{
    printf("normal mode\n");
  }
}
