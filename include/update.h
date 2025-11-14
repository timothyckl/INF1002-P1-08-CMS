#ifndef UPDATE_H
#define UPDATE_H

#include "database.h"

/*
 * Handles an interactive UPDATE operation.
 * Prompts the user for:
 *  - student ID
 *  - which field to update (Name / Programme / Mark)
 *  - new value for that field
 *
 * Returns:
 *   DB_SUCCESS on successful update
 *   DB_ERROR_* on failure (not found, invalid data, etc.)
 *
 * This function ONLY does the logic; cms.c converts the DBStatus
 * into OperationStatus and prints any high-level menu messages.
 */
DBStatus cms_handle_update(StudentDatabase *db);

#endif
