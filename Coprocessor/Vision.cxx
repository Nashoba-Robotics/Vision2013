#include "./Vision.h"

#include <iostream>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <cstring>

#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace cv;
using namespace std;

// A timer using the timespec struct
timespec diff(timespec start, timespec end)
{
    timespec temp;

    if( (end.tv_nsec - start.tv_nsec) < 0 ) 
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    } 
    else 
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }

    return temp;
}

 /// Function header
void processImageCallback(int, void* );

// This function is used to calculate and display histogram of an image
void calcHistogram(Mat &src) 
{
    /// Separate the image in 3 places ( R, G and B )
    vector<Mat> rgb_planes;   // Create a vector of Mat objects
    split( src, rgb_planes ); // Seperate the source image into 3 channels and put each channel into the rgb_planes vector
  
    /// Establish the number of bins
    int histSize = 255;
  
    /// Set the ranges for R,G, and B
    float range[] = { 10, 255 } ;        // The range for RGB is 10 - 255
    const float* histRange = { range };  // creat a floating point histogram range constant pointer
  
    bool uniform = true; bool accumulate = false; // Booleans that need to be set for use in calcHist()
  
    Mat r_hist, g_hist, b_hist;
  
    /// Compute the histograms:
    calcHist( &rgb_planes[0], 1, 0, Mat(), r_hist, 1, &histSize, &histRange, uniform, accumulate );  // The Red Histogram
    calcHist( &rgb_planes[1], 1, 0, Mat(), g_hist, 1, &histSize, &histRange, uniform, accumulate );  // The Green Histogram
    calcHist( &rgb_planes[2], 1, 0, Mat(), b_hist, 1, &histSize, &histRange, uniform, accumulate );  // The Blue Histogram
  
    // Draw the histograms for R, G, and B
    int hist_w = 400; int hist_h = 400;
    int bin_w = cvRound( (double) hist_w / histSize );
  
    Mat histImage( hist_w, hist_h, CV_8UC3, Scalar( 0,0,0 ) );
  
    /// Normalize the result to [ 0, histImage.rows ]
    normalize(r_hist, r_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(g_hist, g_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(b_hist, b_hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
  
    /// Draw for each channel
    for( int i = 1; i < histSize; i++ ) 
    {
        line(histImage, Point( bin_w * (i - 1), hist_h - cvRound(r_hist.at<float>(i - 1)) ) ,
	    Point( bin_w*(i), hist_h - cvRound(r_hist.at<float>(i)) ),
	    Scalar( 255, 0, 0), 2, 8, 0  );

        line(histImage, Point( bin_w * (i - 1), hist_h - cvRound(g_hist.at<float>(i - 1)) ) ,
	    Point( bin_w*(i), hist_h - cvRound(g_hist.at<float>(i)) ),
	    Scalar( 0, 255, 0), 2, 8, 0  );

        line(histImage, Point( bin_w * (i - 1), hist_h - cvRound(b_hist.at<float>(i - 1)) ) ,
	    Point( bin_w*(i), hist_h - cvRound(b_hist.at<float>(i)) ),
	    Scalar( 0, 0, 255), 2, 8, 0  );
    }
  
    // Display the histogram
    namedWindow("Histogram", CV_WINDOW_AUTOSIZE );
    imshow("Histogram", histImage );
}


void createGuiWindows() 
{
  /// Create all Windows
  if (options.guiAll) 
  {
    namedWindow("Source", CV_WINDOW_AUTOSIZE );
    namedWindow("Color", CV_WINDOW_AUTOSIZE );
    namedWindow("Blur", CV_WINDOW_AUTOSIZE );
    namedWindow("Dilate", CV_WINDOW_AUTOSIZE );
    namedWindow("Threshold", CV_WINDOW_AUTOSIZE );
    namedWindow("Contours", CV_WINDOW_AUTOSIZE );
    namedWindow("Polygon", CV_WINDOW_AUTOSIZE );
    namedWindow("PrunedPolygon", CV_WINDOW_AUTOSIZE );
    namedWindow("Targets", CV_WINDOW_AUTOSIZE );
    namedWindow("Final", CV_WINDOW_AUTOSIZE );
    
    createTrackbar("minsize", "PrunedPolygon", &minsize, max_minsize, processImageCallback);
    createTrackbar("threshold", "Threshold", &thresh, max_thresh, processImageCallback);
    createTrackbar("block size", "Threshold", &thresh_block_size, max_thresh_block_size, processImageCallback);
    createTrackbar("Poly epsilon", "Polygon", &poly_epsilon, max_poly_epsilon, processImageCallback);
    createTrackbar("Element: 0:Rect 1:Cross 2:Ellipse", "Dilate", &dilation_elem, max_elem, processImageCallback);
    createTrackbar("Kernel size:\n 2n+1", "Dilate", &dilation_size, max_kernel_size, processImageCallback);
    createTrackbar("Erode", "Dilate", &erode_count, erode_max, processImageCallback);
  }
}

// Determine if a polygon contains another polygon

/* These are targets  
 * (Outer rectangle is the outer part of the reflective tape
 * inner rectangle is the inner part of the reflective tape).
 */

bool rectContainsRect(int polygon_pt, const vector<vector<Point> >&prunedPoly) 
{
  for (int j = 0; j < prunedPoly.size(); j++) 
  {
    // Don't check against yourself
    if (polygon_pt == j) continue;
    
    if (pointPolygonTest(prunedPoly[polygon_pt], prunedPoly[j][0], false) > 0) return true;
  }

  return false;
}

// A Target height enumerated type
typedef enum {
  TARGET_HEIGHT_UNKNOWN = 0,
  TARGET_HEIGHT_HIGH,
  TARGET_HEIGHT_MIDDLE,
  TARGET_HEIGHT_MIDDLE_RIGHT,
  TARGET_HEIGHT_MIDDLE_LEFT,
  TARGET_HEIGHT_LOW,
  TARGET_HEIGHT_MIDDLE_COMBINED
} TargetType;

// Get the type of Target (one of the strings from the TargetType enum)
const char *getTargetTypeString(TargetType targetType) 
{
  switch (targetType) 
  {
    case TARGET_HEIGHT_HIGH:            return "High";
    case TARGET_HEIGHT_MIDDLE:          return "Middle";
    case TARGET_HEIGHT_MIDDLE_RIGHT:    return "Middle Right";
    case TARGET_HEIGHT_MIDDLE_LEFT:     return "Middle Left";
    case TARGET_HEIGHT_LOW:             return "Low";
    case TARGET_HEIGHT_MIDDLE_COMBINED: return "Middle Combined";
    default:                            return "Unknown";
  }
}

// Struct containing information about the vision targets
struct TargetData 
{
    TargetData() 
    {
        centerX = 0;
        centerY = 0;
        sizeX = 0;
        sizeY = 0;
        distanceX = 0;
        distanceY = 0;
        angleX = 0;
        tension = 0;
        targetType = TARGET_HEIGHT_UNKNOWN;
        valid = 0;
    }
    
    vector<Point2f> points;

    float centerX;
    float centerY;
    float sizeX;
    float sizeY;
    float distanceX;
    float distanceY;
    float angleX;
    float tension;
    
    TargetType targetType;
    
    float valid;
};

// Struct containing target groups based on location of target
struct TargetGroup 
{
    TargetData high;
    TargetData middleLeft;
    TargetData middleRight;
    TargetData low;
    TargetData selected;
};

// Computes the offset of the Low Y values
float computeLowYOffset(float distance) 
{
    return -0.0009 * pow(distance, 2) + .3759 * (distance) + 83.232;
}

// Computes the offset of the Middle Y values
float computeMidYOffset(float distance) 
{
  return -0.0034 * pow(distance, 2) + 1.6519 * (distance) - 127.13;
}

// Computes the offset of the High Y values
float computeHighYOffset(float distance) 
{
  // We can't see the high target
  return 232;
}

// Is Height within x% of baseline?
bool approximateHeight(float value, float baseline) 
{
    return (baseline * 1.5 > value) && (baseline * .5 < value);
}

/* Use the distance from the target to calculate
 * the tension of the cam needed to make a shot
 */  
float convertDistanceToTension(float distance) 
{
  return ((1.2821 * distance) + 277.8 + 52);
}

/* Compute the type of target the camera is receving 
 * based on the Y offsets and the approximate height
 */ 
void computeTargetType(TargetData &target) 
{
    if ( approximateHeight( computeLowYOffset(target.distanceY), target.centerY ) )     
    {
      target.targetType = TARGET_HEIGHT_LOW;
    } 
    else if ( approximateHeight( computeMidYOffset(target.distanceY), target.centerY ) ) 
    {
    target.targetType = TARGET_HEIGHT_MIDDLE;
    } 
    else if ( approximateHeight( computeHighYOffset(target.distanceY), target.centerY ) ) 
    {
    target.targetType = TARGET_HEIGHT_HIGH;
    } 
    else 
    {
    target.targetType = TARGET_HEIGHT_UNKNOWN;
    }
}

/* Figure out which targets are of a certain target 
 * type and put them in a targetIndices vector.
 */ 
void getTargetsType(vector<TargetData> &targets, vector<int> &targetIndices, TargetType targetType) 
{
    for (int i = 0; i < targets.size(); i++) 
    {
        if (targets[i].targetType == targetType) 
        {
            targetIndices.push_back(i);
        }
    }
}


void getTargetGroup(Mat &image, vector<TargetData> &targets, TargetGroup &targetGroup) 
{
    vector<int> highTargetIndices;
    getTargetsType(targets, highTargetIndices, TARGET_HEIGHT_HIGH);
    
    if (highTargetIndices.size()) 
    {
        targetGroup.high = targets[highTargetIndices[0]];
    }

    vector<int> lowTargetIndices;
    getTargetsType(targets, lowTargetIndices, TARGET_HEIGHT_LOW);
  
    if (lowTargetIndices.size()) 
    {
        targetGroup.low = targets[lowTargetIndices[0]];
    }
  
    vector<int> middleTargetIndices;
    getTargetsType(targets, middleTargetIndices, TARGET_HEIGHT_MIDDLE);
    
    switch (middleTargetIndices.size()) 
    {
    
        // We have two middle targets
        case 2:
            if (targets[middleTargetIndices[0]].centerX < targets[middleTargetIndices[1]].centerX) 
            {
                targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
                targetGroup.middleLeft = targets[middleTargetIndices[0]];
                targets[middleTargetIndices[1]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
                targetGroup.middleRight = targets[middleTargetIndices[1]];
            } 
            else 
            {
                targets[middleTargetIndices[1]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
                targetGroup.middleLeft = targets[middleTargetIndices[1]];
                targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
                targetGroup.middleRight = targets[middleTargetIndices[0]];
            }
        
            break;
    
        // We only have one middle target
        case 1:
            // We also have a valid high or low
            if (targetGroup.high.valid || targetGroup.low.valid) 
            {
                float centerX;
      
                // Get the center target x location
                if (targetGroup.high.valid) 
                {
	                centerX = targetGroup.high.centerX;
                } 
                else 
                {
	                centerX = targetGroup.low.centerX;
                }

                // Based on the center target location, determine if the middle targets
                // are left or right targets
                if (targets[middleTargetIndices[0]].centerX < centerX) 
                {
	                targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
	                targetGroup.middleLeft = targets[middleTargetIndices[0]];
                } 
                else 
                {
	                targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
	                targetGroup.middleRight = targets[middleTargetIndices[0]];
                }

            /* If we don't have any center targets, assume that they are they are out of the
             * image.  (i.e. don't assume that they are hidden)
             */ 
            } 
            else 
            {
                if (targets[middleTargetIndices[0]].centerX > image.cols/2.0) 
                {
	                targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
	                targetGroup.middleLeft = targets[middleTargetIndices[0]];
                }           
                else 
                {
	                targets[middleTargetIndices[0]].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
	                targetGroup.middleRight = targets[middleTargetIndices[0]];
                }
            }
            
            break;
    }

    // Now determine the location of the center target, given other target data
    if (targetGroup.high.valid) 
    {
        targetGroup.selected = targetGroup.high;
    } 
    else if (targetGroup.low.valid) 
    {
        targetGroup.selected = targetGroup.low;
    } 
    else if (targetGroup.middleLeft.valid && targetGroup.middleRight.valid) 
    {
        targetGroup.selected.centerX = (targetGroup.middleLeft.centerX + targetGroup.middleRight.centerX)/2;
        targetGroup.selected.centerY = (targetGroup.middleLeft.centerY + targetGroup.middleRight.centerY)/2;
        targetGroup.selected.sizeX = (targetGroup.middleLeft.sizeX + targetGroup.middleRight.sizeX)/2;
        targetGroup.selected.sizeY = (targetGroup.middleLeft.sizeY + targetGroup.middleRight.sizeY)/2;
        targetGroup.selected.distanceX = (targetGroup.middleLeft.distanceX + targetGroup.middleRight.distanceX)/2;
        targetGroup.selected.distanceY = (targetGroup.middleLeft.distanceY + targetGroup.middleRight.distanceY)/2;
        targetGroup.selected.angleX = (targetGroup.middleLeft.angleX + targetGroup.middleRight.angleX)/2;
        targetGroup.selected.targetType = TARGET_HEIGHT_MIDDLE_COMBINED;
        targetGroup.selected.valid = true;
    } 
    else if (targetGroup.middleLeft.valid) 
    {
        targetGroup.selected = targetGroup.middleLeft;
    } 
    else if (targetGroup.middleRight.valid) 
    {
        targetGroup.selected = targetGroup.middleRight;
    } 
}

/* This boolean determines whether the 
 * target that has been acquired is the
 * best one
 */ 
bool getBestTarget(vector<TargetData> &targets, TargetData &target) 
{
    for (int i = 0; i < targets.size(); i++) 
    {
        if (targets[i].targetType == TARGET_HEIGHT_HIGH) 
        {
            target = targets[i];
        return true;
        }
    }
    
    for (int i = 0; i < targets.size(); i++) 
    {
        if (targets[i].targetType == TARGET_HEIGHT_LOW) 
        {
            target = targets[i];
            return true;
        }
    }
  
    vector<TargetData> middleTargets;
  
    for (int i=0; i < targets.size(); i++) 
    {
        if (targets[i].targetType == TARGET_HEIGHT_MIDDLE) 
        {
            middleTargets.push_back(targets[i]);
        }
    }
  
    switch ( middleTargets.size() ) 
    {
        case 2:
            if (targets[0].centerX < targets[1].centerX) 
            {
                targets[0].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
                targets[1].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
            } 
            else 
            {
                targets[1].targetType = TARGET_HEIGHT_MIDDLE_LEFT;
                targets[0].targetType = TARGET_HEIGHT_MIDDLE_RIGHT;
            }
    
            target.centerX = (targets[0].centerX + targets[1].centerX)/2;
            target.centerY = (targets[0].centerY + targets[1].centerY)/2;
            target.sizeX = (targets[0].sizeX + targets[1].sizeX)/2;
            target.sizeY = (targets[0].sizeY + targets[1].sizeY)/2;
            target.distanceX = (targets[0].distanceX + targets[1].distanceX)/2;
            target.distanceY = (targets[0].distanceY + targets[1].distanceY)/2;
            target.angleX = (targets[0].angleX + targets[1].angleX)/2;
            target.targetType = TARGET_HEIGHT_MIDDLE_COMBINED;
            return true;
        case 1:
            target = targets[0];
            return true;
    }
    
    return false;
}

// For each of target compute size, distance, angle
void getTargetData(Mat &image, const vector<vector<Point2f> >&targetQuads, vector<TargetData> &targets) 
{
    for (int i = 0; i < targetQuads.size(); i++) 
    {
        TargetData target;
        target.points = targetQuads[i];
        target.valid = true;
        float centerX = 0;
        float centerY = 0;
        
        for (int j = 0; j < target.points.size(); j++) 
        {
            float x = target.points[j].x;
            float y = target.points[j].y;
            centerX += x;
            centerY += y;
        }
        
        target.centerX = centerX / 4.0;
        target.centerY = centerY / 4.0;
        
        float x1 = ( abs( target.points[0].x - target.points[1].x ) + abs( target.points[2].x - target.points[3].x ) ) / 2.0;
        
        float x2 = ( abs( target.points[1].x - target.points[2].x ) + abs( target.points[3].x - target.points[0].x ) ) / 2.0;
        
        float sizeX = max(x1, x2);

        float y1 = ( abs( target.points[0].y - target.points[1].y ) + abs( target.points[2].y - target.points[3].y ) ) / 2.0;
        
        float y2 = ( abs( target.points[1].y - target.points[2].y ) + abs( target.points[3].y - target.points[0].y ) ) / 2.0;
        
        float sizeY = max(y1, y2);

        target.sizeX = sizeX;
        target.sizeY = sizeY;

        /* Distance to target was obtained from distance to target measurements
         * that were trend line fit in LibreOffice
         */ 
        target.distanceX = 11263 * pow(sizeX, -1.061);
        target.distanceY = 7239 * pow(sizeY, -1.025);
    
        // Angle per pixel is based on the camera perameters
        target.angleX = 0.160943017 * ((image.cols / 2) - target.centerX) + 2.3;
        
	computeTargetType(target);

        targets.push_back(target);
    }
}

// Print out information about the targets to the console
void printTargets(vector<TargetData> &targets) 
{
    printf("----------------------------------------------------------------\n");
    for (int i = 0; i < targets.size(); i++) 
    {
        printf("Poly Points[");
        
        for (int j = 0; j < targets[i].points.size(); j++) 
        {
            printf("(%f, %f) ", targets[i].points[j].x, targets[i].points[j].y);
        }   
        
        printf("] ");
        printf("Center (%f, %f) ", targets[i].centerX, targets[i].centerY);
        printf("Size (%f, %f) \n", targets[i].sizeX, targets[i].sizeY);
    }
}

void intersection(Vec4f &line1Params, Vec4f &line2Params, Point2f &targetQuads2f) 
{
    /* Create 2 points from the line parameters vectors, 2 points from the
     * addition of those vectors, and 2 direction vectors based on the line
     * parameters.
     */ 
    Point2f point1(line1Params[2], line1Params[3]);
    Point2f point3(line2Params[2], line2Params[3]);
    Point2f direction_vec1(line1Params[0], line1Params[1]);
    Point2f direction_vec3(line2Params[0], line2Params[1]);
    Point2f point2 = point1 + direction_vec1;
    Point2f point4 = point3 + direction_vec3;


    
    double cross = ( (point1.x - point2.x) * (point3.y - point4.y) )  - ( (point1.y - point2.y) * (point3.x - point4.x) );
    
    if ( abs(cross) <  1e-8 ) // 1e-8 is the epsilon or "close enough" factor
    {
        printf("Bad line!\n");
        exit(1);
    }

    // Calculate the intersection point
    targetQuads2f.x = ( ( (point1.x * point2.y) - (point1.y * point2.x) ) * (point3.x - point4.x) - 
        (point1.x - point2.x) * ( (point3.x * point4.y) - (point3.y * point4.x) ) ) / cross;
    
    targetQuads2f.y = ( (point1.x * point2.y - point1.y * point2.x) * (point3.y - point4.y) - 
        (point1.y - point2.y) * (point3.x * point4.y - point3.y * point4.x) ) / cross;
}


void refineCorners(
            vector<vector<Point> >&targetQuads,
            vector<vector<Point> >&targetHulls,
		    vector<vector<Point2f> >&targetQuads2f,
		    vector<vector<Point> >&targetQuads2fi) 
{
    Size winSize(7,7);
    Size zeroZone(-1,-1);
    TermCriteria criteria(TermCriteria::COUNT, 30, 0);
    for (int i=0; i < targetQuads.size(); i++) 
    {
        for (int j=0; j < targetQuads[i].size(); j++) 
        {
            targetQuads2f[i].push_back(targetQuads[i][j]);
        }

#if 1
        // Find first element
        int k = 0;
    
        while (targetQuads[i][0] != targetHulls[i][k]) 
        {
            if ( k > targetHulls[i].size() ) 
            {
	            printf("Error %d", __LINE__);
	            exit(0);
	            return;
            }
        
            k++;
        }
    
        rotate( targetHulls[i].begin(), targetHulls[i].begin() + k, targetHulls[i].end() );
    
        vector<Point> line1;
        vector<Point> line2;
        vector<Point> line3;
        vector<Point> line4;
    
        k = 0;
    
        while (targetQuads[i][1] != targetHulls[i][k]) 
        {
            line1.push_back(targetHulls[i][k]);
            k++;
        
            if (k == targetHulls[i].size()) 
            {
	        printf("Error %d", __LINE__);
	        return;
            }
        }
     
        line1.push_back(targetHulls[i][k]);
    
        while (targetQuads[i][2] != targetHulls[i][k]) 
        {
            line2.push_back(targetHulls[i][k]);
            k++;
        
            if (k == targetHulls[i].size()) 
            {
	            printf("Error %d", __LINE__);
	            return;
            }
        }   
    
        line2.push_back(targetHulls[i][k]);
    
        while (targetQuads[i][3] != targetHulls[i][k]) 
        {
            line3.push_back(targetHulls[i][k]);
            k++;
        
            if (k == targetHulls[i].size()) 
            {
	            printf("Error %d", __LINE__);
	            return;
            }
        }
    
        line3.push_back(targetHulls[i][k]);
    
        while ( k < targetHulls[i].size() ) 
        {
            line4.push_back(targetHulls[i][k]);
            k++;
        }
    
        line4.push_back(targetHulls[i][0]);
    
        Vec4f line1Params, line2Params, line3Params, line4Params;
    
        fitLine(line1, line1Params, CV_DIST_L2, 0, 0.01, 0.01);
        fitLine(line2, line2Params, CV_DIST_L2, 0, 0.01, 0.01);
        fitLine(line3, line3Params, CV_DIST_L2, 0, 0.01, 0.01);
        fitLine(line4, line4Params, CV_DIST_L2, 0, 0.01, 0.01);
        
        intersection(line4Params, line1Params, targetQuads2f[i][0]);
        intersection(line1Params, line2Params, targetQuads2f[i][1]);
        intersection(line2Params, line3Params, targetQuads2f[i][2]);
        intersection(line3Params, line4Params, targetQuads2f[i][3]);
#endif
    
        for (int j = 0; j < targetQuads[i].size(); j++) 
        {
            targetQuads2fi[i].push_back(targetQuads2f[i][j]);
        }
    }
}

// Send a message about the targets to the cRIO
int sendMessage(float distance, float angle, float tension) 
{
    char sendbuffer[BUFFERSIZE];
    struct sockaddr_in crio_addr;
    int _socket;
    unsigned int addr_size;

    // Create a socket
    if ( ( _socket = socket(PF_INET, SOCK_DGRAM, 0) ) < 0 ) 
    {
        perror("Socket");
        return -1;
    } 
  
    crio_addr.sin_family = AF_INET;
    crio_addr.sin_port = htons(500);
    
    // Store 10.17.68.2 in crio_addr
    if ( inet_aton("10.17.68.2", &crio_addr.sin_addr) == 0 ) 
    {
        perror("cRIO_Addr");
        return -1;
    }

    /* Put the data into a formatted string and send it to the cRIO @
     * 10.17.68.2
     */ 
    sprintf(sendbuffer, "Distance=%f:Angle=%f:Tension=%f", distance, angle, tension);
    sendto( _socket, sendbuffer, strlen(sendbuffer) + 1, 0, (struct sockaddr*) &crio_addr, sizeof(crio_addr) );
        printf("dist=%f angle=%f type=%f\n", distance,
	        angle,
	        tension);
    
    // Close the socket
    close(_socket);
}

/* This is called every time that we get a new image to process the image, get target data,
 * and send the information to the cRIO
 */ 
void processImageCallback(int, void* ) 
{
    static vector<Mat> planes;
    static Mat src_color;
    
    split(src, planes);
  
    // Keep the color that we are intested in and substract off the other planes
    addWeighted(planes[GREEN_PLANE], 1, planes[BLUE_PLANE], -.2, 0, src_color);
    addWeighted(src_color, 1, planes[RED_PLANE], -.2, 0, src_color);
  
    // Dilation + Erosion = Close
    int dilation_type;
  
    if( dilation_elem == 0 ) 
    { 
        dilation_type = MORPH_RECT; 
    }
    else if( dilation_elem == 1 ) 
    { 
        dilation_type = MORPH_CROSS; 
    }
    else if( dilation_elem == 2) 
    { 
        dilation_type = MORPH_ELLIPSE; 
    }
  
    Mat element = getStructuringElement(dilation_type,
				    Size( 2*dilation_size + 1, 2*dilation_size+1 ),
				    Point( dilation_size, dilation_size ) );
  
    static Mat src_gray;
    src_gray = src_color.clone();
 
    // This now does a close
    static Mat src_blur;
    src_blur = src_gray.clone();
    GaussianBlur( src_gray, src_blur, Size( 5, 5 ), 0, 0 );
  
    static Mat threshold_output;
    threshold_output = src_blur.clone();
    
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
  
    // Detect edges using Threshold
    threshold( src_blur, threshold_output, thresh, 255, THRESH_BINARY );
  
    // This now does a close
    static Mat src_dilate;
    dilate(threshold_output, src_dilate, element );
    erode(src_dilate, src_dilate, element);

    static Mat temp;
    temp = src_dilate.clone();
    /// Find contours
    findContours( temp, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE, Point(0, 0) );
  
    // Find the convex hull object for each contour
    
    vector<vector<Point> > hull( contours.size() );
    
    for( int i = 0; i < contours.size(); i++ ) 
    {
        convexHull( Mat(contours[i]), hull[i], false ); 
    }
  
    static Mat drawingContours;
    
    // Draw contours + hull results
    if (options.guiAll) 
    {
        drawingContours = Mat::zeros( threshold_output.size(), CV_8UC3 );
        
        for( int i = 0; i< contours.size(); i++ ) 
        {
            Scalar color = Scalar( 255, 255, 255 );
            drawContours( drawingContours, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        }
    } 
  
    /* Approximate the convex hulls with polygons
     * This reduces the number of edges and makes the contours
     * into quads
     */ 
    vector<vector<Point> > poly( contours.size() );
    
    for (int i=0; i < contours.size(); i++) 
    {
        approxPolyDP(hull[i], poly[i], poly_epsilon, true);
    }
  
    // Prune the polygons into only the ones that we are intestered in.
    vector<vector<Point> > prunedPoly(0);
    vector<vector<Point> > prunedHulls(0);
    vector<vector<Point> > prunedContours(0);
    for (int i=0; i < poly.size(); i++) 
    {
        // Only 4 sized figures
        if (poly[i].size() == 4) 
        {
            Rect bRect = boundingRect(poly[i]);
            // Remove polygons that are too small
            if (bRect.width * bRect.height > minsize) 
            {
	            prunedPoly.push_back(poly[i]);
	            prunedHulls.push_back(hull[i]);
	            prunedContours.push_back(contours[i]);
            }
        }
    }
  
    // Prune to targets (Rectangles that contain an inner rectangle
    vector<vector<Point> > targetQuads(0);
    vector<vector<Point> > targetHulls(0);
    vector<vector<Point> > targetContours(0);
    
    for (int i=0; i < prunedPoly.size(); i++) 
    {
        // Keep only polygons that contain other polygons
        if (rectContainsRect(i, prunedPoly)) 
        {
            targetQuads.push_back(prunedPoly[i]);
            targetHulls.push_back(prunedHulls[i]);
            targetContours.push_back(prunedContours[i]);
        }
    }

    //Refine corner locations
    //  Size winSize(7,7);
    //  Size zeroZone(-1,-1);
    vector<vector<Point2f> > targetQuads2f(targetQuads.size());
    vector<vector<Point> > targetQuads2fi(targetQuads.size());
    vector<vector<Point> > rcontours(targetContours.size());
    
    for (int i=0; i < targetContours.size(); i++) 
    {
        for (int j=0; j < targetContours[i].size(); j++) 
        {
            rcontours[i].push_back(targetContours[i][j]);
        }
        
        reverse(rcontours[i].begin(), rcontours[i].end());
    }

    refineCorners(targetQuads, rcontours, targetQuads2f, targetQuads2fi);

    vector<TargetData> targets;
    getTargetData(src, targetQuads2f, targets);
    TargetGroup targetGroup;
    getTargetGroup(src, targets, targetGroup);

    if (options.guiAll) 
    {
        // Draw the contours in a window
        Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
        
        for( int i = 0; i< contours.size(); i++ ) 
        {
            Scalar color = Scalar( 255, 255, 255 );
            drawContours( drawing, poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        }
    
        // Draw the pruned Poloygons in a window
        Mat prunedDrawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
        
        for (int i=0; i < prunedPoly.size(); i++) 
        {
            Scalar color = Scalar( 255, 255, 255 );
            drawContours(prunedDrawing, prunedPoly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        }
    
        // Draw the targets
        Mat targetsDrawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
        
        for (int i=0; i < targetQuads.size(); i++) 
        {
            Scalar color = Scalar( 64, 64, 64 );
            drawContours(targetsDrawing, targetQuads, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        }
    
        // Draw the targets
        Mat targetsDrawing2fi = Mat::zeros( threshold_output.size(), CV_8UC3 );
    
        for (int i=0; i < targetQuads2fi.size(); i++) 
        {
            Scalar color = Scalar( 255, 255, 255 );
            drawContours(targetsDrawing, targetQuads2fi, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
        }
    
        imshow("Source", src);
        calcHistogram(src);
        imshow("Color", src_color);
        imshow("Blur", src_blur);
        imshow("Dilate", src_dilate);
        imshow("Threshold", threshold_output);
        imshow("Contours", drawingContours);
        imshow("Polygon", drawing );
        imshow("PrunedPolygon", prunedDrawing);
        imshow("Targets", targetsDrawing);
  }

    // Output the final image
    static  Mat finalDrawing;
    finalDrawing = src.clone();
    
    for (int i=0; i < targetQuads.size(); i++) 
    {
        Scalar color = Scalar( 64, 0, 0 );
        drawContours(finalDrawing, targetQuads, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
    }
    
    for (int i=0; i < targetQuads2fi.size(); i++) 
    {
        Scalar color = Scalar( 255, 255, 255 );
        drawContours(finalDrawing, targetQuads2fi, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
    }
  
    for (int i = 0; i < targets.size(); i++ ) 
    {
        Point center( targets[i].centerX, targets[i].centerY );
        Scalar color = Scalar( 255, 255, 255 );
        circle( finalDrawing, center, 10, color );
#ifdef DEBUG_TEXT
        Point textAlign( targets[i].centerX - 50, targets[i].centerY + 35 );
        Point sizeAlign( targets[i].centerX - 50, targets[i].centerY + 50 );
        Point distanceAlign( targets[i].centerX - 50, targets[i].centerY + 65 );
        Point angleXAlign( targets[i].centerX - 50, targets[i].centerY + 80 );
        Point typeTargetAlign( targets[i].centerX - 50, targets[i].centerY + 95 );
        Point tensionAlign( targets[i].centerX - 50, targets[i].centerY + 110 );
        
        ostringstream text, size, distance, angle, typeTarget, tension;
        
        text << "Center: X: " << targets[i].centerX << " Y: " << targets[i].centerY;
        distance << "Distance: X: " << targets[i].distanceX << " Y: " << targets[i].distanceY;
        size << "Size: X: " << targets[i].sizeX << " Y: " << targets[i].sizeY;
        angle << "Angle: X: " << targets[i].angleX;
    
        typeTarget << getTargetTypeString(targets[i].targetType);
        
        tension << "Tension: " << targets[i].tension;
    
        putText( finalDrawing, text.str(), textAlign, CV_FONT_HERSHEY_PLAIN, .7, color );
        putText( finalDrawing, size.str(), sizeAlign, CV_FONT_HERSHEY_PLAIN, .7, color );
        putText( finalDrawing, distance.str(), distanceAlign, CV_FONT_HERSHEY_PLAIN, .7, color );
        putText( finalDrawing, angle.str(), angleXAlign, CV_FONT_HERSHEY_PLAIN, .7, color );
        putText( finalDrawing, typeTarget.str(), typeTargetAlign, CV_FONT_HERSHEY_PLAIN, .7, color );
        putText( finalDrawing, tension.str(), tensionAlign, CV_FONT_HERSHEY_PLAIN, .7, color );
#endif  
    }
    
    // If we have a target then send it to the cRio
    if (targetGroup.selected.valid) 
    {
        Point center( targetGroup.selected.centerX, targetGroup.selected.centerY );
        Scalar color = Scalar( 255, 0, 255 );
        circle( finalDrawing, center, 20, color );
        
	/*        printf("dist=%f angle=%f type=%s\n", targetGroup.selected.distanceY,
            targetGroup.selected.angleX,
	        getTargetTypeString(targetGroup.selected.targetType));
	*/
//#ifdef CRIO_NETWORK
        TargetData target;
        
	//        printf("dist=%f angle=%f type=%s\n", targetGroup.selected.distanceY,
	//	        targetGroup.selected.angleX,
	//	        getTargetTypeString(targetGroup.selected.targetType));

        float tension = convertDistanceToTension(targetGroup.selected.distanceY);
    
        sendMessage(targetGroup.selected.distanceY, targetGroup.selected.angleX, tension);
#//endif
    }
  
    imshow( "Final", finalDrawing );
}

string getOutputVideoFileName() 
{
    char outputFileName[100];
    time_t beginningTime;      // beginning time and end times
    
    tm *timeTm;
    time(&beginningTime);      // start the clock
    
    timeTm = localtime(&beginningTime);
    
    strftime(outputFileName, sizeof(outputFileName)-1, "RobotVideo_%Y_%m_%d_%H_%M_%S.mjpg", timeTm);
    
    return outputFileName;
}

void writeImage(Mat &src) 
{
    char outputFileName[100];
    time_t currentTime;
    
    tm *timeTm;
    time(&currentTime);
    timeTm = localtime(&currentTime);
    
    strftime(outputFileName, sizeof(outputFileName)-1, "RobotImage_%Y_%m_%d_%H_%M_%S.jpg", timeTm);
    imwrite(outputFileName, src);
}

void computeFramesPerSec() 
{
    static bool first = true;
    double fps, avgFps;             // fps calculated using number of frames / seconds
    static int counter = 0;         // frame counter
    double elapsed;                 // floating point seconds elapsed since start
    static timespec beginningTs, currentTs, lastTs;

    if (first) 
    {
        clock_gettime(CLOCK_REALTIME, &beginningTs);
        clock_gettime(CLOCK_REALTIME, &lastTs);
        clock_gettime(CLOCK_REALTIME, &currentTs);
    
        first = false;
    }

    // see how much time has elapsed
    clock_gettime(CLOCK_REALTIME, &currentTs);
    
    // calculate current FPS
    ++counter;       
    
    elapsed = (currentTs.tv_nsec - beginningTs.tv_nsec) * 1e-9 + (currentTs.tv_sec - beginningTs.tv_sec); // 1e-9 is the epsilon or "close enough" factor     
    avgFps = counter / elapsed;
    elapsed = (currentTs.tv_nsec - lastTs.tv_nsec) * 1e-9 + (currentTs.tv_sec - lastTs.tv_sec);           // 1e-9 is the epsilon or "close enough" factor    
    fps = 1 / elapsed;
  // will print out Inf until sec is greater than 0
  
    printf("Avg FPS = %.2f. FPS = %.2f\n", avgFps, fps);
    
    lastTs = currentTs;
}

int main( int argc, char** argv ) 
{
    VideoCapture *cap = 0;
    VideoWriter *record = 0;
  
    // Process any OpenCV arguments
    cvInitSystem(argc, argv);
    options.processArgs(argc, argv);

    if (options.processCamera) 
    {
        if (options.processVideoFile) 
        {
            cap = new VideoCapture(options.fileName);
        }  
        else 
        {
            cap = new VideoCapture("http://10.17.68.9/axis-cgi/mjpg/video.cgi?resolution=320x240&req_fps=30&.mjpg"); 
        }
    
        if(!cap || !cap->isOpened() ) // check if we succeeded
        {       
            printf("ERROR: unable to open camera\n");
            return -1;
        }
    
        *cap >> src;
        
        string outputFileName=getOutputVideoFileName();
        record = new VideoWriter(outputFileName.c_str(), CV_FOURCC('M', 'J', 'P', 'G'), 30, src.size(), true);
        
        if (!record->isOpened()) 
        {
            printf("VideoWriter failed to open!\n");
            return -1;
        }
    } 
    else 
    {
        // Load an image from a file
        src = imread(options.fileName, 1 );
    }

    createGuiWindows();
  
    //  dilation_callback(0,0);
    while (1) 
    {
        timespec time1, time2, time3, time4, time5;

        clock_gettime(CLOCK_REALTIME, &time1);
        
        if (options.processCamera) 
        {
            if (!pause_image) 
            {
	            /* If there are more images to process then process them else
	             * return.  This will be the case when processing an image file.
                 */ 
	            if ( !cap->grab() ) return 0;
	
                if ( !cap->retrieve(src) ) return 0;
            }
        }
    
        clock_gettime(CLOCK_REALTIME, &time2);

        processImageCallback( 0, 0 );
        clock_gettime(CLOCK_REALTIME, &time3);

        char c = 0;

        if (options.processCamera) 
        {
            static int counter = 0;
            counter++;
            if ( (counter % 4) == 0 ) c = waitKey(1);
        } 
        else 
        {
            // Non realtime images
            c = waitKey(1);
        }
        
        clock_gettime(CLOCK_REALTIME, &time4);

        if (c == 'q') return 0;
    
        if (c == 'p') pause_image = !pause_image;
    
        if (c == 'w') writeImage(src);
    
        clock_gettime(CLOCK_REALTIME, &time5);
    
        computeFramesPerSec();
  }
  
    return 0;

}
