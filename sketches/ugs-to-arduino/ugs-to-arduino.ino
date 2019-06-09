int incomingCommand = 0;   // for incoming serial data which will be gcode commands

void setup() {
        Serial.begin(9600);     // opens serial port, sets data rate/baud rate to 9600 bps
        Serial.print("\r\nGrbl 0.8c ['$' for help]\r\n"); // baud rate for lower than v1 is 9600
}

void loop() {
  static int received = 0;
        // send data only when you receive data:
        if (Serial.available() > 0) {
          if (received == 0) {
                Serial.print("\r\nGrbl 0.8c ['$' for help]\r\n");
                received = 1;
          }

                // read the incoming command:
                incomingCommand = Serial.read();

                // say command:
                Serial.print(incomingCommand);
                Serial.print("ok");

        }
}
 
