#include <Keypad.h>
#include <arduinoFFT.h>
#include <FastLED.h>

#define SAMPLES 8        // Must be a power of 2
#define LED_PIN     6     // Data pin to LEDS
#define NUM_LEDS    270  
#define BRIGHTNESS  128    // from 0 to 255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB 

// Mode control
#define NUM_MODES 4
#define MODE_PIN 2


#define ROWS 5
#define COLS 9


// Visualiser
#define MIC_IN A0
double vReal[SAMPLES];
double vImag[SAMPLES];
int intensity[COLS] = { };
int displacement = 1;
arduinoFFT FFT = arduinoFFT();
int xres = COLS * 2;
int yres = ROWS * 3;

int values[ROWS][COLS]; // 2D array to keep track of the current color of each tile
int mem_values[ROWS][COLS]; // 2D array to keep track Memory Colors
int brightness[ROWS][COLS]; // 2D array to keep track of the current brightness of each tile
int mode = 2;

bool escape;

// 5 * 9 matrix
const byte rows = 5;
const byte cols = 9;

char keys[rows][cols] = {
  {1, 2, 3, 4, 5, 6, 7, 8, 9},
  {11, 12, 13, 14, 15, 16, 17, 18, 19},
  {21, 22, 23, 24, 25, 26, 27, 28, 29},
  {31, 32, 33, 34, 35, 36, 37, 38, 39},
  {41, 42, 43, 44, 45, 46, 47, 48, 49},
};

// 22 = First Row
// 31 = First Column
byte rowPins[rows] = {22, 24, 26, 28, 30};
byte colPins[cols] = {31, 33, 35, 37, 39, 41, 43, 45, 47};
Keypad buttons = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

CRGB leds[NUM_LEDS];
CRGBPalette16 currentPalette;

void setup() {
    Serial.begin(9600);
    delay(1000); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    buttons.setHoldTime(500); // require a .5 second hold to change modes
    randomSeed(analogRead(0)); //Seed Random
    // clear_display(); //Make sure the display is blank

    pinMode(MODE_PIN, INPUT_PULLUP);

    pinMode(MIC_IN, INPUT);
}

void loop() {
  
  // draw_tic_border();
  // tic();
  
  for (int i = 0; i < ROWS; i++){
    for (int j = 0; j < COLS; j++){
      // light_tile(i, j, 197, 197);
      // FastLED.show();
      // delay(100);
      // clear_display();
    }
  }
// int location = buttons.getKey();
// light_tile(location/10, (location%10)-1, 197, 255);
// FastLED.show();
// Serial.write(location);


  int modeBtnState = digitalRead(MODE_PIN);
  if (modeBtnState == LOW){
    // clear_display();
    // cycle_mode();
  }

  // light_row(0, 0, 1, 127, 255);
  
  switch(mode){
    case 1:
      Visualiser();
      // light_row(0, 0, 1, 127, 255);
      break;
    case 2:
      paint();
      light_row(1, 5, 2, 127, 255);
      FastLED.show();
      break;
    case 3:
      tic();
      // light_row(0, 0, 3, 127, 255);
      break;
    case 4:
      Memory();
      // light_row(0, 0, 4, 127, 255);
      break;
  }
}

void cycle_mode(){
  mode++;
  if (mode > NUM_MODES){
    mode = 1;
  }
}

// lights a specific led dictated by row and column
void light_led(int row, int col, int color, int brightness){
  int extra = 0;
  if (col % 2 != 0){
    col -= 1;
    extra = 1 + (row * 2);
  }
  int led = (14 - row) + (15 * col) + extra;
  leds[led]= CHSV(192, 255, 190);
  FastLED.show();    
}

void light_column(int rowStart, int rowEnd, int col, int color, int bright){
    for(int i=rowStart; i >= rowEnd; i--){
        leds[i + 15*col]= CHSV(color, 255, bright); 
        
        // values[i][col] = color; 
        // brightness[i][col] = bright;
    }
}
void light_row(int colStart, int colEnd, int row, int color, int bright){
  for (int i = colStart; i <= colEnd; i++){
    light_led(row, i, color, bright);
  }
}

void light_tile(int row, int col, int color, int bright){
  
    if (color == 256) 
        bright = 0; 
    for(int i=14; i >= 12; i--){
        leds[i - 3*row + 30*col]= CHSV(color, 255, bright); 
    }
    for(int i=15; i <= 17; i++){
        leds[i + 3*row + 30*col]= CHSV(color, 255, bright); 
    }

    // values array keeps track of the current colour of a tile
    values[row][col] = color;
    brightness[row][col] = bright; 
}

