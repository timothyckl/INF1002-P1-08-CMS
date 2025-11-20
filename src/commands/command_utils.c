#include "commands/command_utils.h"
#include "ui.h"
#include <ctype.h>
#include <stdio.h>

void cmd_wait_for_user(void) {
  char continue_buf[256];
  printf("\nPress Enter to continue...");
  (void)fgets(continue_buf, sizeof continue_buf, stdin);
  fflush(stdout);
}

OpStatus cmd_report_error(const char *error_msg, OpStatus status) {
  ui_display_error(error_msg);
  cmd_wait_for_user();
  return status;
}

int cmd_is_alphabetic(const char *str) {
  if (!str) {
    return 0;
  }

  // check each character is either alphabetic or a space
  for (const char *p = str; *p != '\0'; p++) {
    if (!isalpha((unsigned char)*p) && *p != ' ') {
      return 0;
    }
  }

  return 1;
}
