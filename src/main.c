#include "cms.h"
#include <stdlib.h>

// TODO: abstract away public facing interface with simpler API calls

int main() {
  CMSStatus status = main_loop();
  // process status
  return EXIT_SUCCESS;
}
