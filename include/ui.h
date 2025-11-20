#ifndef UI_H
#define UI_H

#include "cms.h"

/**
 * displays the programme declaration from file
 * @return CMS_SUCCESS if successful, CMS_ERROR_FILE_OPEN if file cannot be opened
 */
CMSStatus ui_display_declaration(void);

/**
 * displays the main menu options from file
 * @return CMS_SUCCESS if successful, CMS_ERROR_FILE_OPEN if file cannot be opened
 */
CMSStatus ui_display_menu(void);

/**
 * displays an error message to the user
 * @param error_msg the error message to display (without "CMS: " prefix or newline)
 */
void ui_display_error(const char *error_msg);

#endif // UI_H
