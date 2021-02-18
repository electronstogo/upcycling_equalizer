// 2020 - electronstogo

#include <Adafruit_NeoPixel.h>


// Point structure just describes one point in the LED matrix.
struct Point
{
	//char x, y;
	char r, g, b;
	bool active;
};


// top point structure, that is used to control the falling mode, of the top LEDs in every column.
struct TopPoint
{
	int position;
	
	// pushed means that the level has reached a new top level,
	// and the top LED will keep the position for at least a moment.
	int pushed;
};



// constants which define the LED matrix size.
#define ROWS         10

#define COLUMNS      7

#define NUMPIXELS    ROWS * COLUMNS


// pin constants for LED control.
#define LED_DATA_PIN 8 


// pin constants for the spectrum analyzer chip msgeq7.
#define STROBE_PIN   10

#define RESET_PIN    3

#define ANALOG_PIN   0



// create a LED matrix of ROWS * COLUMNS points.
// This will be used as buffer, to flush the LEDs. 
Point g_led_matrix[ROWS][COLUMNS];

// array of top points, to handle falling mode for top LEDs in the columns.
TopPoint g_array_top[COLUMNS];


// pixel control
Adafruit_NeoPixel g_pixels = Adafruit_NeoPixel(NUMPIXELS, LED_DATA_PIN, NEO_RGB + NEO_KHZ800);




void setup() 
{   
	g_pixels.begin();
	g_pixels.show();

	pinMode(STROBE_PIN, OUTPUT);
	pinMode(RESET_PIN, OUTPUT);
	pinMode(ANALOG_PIN, INPUT);

	// init  msgeq7
	digitalWrite(RESET_PIN, LOW);
	digitalWrite(STROBE_PIN, LOW);
	delay(1);

	digitalWrite(RESET_PIN, HIGH);
	delay(1);

	digitalWrite(RESET_PIN, LOW);
	digitalWrite(STROBE_PIN, HIGH);
	delay(1);
	
	
	for(int column = 0; column < COLUMNS; column++)
	{
		g_array_top[column].position = 0;
		g_array_top[column].pushed = 0;
		g_led_matrix[column][0].r = 254;
	}
}



void loop() 
{
	// main loop counter, used to control function calls, for the top LED sinking mode.  
	static unsigned int s_loop_counter = 0;


	clear_led_matrix();

	digitalWrite(RESET_PIN, HIGH);
	delay(5);
	digitalWrite(RESET_PIN, LOW);


	// array to keep the spectrum levels.
	int spectrum_value[7];


	// get frequency levels.
	for(int channel = 0; channel < 7; channel++)
	{
		digitalWrite(STROBE_PIN, LOW);
		delay(10);
		spectrum_value[channel] = analogRead(ANALOG_PIN);
		
		// remove noise.
		if(spectrum_value[channel] < 100)
		{
			spectrum_value[channel] = 0;
		}
		
		digitalWrite(STROBE_PIN, HIGH);

		// map frequency value to LED height.
		spectrum_value[channel] = constrain(spectrum_value[channel], 0, 1023);
		spectrum_value[channel] = map(spectrum_value[channel], 0, 1023, 0, ROWS - 1);
		
		if(spectrum_value[channel] > 9)
		{
			spectrum_value[channel] = 9;
		}
		else if(spectrum_value[channel] < 0)
		{
			spectrum_value[channel] = 0;
		}
	}


	// loop through all LED columns in the LED matrix.
	for(int column = 0; column < COLUMNS; column++)
	{
		// loop through all LEDs in the current column, until the spectrum value top point is reached.
		for(int row = 0; row < spectrum_value[column]; row++)
		{
			g_led_matrix[row][COLUMNS - 1 - column].active = true;
			g_led_matrix[row][COLUMNS - 1 - column].b = 254;
			g_led_matrix[row][COLUMNS - 1 - column].g = 0;
			g_led_matrix[row][COLUMNS - 1 - column].r = 0;
		}

		
		// check if the column got a new top level LED.
		if(spectrum_value[column] >= g_array_top[column].position)
		{
			g_array_top[column].position = spectrum_value[column];
			g_array_top[column].pushed = 6;
		}
		
		g_led_matrix[g_array_top[column].position][COLUMNS - 1 - column].active = true;
		g_led_matrix[g_array_top[column].position][COLUMNS - 1 - column].r = 254;
		g_led_matrix[g_array_top[column].position][COLUMNS - 1 - column].g = 0;
		g_led_matrix[g_array_top[column].position][COLUMNS - 1 - column].b = 0;
	}
  
	flush_led_matrix();



	if(s_loop_counter % 2)
	{
		top_sinking();
	}
	
	
	s_loop_counter++;
}


// handle the sinking mode, for the top LEDs in every column.
void top_sinking()
{
	for(int column = 0; column < COLUMNS; column++)
	{
		if(g_array_top[column].position > 0 && g_array_top[column].pushed <= 0)
		{
			g_array_top[column].pushed = 0;
			g_array_top[column].position--;
		}
		else if(g_array_top[column].pushed > 0)
		{
			g_array_top[column].pushed--;
		}
	}
}



// just clear the whole matrix buffer for the LEDs.
void clear_led_matrix()
{
	for(int row = 0; row < ROWS; row++)
	{
		for(int column = 0; column < COLUMNS; column++)
		{
			g_led_matrix[row][column].active = false;
		}
	}
}



// Update the LED matrix, with the current buffer data.
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

void flush_led_matrix()
{
	// loop through the LED matrix columns.
	for(int column = 0; column < COLUMNS; column++)
	{
		for(int row = 0; row < ROWS; row++)
		{
			// data direction of columns variies, because of the hardware wiring. Choose direction.
			int corrected_row = row;
		
			if(column % 2)
			{
				corrected_row = ROWS - 1 - row;
			}
			
			
			// sendo color config to active LEDs 
			if(g_led_matrix[corrected_row][column].active)
			{
				g_pixels.setPixelColor(column * ROWS + row, g_pixels.Color(g_led_matrix[corrected_row][column].r,
				g_led_matrix[corrected_row][column].g, g_led_matrix[corrected_row][column].b));
			}
			else
			{
				g_pixels.setPixelColor(column * ROWS + row, 0, 0, 0);  
			}
		} 
	}


	g_pixels.show();
}
