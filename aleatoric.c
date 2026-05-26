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


char *song_structures[3] = {"AABBCC", "ABABCD", "ABCDDD"};
char *line_structures[10] = { //a is 1 minor, A is 1 major, b is 2 minor, B is 2 major, etc
  "ADbE",
  "AfbE",
  "AcDd",
  "AEbE",
  "AfDE",
  "DAfD",
  "AEFa",
  "ADdA",
  "DEAA",
  "fDAE",
};

int line_choices[4] = {-1, -1, -1, -1}; //index 0 is the choice for A, 1 for B, etc. stored values are indices into line_structures

char *base_keys[13] = {
  "A3",
  "Bf3", //b flat
  "B3",
  "C4",
  "Df4",
  "D4",
  "Ef4",
  "E4",
  "F4",
  "Gf4",
  "G4",
  "Af4",
  "A4"
};

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

char *major_scale = "AbcDEfg"; //i think, unsure about the last note

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
  
  //setup end ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (output){
    printf("the output file is %s\n", output_file);
  }
  else{
    printf("normal mode\n");
  }
  printf("Ok, i've decided on a song. it's going to be in the key of %s, %d bpm. The structure will be %s, with A = %s, B = %s, C = %s, D = %s\n", 
    base_keys[key_choice], tempo, song_structures[song_choice], 
    line_structures[line_choices[0]],
    line_structures[line_choices[1]],
    line_structures[line_choices[2]],
    line_structures[line_choices[3]]
    );
}
