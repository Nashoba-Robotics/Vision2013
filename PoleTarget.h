#ifndef __POLE_TARGET_H__
#define __POLE_TARGET_H__

/**
   @brief Struct containing information about the vision targets
*/
class PoleTarget {
 public:

  //! The points that compose the target
  std::vector<cv::Point2f> points;

  float centerX;   //!< Horizontal center of target
  float centerY;   //!< Vertical center of target
  float sizeX;     //!< Horizontal size of target
  float sizeY;     //!< Vertical size of target
  float distanceX; //!< Distance to the horizontal lines of the target
  float distanceY; //!< Distance to the vertical lines of target
  float angleX;    //!< Angle that the target is off center
  float tension;   //!< Computed value to tension tensioner
  float valid;            //!< Whether or not the target is valid
  cv::Point2f topPt;
  cv::Point2f bottomPt;
  float anglePole;
  float anglePerpendicular;

  PoleTarget();
  void computeTargetType();
  void printTarget();
  static void getPoleTargets(cv::Mat &image, const std::vector<std::vector<cv::Point2f> > &targetQuads, std::vector<PoleTarget> &targets);
  static PoleTarget& getBestPoleTarget(std::vector<PoleTarget> &targets, PoleTarget &bestTarget);
  static void printTargets(std::vector<PoleTarget> &targets);
};

#endif
