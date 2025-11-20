#include "commands/command_utils.h"
#include "ui.h"
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
