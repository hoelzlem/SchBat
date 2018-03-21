#include "mcc_generated_files/mcc.h"

void main(void) {
  volatile uint8_t myint;
  SYSTEM_Initialize();
  
  for (myint = 3;;) {
      myint = myint;
  }
}
