#include <string>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#include "Arguments.h"

/*!
  @brief Method to process command line arguments.  Pass main's argc
    and argv arguments to this function.
  @param[in] argc  Number of arguments
  @param[in] argv  List of char string arguments
*/
int OptionsProcess::processArgs(int argc, char** argv) {
  int c;
  
  while (1) {
    static struct option long_options[] =
      {
	/* These options set a flag. */
	{"verbose", no_argument,       &verbose_flag, 1},
	{"guiAll",  no_argument,       &guiAll, 1},
	{"brief",   no_argument,       &verbose_flag, 0},
	/* These options don't set a flag.
	   We distinguish them by their indices. */
	{"help",    no_argument,       0, 'h'},
	{"wpiImages", no_argument,     0, 'w'},
	{"file",    required_argument, 0, 'f'},
	{0, 0, 0, 0}
      };
    /* getopt_long stores the option index here. */
    int option_index = 0;
    
    c = getopt_long (argc, argv, "f:h",
		     long_options, &option_index);
    
    /* Detect the end of the options. */
    if (c == -1)
      break;
    
    switch (c) {
    case 0:
      /* If this option set a flag, do nothing else now. */
      if (long_options[option_index].flag != 0)
	break;
      printf ("option %s", long_options[option_index].name);
      if (optarg)
	printf (" with arg %s", optarg);
      printf ("\n");
      break;
      
    case 'f': {
      /* This option is to process a file.  Now look at the filename and determine
       * if it is a .jpg file or an .mpg file.  I.e. if it is not a .jpg then it is
       * a .mpg
       */
      string s(optarg);
      int jpeg =  s.find(".jpg");
      if (jpeg != s.npos) {
	processJpegFile = true;
	processVideoFile = false;
	processCamera = false;
      } else {
	processJpegFile = false;
	processVideoFile = true;
	processCamera = true;
      }
      fileName = optarg;
      break;
    }
    case 'w':
      // Flop the green and red planes
      BLUE_PLANE = 0;
      GREEN_PLANE = 2;
      RED_PLANE = 1;
      break;
    case 'h':
      printf("Vision [-h|--help]          : Print this message\n");
      printf("       [--guiAll]           : Display all debugging windows\n");
      printf("       [-w|--wpiImages]     : Process wpi type images (red targets)\n");
      printf("       [-f|--file] filename : Process a mjpg video or jpeg image\n");
      exit(0);
      break;
      
    case '?':
      // getopt_long already printed an error message.
      break;
      
    default:
      abort ();
    }
  }
}