void paint(){
  clear_display();
  // light_column(14, 6, 6, 192, 190);
  // light_row(0, 6, 9, 192, 190);
  bool tap = 1;
  int color = 0;
  while(tap){                                               //Loop until a button is held
    int location = buttons.getKey();
    if (location){                                          //if a button is pressed
      if(values[location/10][(location%10)-1] != 256){
        color = values[location/10][(location%10)-1] + 32;  //Update Color if tile is already colored
        if (color > 256)
          color = 0;
      }  
      else if (color == 256)                                //Loop back to red at end of rainbow
        color = 0;
      light_tile(location/10, (location%10)-1, color, 255); //Light tile color
      values[location/10][(location%10)-1] = color;
      FastLED.show();                                       //Update display
      delay(100);                                           //wait 1/10th of a second
    }
    if(buttons.getState() == HOLD)                          //Exit function if tile held
      tap = 0;
  }
}


void clear_display(){
  for(int i = 0; i < ROWS; i++){
    for(int j = 0; j < COLS; j++){
      light_tile(i, j, 256, 0);
      values[i][j] = 256;
    }
  }
  FastLED.show();
}

void draw_tic_border(){
  // top of grid
  // clear_display();
  int tileColour = 185;
  int tileBrightness = 100;
  
  light_column(11, 3, 5, tileColour, tileBrightness);
  light_column(11, 3, 12, tileColour, tileBrightness);
  light_row(5, 12, 2, tileColour, tileBrightness);
  light_row(5, 12, 12, tileColour, tileBrightness);
  
  FastLED.show();
}

//-----------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------
void tic(){
  clear_display();
  draw_tic_border();
  bool tap = 1;
  bool turn = 0;
  while(tacwinner()){
    int location = buttons.waitForKey();
    if(buttons.getState() == HOLD)
      return;
    else if (location){
      int color = 0;
      if(turn)
        color = 120;
      light_tile(location/10, (location%10)-1, color, 255);
      FastLED.show();
      delay(100);
      turn = !turn;
    }
  }
}


bool tacwinner(){
  //// Modified check meant for top left corner

  // Checks for tiles lit up in a row
  for (int i = 1; i < 4; i++){
    if (values[i][0] == values[i][1] && values[i][0] == values[i][2]){
      if (values[i][0] == 0){
        redwins();
        return 0;
      }
      if (values[i][0] == 120){
        bluewins();
        return 0;
      }
    }
  }

  // Checks for tiles lit up in a column
  for (int i = 3; i < 6; i++){
    if (values[0][i] == values[1][i] && values[0][i] == values[2][i]){
      if (values[i][0] == 0){
        redwins();
        return 0;
      }
      if (values[i][0] == 120){
        bluewins();
        return 0;
      }
    }
  }

  if (values[1][3] == values[2][4] && values[1][3] == values[3][5]){
    if (values[1][3] == 0){
      redwins();
      return 0;
    }
    if (values[1][3] == 120){
      bluewins();
      return 0;
    }
  }

  if (values[1][5] == values[2][4] && values[1][5] == values[3][3]){
      if (values[1][5] == 0){
        redwins();
        return 0;
      }
      if (values[1][5] == 120){
        bluewins();
        return 0;
      }
    }

  // Draw condition
  for (int i = 1; i < 4; i++){
    for (int j = 3; j < 6; j++){
      if (values[i][j] == 256){
        return 1;
      }
    }
  }

  //// Original check
  // for(int i = 3; i < ROWS; i+=2){
  //   if (values[i][0] == values[i][2] && values[i][0] == values[i][4]){
  //     if (values[i][0] == 0){
  //       redwins();
  //       return 0;
  //     }
  //     if (values[i][0] == 120){
  //       bluewins();
  //       return 0;
  //     }
  //   }
  // }
  // for(int i = 0; i < 5; i+=2){
  //   if (values[3][i] == values[5][i] && values[3][i] == values[7][i]){
  //     if (values[3][i] == 0){
  //       redwins();
  //       return 0;
  //     }
  //     if (values[3][i] == 120){
  //       bluewins();
  //       return 0;
  //     }
  //   }
  // }
  // if (values[3][0] == values[5][2] && values[3][0] == values[7][4]){
  //   if (values[3][0] == 0){
  //     redwins();
  //     return 0;
  //   }
  //   if (values[3][0] == 120){
  //     bluewins();
  //     return 0;
  //   }
  // }
  // if (values[7][0] == values[5][2] && values[7][0] == values[3][4]){
  //   if (values[7][0] == 0){
  //     redwins();
  //     return 0;
  //   }
  //   if (values[7][0] == 120){
  //     bluewins();
  //     return 0;
  //   }
  // }
  // // Draw condition
  // for (int i = 3; i < ROWS; i += 2)
  //   for (int j = 0; j < 5; j += 2)
  //     if (values[i][j] == 256)
  //       return 1;
        
  nowins();
  return 0;
}

