#ifndef COMMAND_UTILS_H
#define COMMAND_UTILS_H

#include "commands/command.h"

#define STUDENT_RECORDS_TABLE_INDEX 0
#define DEFAULT_DATA_FILE "data/P1_8-CMS.txt"
#define DEFAULT_FILE_MSG "No input received. Using default data file (%s).\n"

/**
 * waits for user to press enter
 */
void cmd_wait_for_user(void);

/**
 * reports cms error message, waits for user input, and returns status
 * note: error_msg should NOT include "CMS: " prefix or trailing newline
 */
OpStatus cmd_report_error(const char *error_msg, OpStatus status);

/**
 * validates that a string contains only alphabetic characters and spaces
 * returns: 1 if valid, 0 if invalid
 */
int cmd_is_alphabetic(const char *str);

#endif // COMMAND_UTILS_H
