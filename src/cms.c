#include "cms.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

#define DECLARATION_FILE_PATH "assets/declaration.txt"
#define MENU_FILE_PATH "assets/menu.txt"

// we define these enums here (and not in the header) because the will not be
// used anywhere else in the codebase
typedef enum {
  OPEN = 1,
  SHOW_ALL,
  INSERT,
  QUERY,
  UPDATE,
  DELETE,
  SAVE,
  EXIT,
} Operation;

typedef enum {
  OP_SUCCESS,
  OP_ERR,
  OP_INVALID,
} OperationStatus;

CMSStatus cms_init(int argc, char *argv[]) {
  // check command line arguments
  if (check_args(argc, argv) != 0) {
    return CMS_INVALID_ARG;
  }

  // print declaration
  int buf_size = 256;
  FILE *handle = get_file_handle(DECLARATION_FILE_PATH);

  if (handle == NULL) {
    return CMS_FILE_OPEN_ERR;
  }

  print_file_lines(handle, buf_size, false);

  // clear screen
  char buf[256];
  printf("\nPress Enter to continue...");
  (void)fgets(buf, sizeof buf, stdin);
  fflush(stdout);
  // NOTE: use "cls" for windows
  system("clear");

  return CMS_SUCCESS;
}

CMSStatus display_menu(void) {
  int buf_size = 256;
  FILE *handle = get_file_handle(MENU_FILE_PATH);
  if (handle == NULL) {
    return CMS_FILE_OPEN_ERR;
  }
  print_file_lines(handle, buf_size, false);

  return CMS_SUCCESS;
}

static OperationStatus get_user_input(char *buf, Operation *op) {
  printf("Select an option: ");
  if (fgets(buf, sizeof buf, stdin) == NULL) {
    return OP_ERR; // should be a CMS error instead
  }
  fflush(stdout);
  putchar('\n');

  // convert inp_buf to int
  // im kms... ts is super unsafe change to strtok and read the docs
  *op = atoi(buf);

  return OP_SUCCESS; // should be a CMS_SUCCESS instead
}

// CMS operations (not to be confused with db operations)
// are defined here. feel free to rename them and modify the function signatures
OperationStatus open() {
  // your code here
  printf("you selected open!\n");

  return OP_SUCCESS;
}

OperationStatus show_all() {
  // your code here
  printf("you selected show all!\n");

  return OP_SUCCESS;
}

OperationStatus insert() {
  // your code here
  printf("you selected insert!\n");

  return OP_SUCCESS;
}

OperationStatus query() {
  // your code here
  printf("you selected query!\n");

  return OP_SUCCESS;
}

OperationStatus update() {
  // your code here
  printf("you selected update!\n");

  return OP_SUCCESS;
}

OperationStatus delete() {
  // your code here
  printf("you selected delete!\n");

  return OP_SUCCESS;
}

OperationStatus save() {
  // your code here
  printf("you selected save!\n");

  return OP_SUCCESS;
}

static OperationStatus operation_router(Operation op) {
  OperationStatus status;

  switch (op) {
  case OPEN:
    status = open();
    return status;
  case SHOW_ALL:
    status = show_all();
    return status;
  case INSERT:
    status = insert();
    return status;
  case QUERY:
    status = query();
    return status;
  case UPDATE:
    status = update();
    return status;
  case DELETE:
    status = delete();
    return status;
  case SAVE:
    status = save();
    return status;
  case EXIT:
    printf("Goodbye!\n");
    return OP_SUCCESS;
  default:
    printf("invalid operation type shi\n");
    return OP_INVALID;
  }
}

// TODO: i need a better name for this function ;-;
CMSStatus main_loop(int argc, char *argv[]) {
  CMSStatus status;

  status = cms_init(argc, argv);
  // process status

  char inp_buf[100];
  Operation op;

  do {
    status = display_menu();
    // process status

    get_user_input(inp_buf, &op);
    operation_router(op);

  } while (op != EXIT);

  return CMS_SUCCESS;
}

// static const char *op_string(CMSStatus status) {
//   switch (status) {
//   case CMS_SUCCESS:
//     return "operation completed successfully";
//   case CMS_INIT_FAILURE:
//     return "failed initialisation step";
//   case CMS_INVALID_ARG:
//     return "invalid argument received";
//   case CMS_FILE_OPEN_ERR:
//     return "failed to retrieve file handle";
//   case CMS_FILE_IO_ERR:
//     return "failed to complete operation on file";
//   default:
//     return "unknown error";
//   }
// }

const char *cms_status_string(CMSStatus status) {
  switch (status) {
  case CMS_SUCCESS:
    return "operation completed successfully";
  case CMS_INIT_FAILURE:
    return "failed initialisation step";
  case CMS_INVALID_ARG:
    return "invalid argument received";
  case CMS_FILE_OPEN_ERR:
    return "failed to retrieve file handle";
  case CMS_FILE_IO_ERR:
    return "failed to complete operation on file";
  default:
    return "unknown error";
  }
}
