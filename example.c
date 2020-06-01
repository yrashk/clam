#include <stdio.h>
#include <stdbool.h>

#include "clam.h"

int main(int argc, char **argv)
{
   for (int a = 0; a < argc; a++) {
           const char *arg = argv[a];
           clam_match_result_t i = 0;
           if ((i = clam_match_posix_option(arg, "h")) || (i = clam_match_posix_long_option(arg, "-help"))) {
                   printf("Usage: example [option]\n");
                   printf("  -h | --help This help information\n");
                   printf("  -lname | -l name | --link name | --link=name | -link name | --link=name\n");
                   printf("  /f [value]\n");
                   return 0;
           }

           bool longopt;
           if (
               (longopt = true, i = clam_match_posix_long_option(arg, "link")) || 
               (longopt = true, i = clam_match_posix_long_option(arg, "-link")) || 
               (longopt = false, i = clam_match_posix_option(arg, "l"))) {
                   clam_match_result_t eq = 0;
                   if (longopt && !clam_match_end(arg + i) && !clam_match_char(arg + i, '=')) {
                           printf("invalid trailer %s at %d in %s\n", arg + i, i, arg);
                           return 1;
                   }
                   if (longopt) {
                           i += eq = clam_match_char(arg + i, '=');
                   }
                   arg += i;
                   if (!eq && clam_match_end(arg)) {
                           arg = argv[++a];
                           if (a == argc) {
                                   printf("link requires an argument\n");
                                   return 1;
                           }
                   }
                   printf("linking with %s\n", arg);
                   continue;
           }

        if (i = clam_match_windows_switch(arg, "Ff")) {
                arg = argv[++a];
                if (a < argc) {
                        printf("doing something with with %s\n", arg);
                } else {
                        printf("using a default with /f");
                }
                continue;
        }

   }
}
