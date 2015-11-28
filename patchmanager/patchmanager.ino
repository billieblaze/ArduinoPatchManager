#include <EEPROM.h>
int eeAddress = 0; //EEPROM address to start reading from

// setup LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C  lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// setup Encoder / Button combo 
#include <Encoder.h>
Encoder myEnc(2,3);
int position = 0;
int encoderState;             // the current reading from the input pin
int lastEncoderState = LOW;   // the previous reading from the input pin
long lastEncoderDebounceTime = 0;  // the last time the output pin was toggled


int menuButton = 8;
int buttonState;             // the current reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

// All the midi
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

// program variables
byte channels = 4;  // number of midi channels
byte currentProgram = 0; // currently selected program

// first program slot is just a placeholder, position 2+ are each channels patch number
byte programs[][5] = {
  {1, 0, 0, 0, 0},
  {2, 0, 0, 0, 0},
  {3, 0, 0, 0, 0},
  {4, 0, 0, 0, 0},
  {5, 0, 0, 0, 0},
  {6, 0, 0, 0, 0},
  {7, 0, 0, 0, 0},
  {8, 0, 0, 0, 0},
  {9, 0, 0, 0, 0},
  {10, 0, 0, 0, 0},
  {11, 0, 0, 0, 0},
  {12, 0, 0, 0, 0},
  {13, 0, 0, 0, 0},
  {14, 0, 0, 0, 0},
  {15, 0, 0, 0, 0},
  {16, 0, 0, 0, 0}
  
};


byte numPrograms = 15;

// where on the screen am i, and how do i get to my next cursor position?
int currentPosition = 0;
int screenPositions[][2] =  {{ 10, 0 }, { 4, 3 }, { 8, 3 }, { 12, 3 }, {16, 3} };

byte screenSelect = 0;
byte isSaving = 0; 

//#define debug true

// on your mark...
void setup(){

  #if defined(  debug )
      Serial.begin(9600);
      while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
      }
      Serial.write("Begin");
  #endif 
 
  
  #if defined(  debug )
    Serial.print("CONFIG");
  #endif 
  
  // setup ui
  pinMode(menuButton, INPUT); 
  
  loadData();

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

  int reading = myEnc.read();

  // debounce it 
  if (reading != lastEncoderState) {
    lastEncoderDebounceTime = millis();
  }

  if ((millis() - lastEncoderDebounceTime) > debounceDelay) {
  
    if (reading != encoderState) {
      encoderState = reading;

      if (reading != position) {
  
        if (reading > position) {
          handleEncoder(1); // increment
        } else {
          handleEncoder(-1); // decrement
        }
      }
      
      position = reading;  // rememberit
    }
  }
  lastEncoderState = reading;  // clear debouncer
 
}

void handleEncoder( int increment){  
  if (screenSelect==0){ 
    // if we're on the current progam position, change program up / down
    if ( currentPosition == 0 ) {
        currentProgram += increment;

        
        if (currentProgram < 0 ) {
          currentProgram = numPrograms;
        }

        if (currentProgram > numPrograms ) {
          currentProgram = 0;
        }

        setAllPrograms();  // got eem 

      } else {
        // we're on a specific channel, lets only update that
        programs[currentProgram][currentPosition] += increment;
        changeProgram( currentPosition );       
      }
      
  } else { 
   
    isSaving = !isSaving; 
    
  }
  
  paintLCD(); // refreshit
        
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
        
          if(screenSelect == 0){ 
             currentPosition++; //move cursor
              // prevent overflow
              if (currentPosition == 5) { 
                screenSelect = 1; 
                paintLCD();
              }
              
              if (currentPosition > 4 ) {
                currentPosition = 0; 
              }
          } else { 
            if ( isSaving == 0){ 
              lcd.setCursor(0,2);
              lcd.print("CANCELLING...");
              delay(200);
              screenSelect = 0; 
            } else { 
              lcd.setCursor(0,2);
              lcd.print("SAVING...");
              delay(200);
              lcd.setCursor(0,3);
              saveData();
              screenSelect = 0; 
            }
            
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
  
  if (screenSelect==0) screen0();
  if (screenSelect==1) screen1();
}

void screen0(){ 
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

void screen1(){ 
   lcd.print("Save? ");
   
   if ( isSaving == 0) { 
    lcd.print("NO"); 
   } else { 
    lcd.print("YES");
   }
}

void loadData(){
  // load the data from eeprom
  eeAddress = 0; 
  for( int i = 0; i < 16; i++) { 
    for ( int j=0; j < sizeof(programs[i]); j++){         
       programs[i][j] = EEPROM.read(eeAddress);  
       eeAddress++;
     }
   }   
}

void saveData(){
  // load the data from eeprom
  eeAddress = 0; 
  for( int i = 0; i < 16; i++) { 
    for ( int j=0; j < sizeof(programs[i]); j++){         
       EEPROM.update(eeAddress, programs[i][j]);  
       eeAddress++;
       lcd.print(".");
     }
   }   
}

