#include "cms.h"
#include <stdlib.h>

int main(void) {
  CMSStatus status = main_loop();
  return (status == CMS_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
