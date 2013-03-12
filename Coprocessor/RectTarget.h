#ifndef __RECTTARGET_H__
#define __RECTTARGET_H__

struct TargetGroup;

/**
   @brief Struct containing information about the vision targets
*/
class RectTarget {
 public:
  //! Possible target types including (virtual/combined) targets
  typedef enum {
    TARGET_HEIGHT_UNKNOWN = 0,
    TARGET_HEIGHT_HIGH,
    TARGET_HEIGHT_MIDDLE,
    TARGET_HEIGHT_MIDDLE_RIGHT,
    TARGET_HEIGHT_MIDDLE_LEFT,
    TARGET_HEIGHT_LOW,
    TARGET_HEIGHT_MIDDLE_COMBINED
  } TargetType;

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
  TargetType targetType;  //!< The type of target detected
  float valid;            //!< Whether or not the target is valid

  RectTarget();
  const char *getTargetTypeString();
  void computeTargetType();
  void printTarget();
  static void computeTargetGroups(cv::Mat &image, std::vector<RectTarget> &targets, TargetGroup &targetGroup);
  static bool getBestTarget(std::vector<RectTarget> &targets, RectTarget &target);
  static void getTargetsType(std::vector<RectTarget> &targets, std::vector<int> &targetIndices,
			     RectTarget::TargetType targetType);
  static void getRectTarget(cv::Mat &image, const std::vector<std::vector<cv::Point2f> > &targetQuads, std::vector<RectTarget> &targets);
  static void printTargets(std::vector<RectTarget> &targets);
};

/**
   @brief The types of all targets seen in an image.  If targets aren't valid
          then their valid bit is not set.
*/
struct TargetGroup {
  RectTarget high;          //!< The high target
  RectTarget middleLeft;    //!< The middle right target
  RectTarget middleRight;   //!< The middle left target
  RectTarget low;           //!< The low target
  RectTarget selected;      //!< The best target
                            //!< (Can be a combination of multiple targets)
};

#endif
