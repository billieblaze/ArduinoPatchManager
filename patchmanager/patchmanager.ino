// setup LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C  lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// setup Encoder / Button combo 
#include <Encoder.h>
Encoder myEnc(2,3);
int position = 0;

int menuButton = 8;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 100;    // the debounce time; increase if the output flickers

// All the midi
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

// program variables
int channels = 4;  // number of midi channels
int currentProgram = 0; // currently selected program

// first program slot is just a placeholder, position 2+ are each channels patch number
int programs[][5] = {
  {1, 10, 1, 5, 1},
  {2, 20, 2, 10, 0},
  {3, 30, 3, 15, 2},
  {4, 40, 4, 20, 0},
  {5, 50, 5, 25, 3},
  {6, 0, 0, 0, 0},
  {7, 0, 0, 0, 0},
  {8, 0, 0, 0, 0}
};

// where on the screen am i, and how do i get to my next cursor position?
int currentPosition = 0;
int screenPositions[][2] =  {{ 10, 0 }, { 4, 3 }, { 8, 3 }, { 12, 3 }, {16, 3} };

// on your mark...
void setup()
{
  // setup ui
  pinMode(menuButton, INPUT);
  
  // initialize the lcd
  lcd.begin(20, 4);              
  paintLCD();
  lcd.home();
  lcd.blink();

  // roll that beautiful bean footage
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.sendNoteOn(42, 127, 1);   // send a note for sanity.. 
  setAllPrograms(); // jam on it. 
}

void loop()
{
  MIDI.read();   // just hanging out passing MIDI thru until we get a UI update
  readEncoders(); 
  readButtons();
}


void changeProgram( int channel ) {
  // update a single channel to the currentValue in the program array 
  MIDI.sendProgramChange( programs[currentProgram][channel]-1, channel );
}

void setAllPrograms() {
  // update all channels to their current values 
  for ( int i = 1; i < 5; i++ ) {
    MIDI.sendProgramChange(programs[currentProgram][i]-1, i);
  }
}

void readEncoders() { 
  long newPos = myEnc.read();

  if (newPos != position) {

    if (newPos > position) {
      handleEncoder(1); // increment
    } else {
      handleEncoder(-1); // decrement
    }
    paintLCD(); // refreshit
    position = newPos;  // rememberit
  }
 
}

void handleEncoder( int increment){  
    // if we're on the current progam position, change program up / down
    if ( currentPosition == 0 ) {
        currentProgram += increment;

        // only supports 0-7, will clean up when i get to memory storage
        if (currentProgram < 0 ) {
          currentProgram = 7;
        }

        if (currentProgram > 7 ) {
          currentProgram = 0;
        }

        setAllPrograms();  // got eem 

      } else {
        // we're on a specific channel, lets only update that
        programs[currentProgram][currentPosition] += increment;
        changeProgram(currentPosition );
        
      }
  
}

void readButtons(){
   int reading = digitalRead(menuButton);

  // debounce it 
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
  
    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only respond if the new button state is HIGH
      if (buttonState == HIGH) {
             currentPosition++; //move cursor
              // prevent overflow
              if (currentPosition > 4 ) {
                currentPosition = 0; 
              }
        
            paintLCD();

      }
    }
  }
  
  lastButtonState = reading;  // clear debouncer
}

void paintLCD() {
  lcd.clear();
  lcd.home ();                   // go home
  lcd.print("Program: ");
  lcd.print(programs[currentProgram][0]);

  // iterate channels, display their numbers
  lcd.setCursor(0, 2);
  for ( int  channel = 0; channel < channels; channel++ ) {
    lcd.print ( " CH");
    lcd.print(channel + 1);
  }

  // iterate values, show dem
  lcd.setCursor(0, 3);
  for ( int  channel = 0; channel < channels; channel++ ) {
    lcd.setCursor(channel * 4 + 2 , 3);
    lcd.print(programs[currentProgram][channel + 1]);

  }

  // move cursor to current position
  lcd.setCursor( screenPositions[currentPosition][0],  screenPositions[currentPosition][1]) ;

}

