#include <Keypad.h>
#include <FastLED.h>

#define SAMPLES 8        // Must be a power of 2
#define LED_PIN     6     // Data pin to LEDS
#define NUM_LEDS    270  
#define BRIGHTNESS  128    // from 0 to 255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB 
#define NUM_MODES 4

#define ROWS 9
#define COLS 5

int values[ROWS][COLS]; // 2D array to keep track of the current color of each tile
int mem_values[ROWS][COLS]; // 2D array to keep track Memory Colors
int brightness[ROWS][COLS]; // 2D array to keep track of the current brightness of each tile

bool escape;

// 3 * 3 array
const byte rows = 3;
const byte cols = 3;
char keys[rows][cols] = {
  {1, 2, 3},
  {11, 12, 13},
  {21, 22, 23},
};

byte rowPins[rows] = {36, 37, 38};
byte colPins[cols] = {22, 23, 24};
Keypad buttons = Keypad( makeKeymap(keys), rowPins, colPins, rows, cols );

CRGB leds[NUM_LEDS];
int mode = 1;
CRGBPalette16 currentPalette;

void setup() {
    Serial.begin(9600);
    delay(1000); // power-up safety delay
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
    FastLED.setBrightness(  BRIGHTNESS );
    
    buttons.setHoldTime(500); // require a .5 second hold to change modes
    randomSeed(analogRead(0)); //Seed Random
    // clear_display(); //Make sure the display is blank
}

void loop() {
  
  // draw_tic_border();
  tic();
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
  light_column(14, 6, 6, 192, 190);
  light_row(0, 6, 9, 192, 190);
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
  for (int i = 0; i < 3; i++){
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
  for (int i = 0; i < 3; i++){
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

  if (values[0][0] == values[1][1] && values[0][0] == values[2][2]){
    if (values[i][0] == 0){
      redwins();
      return 0;
    }
    if (values[i][0] == 120){
      bluewins();
      return 0;
    }
  }

  if (values[0][2] == values[1][1] && values[0][2] == values[2][0]){
    if (values[i][0] == 0){
      redwins();
      return 0;
    }
    if (values[i][0] == 120){
      bluewins();
      return 0;
    }
  }
  // Draw condition
  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      if (values[i][j] == 256){
        return 1
      }
    }
  }

  //// Original check
  for(int i = 3; i < ROWS; i+=2){
    if (values[i][0] == values[i][2] && values[i][0] == values[i][4]){
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
  for(int i = 0; i < 5; i+=2){
    if (values[3][i] == values[5][i] && values[3][i] == values[7][i]){
      if (values[3][i] == 0){
        redwins();
        return 0;
      }
      if (values[3][i] == 120){
        bluewins();
        return 0;
      }
    }
  }
  if (values[3][0] == values[5][2] && values[3][0] == values[7][4]){
    if (values[3][0] == 0){
      redwins();
      return 0;
    }
    if (values[3][0] == 120){
      bluewins();
      return 0;
    }
  }
  if (values[7][0] == values[5][2] && values[7][0] == values[3][4]){
    if (values[7][0] == 0){
      redwins();
      return 0;
    }
    if (values[7][0] == 120){
      bluewins();
      return 0;
    }
  }
  // Draw condition
  for (int i = 3; i < ROWS; i += 2)
    for (int j = 0; j < 5; j += 2)
      if (values[i][j] == 256)
        return 1;
        
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
