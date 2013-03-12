#ifndef __RECT_TARGET_MEASURED_H__
#define __RECT_TARGET_MEASURED_H__

/**
   @brief  This class contains methods of formula transformations that were
           taken from measured values.
*/
class RectTargetMeasured {
 public:
  static float computeLowYOffset(float distance);
  static float computeMidYOffset(float distance);
  static float computeHighYOffset(float distance);
  static float convertDistanceToTension(float distance);
  static bool approximateHeight(float value, float baseline);
  static float pixelsToAngleX(int pixels);
  static float rectSizeXToDistance(float sizeX);
  static float rectSizeYToDistance(float sizeY);
};

#endif