void redwins(){
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLS; j++)
      light_tile(i, j, 0, 255);
  FastLED.show();
  delay(3000);
  clear_display();
}

void bluewins(){
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLS; j++)
      light_tile(i, j, 120, 255);
  FastLED.show();
  delay(3000);
  clear_display();
}

void nowins(){
  bool yes = 0;
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLS; j++){
      int color = 0;
      if(yes)
        color = 120;
      light_tile(i, j, color, 255);
      yes = !yes;
    }
  FastLED.show();
  delay(3000);
  clear_display();
}
//-----------------------------------------------------------------------------------

void Visualiser(){
  //Collect Samples
  getSamples();
  
  //Update Display
  visualiserUpdate();
  
  FastLED.show();
}

void getSamples(){
  for(int i = 0; i < SAMPLES; i++){
    vReal[i] = analogRead(MIC_IN);
    Serial.println(vReal[i]);
    vImag[i] = 0;
  }

  //FFT
  FFT.Windowing(vReal, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, SAMPLES);

  //Update intensity Array
  for(int i = 2; i < (xres*displacement)+2; i+=displacement){
    vReal[i] = constrain(vReal[i],0 ,2047);            // set max value for input data
    vReal[i] = map(vReal[i], 0, 2047, 0, yres);        // map data to fit our display

    intensity[(i/displacement)-2] --;                      // Decrease displayed value
    if (vReal[i] > intensity[(i/displacement)-2])          // Match displayed value to measured value
      intensity[(i/displacement)-2] = vReal[i];
  }
}

void visualiserUpdate(){
  int color = 0;
  for(int i = 0; i < xres; i++){
    for(int j = 0; j < yres; j++){
      int index;
      if (i % 2 == 0) {
        index = i * yres + j;
      } else {
        index = (i + 1) * yres - j - 1;
      }
      if(j <= intensity[i]){
        leds[index] = CHSV(color, 255, BRIGHTNESS);
      }
      else{
        leds[index] = CHSV(color, 255, 0);
      }
    }
    color += 255/xres;
  }
}


//-----------------------------------------------------------------------------------
void Memory(){
  clear_display();                    //Clear display
  for(int i = 2; i < 6; i++){         //Print Vertical lines of box
    light_tile(i, 1, 192, 255);
    light_tile(i, 6, 192, 255);
  }
  for(int i = 1; i < 7; i++){         //Print Horizontal lines of box
    light_tile(1, i, 192, 255);       
    light_tile(6, i, 192, 255);       
  }
  FastLED.show();
  Set_Colors();                       //Set Random Color Locations
  bool tap = 1;
  int color;
  int tile1;
  int tile2;
  int end = 0;
  while(tap){                         //Loop until BUTTON is held
    int location1 = buttons.waitForKey();
    if(buttons.getState() == HOLD)    //Exit if button is held
      tap = 0;
    if (location1){                    //When a button is pushed
      color = mem_values[location1/10][(location1%10)-1];         //Update the color of the clicked tile
      light_tile(location1/10, (location1%10)-1, color, 255);
      tile1 = color;
      FastLED.show();
    }
    int location2 = buttons.waitForKey();
    if(buttons.getState() == HOLD)   //Exit if held
      tap = 0;
    if (location2){                  //When a button is pushed
      color = mem_values[location2/10][(location2%10)-1];         //Update the color of the clicked tile
      light_tile(location2/10, (location2%10)-1, color, 255);
      tile2 = color;
      FastLED.show();
    }
    if(tile1 != tile2 || location1 == location2){              //If tiles are not the same color
        light_tile(location1/10, (location1%10)-1, 256, 255);   //Light the tiles black
        light_tile(location2/10, (location2%10)-1, 256, 255);
    }
    else
      end++;
    delay(1000);
    FastLED.show();
    if(end == 8)
      tap = 0;
  }
}


void Set_Colors(){  //Makes a 4x4 grid of colored pairs in random locations for memory game
  int color = 0;
  for(int i = 0; i < ROWS; i++)
    for(int j = 0; j < COLS; j++)
      mem_values[i][j] = 256;
  for(int j = 0; j < 8; j++){         //Place 8 colors
    for(int i = 0; i< 2; i++){        //place each color twice
      bool tile_not_empty = 1;
      while(tile_not_empty){          //don't place color over another color
        int row = (random()%4)+2;       //Get random tile
        int col = (random()%4)+2;
        if(mem_values[row][col] == 256){  //Check if tile is empty
          mem_values[row][col] = color;   //Place color
          tile_not_empty = 0;         //Exit loop
        }
      }
    }
    color += 32;                      //Increment Color
  }
}
//-----------------------------------------------------------------------------------