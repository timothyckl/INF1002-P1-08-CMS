#ifndef CMS_H
#define CMS_H

// enums provide meaningful identifiers for status/errors at various failure
// points
typedef enum {
  CMS_SUCCESS,         // when an operation executes without issues
  CMS_INIT_FAILURE,    // failed initialise step
  CMS_DB_INIT_FAILURE, // database failed to initialise
  CMS_INVALID_ARG,     // failed to receive expected arguments
  CMS_FILE_OPEN_ERR,   // failed to get file handle
  CMS_FILE_IO_ERR,     // failed any file read/write operations etc
} CMSStatus;

CMSStatus cms_init(void);
CMSStatus display_menu(void);
CMSStatus main_loop(void);
const char *cms_status_string(CMSStatus status);

#endif
