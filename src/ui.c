#include "ui.h"
#include "utils.h"
#include <stdio.h>

#define DECLARATION_FILE_PATH "assets/declaration.txt"
#define MENU_FILE_PATH "assets/menu.txt"

CMSStatus ui_display_declaration(void) {
  int buf_size = 256;
  FILE *handle = get_file_handle(DECLARATION_FILE_PATH);

  if (handle == NULL) {
    return CMS_ERROR_FILE_OPEN;
  }

  print_file_lines(handle, buf_size, false);

  return CMS_SUCCESS;
}

CMSStatus ui_display_menu(void) {
  int buf_size = 256;
  FILE *handle = get_file_handle(MENU_FILE_PATH);

  if (handle == NULL) {
    return CMS_ERROR_FILE_OPEN;
  }

  print_file_lines(handle, buf_size, false);

  return CMS_SUCCESS;
}

void ui_display_error(const char *error_msg) { printf("CMS: %s\n", error_msg); }
