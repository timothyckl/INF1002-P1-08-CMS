#include "cms.h"
#include <stdlib.h>

int main(void) {
  CMSStatus status = run_cms_session();
  return (status == CMS_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;
}
