#ifndef CMS_H
#define CMS_H

// enums provide meaningful identifiers for status/errors at various failure
// points
typedef enum {
  CMS_SUCCESS = 0,            // operation completed successfully
  CMS_ERROR_INIT,             // cms initialisation failed
  CMS_ERROR_DB_INIT,          // database initialisation failed
  CMS_ERROR_INVALID_ARGUMENT, // invalid argument provided
  CMS_ERROR_FILE_OPEN,        // failed to open file
  CMS_ERROR_FILE_IO           // file i/o operation failed
} CMSStatus;

CMSStatus cms_init(void);
CMSStatus display_menu(void);
CMSStatus main_loop(void);
const char *cms_status_string(CMSStatus status);

#endif
