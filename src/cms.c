#include "cms.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#define DECLARATION_FILE_PATH "assets/declaration.txt"
#define MENU_FILE_PATH "assets/menu.txt"

typedef enum {
  OPEN_OP = 1,
  SHOW_ALL_OP,
  INSERT_OP,
  QUERY_OP,
  UPDATE_OP,
  DELETE_OP,
  SAVE_OP,
  EXIT_OP,
  INVALID_OP,
  OP_ERR
} Operation;

CMSStatus cms_init(int argc, char *argv[]) {
  // check command line arguments
  if (check_args(argc, argv) != 0) {
    return CMS_OP_FAILURE;
  }

  // print declaration
  int buf_size = 256;
  FILE *handle = get_file_handle(DECLARATION_FILE_PATH);

  if (handle == NULL) {
    return CMS_OP_FAILURE;
  }

  print_file_lines(handle, buf_size, false);

  // clear screen
  char buf[256];
  printf("\nPress Enter to continue...");
  (void)fgets(buf, sizeof buf, stdin);
  fflush(stdout);
  // NOTE: use "cls" for windows
  system("clear");

  return CMS_OP_SUCCESS;
}

CMSStatus display_menu(void) {
  int buf_size = 256;
  FILE *handle = get_file_handle(MENU_FILE_PATH);
  if (handle == NULL) {
    return CMS_OP_FAILURE;
  }
  print_file_lines(handle, buf_size, false);

  return CMS_OP_SUCCESS;
}

static Operation get_user_input(char *buf) {
  printf("Select an option: ");
  if (fgets(buf, sizeof buf, stdin) == NULL) {
    return OP_ERR;
  }
  fflush(stdout);
  putchar('\n');

  // convert inp_buf to int
  Operation op = atoi(buf);

  return op;
}

// TODO: i need a better name for this function ;-;
CMSStatus main_loop(int argc, char *argv[]) {
  CMSStatus status;

  status = cms_init(argc, argv);
  // process status

  char inp_buf[100];
  Operation usr_inp;

  do {
    status = display_menu();
    // process status

    usr_inp = get_user_input(inp_buf);

    // handle usr_inp
    switch (usr_inp) {
    case OPEN_OP:
      printf("fein\n");
      break;
    case SHOW_ALL_OP:
      printf("fweh\n");
      break;
    case INSERT_OP:
    case QUERY_OP:
    case UPDATE_OP:
    case DELETE_OP:
    case SAVE_OP:
    case EXIT_OP:
      printf("Goodbye!\n");
      break;
    default:
      printf("invalid operation type shi\n");
      break;
    }

  } while (usr_inp != EXIT_OP);

  return CMS_OP_SUCCESS;
}

const char *cms_status_string(CMSStatus status) {
  switch (status) {
  case CMS_OP_SUCCESS:
    return "operation completed successfully";
  case CMS_OP_FAILURE:
    return "operation failed to execute";
  case CMS_INVALID_ARG:
    return "invalid argument received";
  default:
    return "unknown error";
  }
}
