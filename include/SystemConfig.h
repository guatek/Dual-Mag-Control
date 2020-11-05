#include <arduino.h>

#define MAX_PARAMS 256

class SystemConfig {
    public:
        uint32_t vals[MAX_PARAMS];
        uint8_t types[MAX_PARAMS];
};