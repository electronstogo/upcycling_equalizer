#include <Adafruit_NeoPixel.h>


// Point structure just describes one point in the LED matrix.
struct Point
{
  char x, y;
  char r,g,b;
  bool active;
};

// top point structure, that is used the falling down mode, of the top LEDs in every column.
struct TopPoint
{
  int position;
  int pushed;
};



// constants which define the matrix size.
#define ROWS         10

#define COLUMNS      7

#define NUMPIXELS    ROWS * COLUMNS

// pin constants for the spectrum analyzer chip.
#define DATA_PIN     8

#define STROBE_PIN   2

#define RESET_PIN    3

#define ANALOG_PIN   0


// create a matrix of ROWS * COLUMNS points.
// This will be used as buffer, to flush the LEDs. 
Point matrix[ROWS][COLUMNS];

// array of top points, to handle falling mode for top LEDs in the columns.
TopPoint arrayTop[COLUMNS];

// array the keeps the spectrum levels.
int spectrumValue[7];

// main loop counter, used to reduce function calls.  
unsigned int loopCounter = 0;


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, DATA_PIN, NEO_RGB + NEO_KHZ800);

void setup() 
{
    pixels.begin();
    pixels.show();

    pinMode      (STROBE_PIN, OUTPUT);
    pinMode      (RESET_PIN,  OUTPUT);
    pinMode      (ANALOG_PIN,    INPUT);
   
    digitalWrite (RESET_PIN,  LOW);
    digitalWrite (STROBE_PIN, LOW);
    delay        (1);
   
    digitalWrite (RESET_PIN,  HIGH);
    delay        (1);
    digitalWrite (RESET_PIN,  LOW);
    digitalWrite (STROBE_PIN, HIGH);
    delay        (1);

}

void loop() 
{
  loopCounter++;
  
  clearMatrix();
  
  digitalWrite(RESET_PIN, HIGH);
  delay(5);
  digitalWrite(RESET_PIN, LOW);

  for(int i = 0; i < 7; i++)
  {
    digitalWrite(STROBE_PIN, LOW);
    delay(10);
    spectrumValue[i] = analogRead(ANALOG_PIN);
    if(spectrumValue[i] < 100)spectrumValue[i] = 0;
    digitalWrite(STROBE_PIN, HIGH);

    spectrumValue[i] = constrain(spectrumValue[i], 0, 1023);
    spectrumValue[i] = map(spectrumValue[i], 0, 1023, 0, ROWS);
  }


  for(int j = 0; j < COLUMNS; j++)
  {
      for(int i = 0; i < spectrumValue[j]; i++)
      {
          matrix[i][COLUMNS - 1 - j].active = true;
          matrix[i][COLUMNS - 1 - j].r = 0; matrix[i][COLUMNS - 1 - j].g = 0; matrix[i][COLUMNS - 1 - j].b = 254;
      }
  


  if(spectrumValue[j] - 1 > arrayTop[j].position)
  {
    matrix[spectrumValue[j] - 1][COLUMNS - 1 - j].r = 254; matrix[spectrumValue[j] - 1][COLUMNS - 1 - j].g = 0; matrix[spectrumValue[j] - 1][COLUMNS - 1 - j].b = 0;
    arrayTop[j].position = spectrumValue[j] - 1;
    arrayTop[j].pushed = 5;
  }
  else
  {
      matrix[arrayTop[j].position ][COLUMNS - 1 - j].active = true;
      matrix[arrayTop[j].position][COLUMNS - 1 - j].r = 254; matrix[arrayTop[j].position][COLUMNS - 1 - j].g = 0; matrix[arrayTop[j].position][COLUMNS - 1 - j].b = 0;
  }


  }
  
  flushMatrix();

  if(loopCounter % 2 == 0)topSinking();
}


// handle the sinking mode, for the top LEDs in every column.
void topSinking()
{
  for(int j = 0; j < COLUMNS; j++)
  {
      if(arrayTop[j].position > 0 && arrayTop[j].pushed <= 0) arrayTop[j].position--;
      else if(arrayTop[j].pushed > 0) arrayTop[j].pushed--;  
  }
  
}


// just clear the whole buffer for the LEDs.
void clearMatrix()
{
  for(int i = 0; i < ROWS; i++)
  {
    for(int j = 0; j < COLUMNS; j++)
    {
      matrix[i][j].active = false;
    }
    
  }
}



//  update the LED matrix, with the current buffer data.
// Following describes the pattern, for LED control directions.
// xxxxxxxxxxxxxx
//            <--
// xxxxxxxxxxxxxx
// --> 
// xxxxxxxxxxxxxx
//            <--
// xxxxxxxxxxxxxx
// -->    
// xxxxxxxxxxxxxx    
//            <--

void flushMatrix()
{
  for(int j = 0; j < COLUMNS; j++)
  {

    if( j % 2 != 0)
    {
      for(int i = 0; i < ROWS; i++)
      {
        if(matrix[ROWS - 1 - i][j].active)
        {
            pixels.setPixelColor(j * ROWS + i, pixels.Color(matrix[ROWS - 1 - i][j].r, matrix[ROWS - 1 - i][j].g, matrix[ROWS - 1 - i][j].b));
          
        }
        else
        {
            pixels.setPixelColor( j * ROWS + i, 0, 0, 0);  
        }
      
      }
    }
    else
    {
      for(int i = 0; i < ROWS; i++)
      {
        if(matrix[i][j].active)
        {
            pixels.setPixelColor(j * ROWS + i, pixels.Color(matrix[i][j].r, matrix[i][j].g, matrix[i][j].b));
          
        }
        else
        {
            pixels.setPixelColor( j * ROWS + i, 0, 0, 0);  
        }
      }      
    } 
  }


  pixels.show();
}

