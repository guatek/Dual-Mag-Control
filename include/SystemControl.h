#ifndef _SYSTEMCONTROL

#define _SYSTEMCONTROL

#include <Arduino.h>

#define CMD_CHAR '!'
#define CMD_BUFFER_SIZE 128
#define CMD_TIMEOUT 10000
#define LOCAL_ECHO 1

class SystemControl
{
    private:
    
    Stream *debug;
    Stream *ui1;
    Stream *ui2;
    char cmdBuffer[CMD_BUFFER_SIZE];
    
    void readInput(Stream *in) {
      
        if (in != NULL && in->available() > 0) {
            char c = in->read();
            if (c == CMD_CHAR) {

                if (LOCAL_ECHO)
                    in->write(c);
              
                unsigned long startTimer = millis();
                int index = 0;
                while (startTimer <= millis() && millis() - startTimer < CMD_TIMEOUT) {

                    // Break if we have exceed the buffer size
                    if (index >= CMD_BUFFER_SIZE)
                        break;

                    // Wait on user input
                    if (in->available()) {
                        // Read the next char and reset timer          
                        c = in->read();
                        startTimer = millis();
                    }
                    else {
                        continue;
                    }
                    
                    if (c == '\n' || c == '\r') {
                        // Command ended try to 
                        if (index <= CMD_BUFFER_SIZE)
                            cmdBuffer[index++] = '\0';
                        else
                            cmdBuffer[CMD_BUFFER_SIZE-1] = '\0';
                        
                        in->println("\n\rReceived: " + String(cmdBuffer));
                        break;
                    }
                    
                    // Handle backspace
                    if (c == '\b') {
                        index -= 1;
                        if (index < 0)
                            index = 0;
                        if (LOCAL_ECHO) {
                           in->write("\b \b");
                        }
                    }
                    else {
                        cmdBuffer[index++] = c;
                        if (LOCAL_ECHO)
                            in->write(c);
                    }

                    
                }
            }
        }
    }
                      
 
    public:
  
    SystemControl(Stream *ui1, Stream *ui2 = NULL, Stream *debug = NULL) {
        this->ui1 = ui1;
        this->ui2 = ui2;
        this->debug = debug;
    }

    void checkInput() {
        if (ui1->available() > 0) {
            readInput(ui1);
        }
        if (ui2 != NULL && (ui2->available() > 0)) {
            readInput(ui2);
        }
    }
        
};

#endif
