#include <EEPROM.h>

byte programs[][5] = {
  {1, 10, 1, 5, 1},
  {2, 20, 2, 10, 0},
  {3, 30, 3, 15, 2},
  {4, 40, 4, 20, 0},
  {5, 50, 5, 25, 3},
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

void setup() {

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

   int eeAddress = 0;   //Location we want the data to be put.
   Serial.print("loading programs");
   Serial.println();
   
   for( int i = 0; i < 16; i++) { 
     Serial.print(i);
     Serial.print(":");
     
     for ( int j=0; j < sizeof(programs[i]); j++){ 
       Serial.print(programs[i][j]);
       Serial.print("  ");
       
       
       EEPROM.update(eeAddress, programs[i][j]);
       eeAddress++;
     }
     Serial.println();
   }  
   
   Serial.print("Program data loaded! \n");
}

void loop() {
  /* Empty loop */
}
