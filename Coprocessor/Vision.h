/* \\            //   { }              { }     _____          __
 *  \\          //             ====          //     \\    ||//  \\ 
 *   \\        //     |||    //        |||  //       \\   ||     ||
 *    \\      //      |||    \\===     |||  ||  [ ]  ||   ||     ||
 *     \\    //       |||         \\   |||  ||  [ ]  ||   ||     ||
 *      \\  //        |||         ||   |||  \\       //   ||     ||
 *       \\//         |||    ====//    |||   \\_____//    ||     ||
 *
 * This is the Nashoba Robotics Team's Computer Vision program for the 2011 -
 * 2012 season. We implement the OpenCV computer vision library in this code.
 * 
 * Our primary objective is to find the distance between the camera (the robot)
 * and the target. We do this by shining a green light at a target with
 * reflective tape (FIRST standard) and capture that light using the camera. In
 * the program, we filter the green light and if it is close to the shape of a
 * rectangle, it is a good target. 
 *
 * From there, we calculate the angle we are at relative to the robot. The
 * angle goes up as we turn away from the target. When the camera is directly
 * facing the target, we are at an optimal angle. We calculate the angle using
 * trigonometric functions. We calculate the center of the target and from
 * there, we calculate the angle using the center coordinate and half the vertical
 * data in the picture. We stick these values into an equation we generated in
 * LibreOffice after taking raw data.
 *
 * Finally, we calculate the needed tension to make a shot by using the
 * distance from the target. We again generated an equation using raw data in
 * LibreOffice.
 *
 * Enjoy!
 *
 * Nashoba Robotics Programming Team
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * FreeBSD License:
 *
 * Copyright (c) <2012>, <Nashoba Robotics Programming Team>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met: 
 * 
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies, 
 * either expressed or implied, of the FreeBSD Project.
 *
 */ 


#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <string>
#include <cstdio>
#include <getopt.h>

#define DEBUG_TEXT              1
#define BUFFERSIZE              1024

#define CRIO_NETWORK               // Normal case
//#define WPI_IMAGES               // For debugging with WPI images

// Image Color Plane definitions
int BLUE_PLANE  = 0;
int GREEN_PLANE = 1;
int RED_PLANE   = 2;

cv::Mat src;                            // The source image matrix

bool pause_image = false;
int thresh = 130;                   // Defines the threshold level to apply to image
int thresh_block_size = 23;         // Defines the threshold block size to apply to image
int max_thresh_block_size = 255;    // Defines the threshold block size to apply to image
int max_thresh = 255;               // Max threshold for the trackbar
int poly_epsilon = 10;              // Epsilon to determine reduce number of poly sides
int max_poly_epsilon = 50;          // Max epsilon for the trackbar

int minsize = 500;                  // Polygons below this size are rejected
int max_minsize = 10000;            // Max polygon size for the trackbar

int dilation_elem = 0;              // The element type dilate/erode with
int const max_elem = 2;             // Max trackbar element type
int dilation_size = 4;              // The size of the element to dilate/erode with
int const max_kernel_size = 21;     // Max trackbar kernel dialation size

int erode_count = 1;                // The number of times to erode the image
int erode_max = 20;                 // Max number of times to erode on trackbar

int target_width_inches = 24;       // Width of a physical target in inches
int target_height_inches = 16;      // Height of a physical target in inches

/* The command line options processing class
 * Processes command line options using getopt_long()
 */ 
class OptionsProcess 
{
    public:
        int processCamera;
        int guiAll;
        int verbose_flag;
        int processVideoFile;
        int processJpegFile;
        char *fileName;

        OptionsProcess(): 
            processCamera(true), 
            guiAll(false), 
            processVideoFile(false),
		    processJpegFile(false),
		    fileName(0) 
            {}

        int processArgs(int argc, char** argv)
        {
            int get_longOptions;
    
             while (1) 
             {
                /* This struct contains all the command line arguments
                 * we want to filter. So far, we have arguments for our debug
                 * mode.
                 */ 
                static struct option long_options[] =
	            {
	                /* These options set a flag */
	                {"verbose",     no_argument,       &verbose_flag, 'v'},     // The verbosity flag
	                {"guiAll",      no_argument,       &guiAll, 'g'},           // The debug flag showing all of the windows
	                {"brief",       no_argument,       &verbose_flag, 'b'},     // The anti-verbosity flag
	                
                    /* These options don't set a flag.
	                We distinguish them by their indices */
	                {"help",        no_argument,        0, 'h'},                // The help flag
	                {"wpiImages",   no_argument,        0, 'w'},                // The WPI image processing flag
	                {"file",        required_argument,  0, 'f'},                // The jpeg file loading flag
	                {0, 0, 0, 0}                                                // The default, no options flag
	            };
      
                /* getopt_long stores the option index here. */
                int option_index = 0;
      
                get_longOptions = getopt_long (argc, argv, "f:h", long_options, &option_index);
      
                /* Detect the end of the options. */
                if (get_longOptions == -1) break;
      
                 switch (get_longOptions) 
                 {
                    case 0:
	                /* If this option sets a flag, do nothing else now */
	                    if (long_options[option_index].flag != 0)    break;

	                    printf ("option %s", long_options[option_index].name);

	                    if (optarg)    printf (" with arg %s", optarg);

	                    printf ("\n");  
                        
                        break;
	
                    case 'f': 
                    {
                        std::string optstring(optarg);

                        // Find ".jpg" in the argument string
	                    int jpeg =  optstring.find(".jpg");

                        /* If ".jpg" is found in the file name,
                         * set the processJpegFile boolean to true
                         * and the processVideoFile and processCamera
                         * to booleans to false
                         */ 
                        if (jpeg != optstring.npos)
                        {
	                        processJpegFile = true;
	                        processVideoFile = false;
	                        processCamera = false;
	                    } 
                        else // Else set the opposite
                        {
	                        processJpegFile = false;
	                        processVideoFile = true;
                            processCamera = true;
	                    }
	                    
                        // File name is equal to the file name inputted 
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
                    
                    // Print the help menu
	                printf("Usage: ./Vision\n"); 
                    printf("[-h|--help]:\tPrint this message\n");
	                printf("[--guiAll]:\tDisplay all debugging windows\n");
	                printf("[-w|--wpiImages]:\tProcess WPI type images (red targets)\n");
	                printf("[-f|--file] filename : Process a mjpg video or jpeg image\n");
	                
                    exit(0);
	                break;
	
                    case '?':
	                
                    /* getopt_long already printed an error message. */
	                break;
	
                    default: // The default action is to exit
	                abort ();
                 }
             }
        }
};

OptionsProcess options;
