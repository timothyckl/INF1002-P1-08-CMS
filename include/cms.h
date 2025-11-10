#ifndef CMS_H
#define CMS_H

typedef enum {
  CMS_OP_SUCCESS,
  CMS_OP_FAILURE,
  CMS_INVALID_ARG,
  // ...
} CMSStatus;

CMSStatus cms_init(int argc, char *argv[]);
CMSStatus display_menu(void);
CMSStatus main_loop(int argc, char *argv[]); 

#endif
