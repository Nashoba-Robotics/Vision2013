#ifndef __HSV_THRESHOLD_H__
#define __HSV_THRESHOLD_H__

#include <string>
#include <stdio.h>
#include "opencv2/imgproc/imgproc.hpp"

class HsvThreshold {
 public:
  std::string name;
  class Params {
  public:
  Params(bool twohuesIn, int huelowIn, int huehighIn, int huelowIn2, int huehighIn2,
	 int satlowIn, int sathighIn, int valuelowIn, int valuehighIn) :
    twohues(twohuesIn), huelow(huelowIn), huehigh(huehighIn), huelow2(huelowIn2), huehigh2(huehighIn2),
      satlow(satlowIn), sathigh(sathighIn), valuelow(valuelowIn), valuehigh(valuehighIn) {}
    bool twohues;
    int huelow;
    int huehigh;
    int huelow2;
    int huehigh2;
    int satlow;
    int sathigh;
    int valuelow;
    int valuehigh;
  } params;
 HsvThreshold(bool twohuesIn, int huelowIn, int huehighIn, int huelowIn2, int huehighIn2,
	      int satlowIn, int sathighIn, int valuelowIn, int valuehighIn) :
  params(twohuesIn, huelowIn, huehighIn, huelowIn2, huehighIn2,
	 satlowIn, sathighIn, valuelowIn, valuehighIn) {}
  Params getParams() {
    return params;
  }

  cv::Mat temp;

  void thresholdImage(cv::Mat &in, cv::Mat &out) {
    if (params.twohues) {
      cv::inRange(in,  
		  cv::Scalar(params.huelow, params.satlow, params.valuelow),  
		  cv::Scalar(params.huehigh, params.sathigh, params.valuehigh),  
		  out);

      cv::inRange(in,  
		  cv::Scalar(params.huelow2, params.satlow, params.valuelow),  
		  cv::Scalar(params.huehigh2, params.sathigh, params.valuehigh),  
		  temp);
      out |= temp;
    } else {
      cv::inRange(in,  
		  cv::Scalar(params.huelow, params.satlow, params.valuelow),  
		  cv::Scalar(params.huehigh, params.sathigh, params.valuehigh),  
		  out);
    }
  }

  void writeDefines(FILE *file, std::string indent) {
    fprintf(file, "%s namespace HSVThreshold {\n", indent.c_str());
    std::string indentMore = indent;
    indentMore += "  ";
    fprintf(file, "%s static const int HueLow = %d;\n", indentMore.c_str(), params.huelow);
    fprintf(file, "%s static const int HueHigh = %d;\n", indentMore.c_str(), params.huehigh);
    if (params.twohues) {
      fprintf(file, "%s static const int HueLow2 = %d;\n", indentMore.c_str(), params.huelow2);
      fprintf(file, "%s static const int HueHigh2 = %d;\n", indentMore.c_str(), params.huehigh2);
    }
    fprintf(file, "%s static const int SatLow = %d;\n", indentMore.c_str(), params.satlow);
    fprintf(file, "%s static const int SatHigh = %d;\n", indentMore.c_str(), params.sathigh);
    fprintf(file, "%s static const int ValueLow = %d;\n", indentMore.c_str(), params.valuelow);
    fprintf(file, "%s static const int ValueHigh = %d;\n", indentMore.c_str(), params.valuehigh);
    fprintf(file, "%s };\n", indent.c_str());
  }
};

#endif
