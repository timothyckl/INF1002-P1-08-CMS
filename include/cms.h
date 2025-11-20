#ifndef CMS_H
#define CMS_H

/*
 * class management system (cms) module
 *
 * provides the main program structure including initialisation,
 * menu display, and main event loop for the interactive cms application.
 */

typedef enum {
  CMS_SUCCESS = 0,            // operation completed successfully
  CMS_ERROR_INIT,             // cms initialisation failed
  CMS_ERROR_DB_INIT,          // database initialisation failed
  CMS_ERROR_INVALID_ARGUMENT, // invalid argument provided
  CMS_ERROR_FILE_OPEN,        // failed to open file
  CMS_ERROR_FILE_IO           // file i/o operation failed
} CMSStatus;

// initialises the cms application and database
CMSStatus cms_init(void);

// displays the main menu options to the user
CMSStatus display_menu(void);

// runs the main interactive loop, processing user commands
CMSStatus main_loop(void);

// converts cms status code to readable string
const char *cms_status_string(CMSStatus status);

#endif
