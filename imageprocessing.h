#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <iostream>
#include <functional>
#include <numeric>

const int DM = 256;
const int MIN_GRAY_LEVEL = 0;
const int MAX_GRAY_LEVEL = 255;

using His = std::array<int,DM>;
using Image = cv::Mat;
using Weight = std::vector<float>;
using Operator = std::function<uchar(const Weight &)>;
using Neighbor = std::function<Image(const Image &, const Operator &, const Weight &)>;

His calcHistogram(const Image &image);
His calcSumOfHist(const His &histogram);

Image histogramTransforms(const Image &mat);
Image negativeTransforms(const Image &mat);
Image thresholdingTransforms(const Image &mat, int threshold);
Image logaricTransforms(const Image &mat, float c);
Image powerlawTransforms(const Image &mat, float c, float y);
Image piecewiseLinearTransforms(const Image &mat, int r1, int s1, int r2, int s2);
Image grayLevelSlicingTransforms(const Image &mat, int a, int b, int val);
Image bitPlaneSlicingTransforms(const Image &mat, int th, const std::function<uchar(uchar, int)> &func);

Image neighBorTransformsOmit(const Image &mat, const Operator &op, const Weight &weight);
Image neighBorTransformsPad(const Image &mat, const Operator &op, const Weight &weight);
Image neighBorTransformsRep(const Image &mat, const Operator &op, const Weight &weight);
Image neighBorTransformsTrunc(const Image &mat, const Operator &op, const Weight &weight);
Image neighBorTransformsAllow(const Image &mat, const Operator &op, const Weight &weight);

Image morphology(const Image &mat, const Operator &op, const Weight &structEle);
Image open(const Image &mat, const Weight &structEle);
Image close(const Image &mat, const Weight &structEle);
Image bound(const Image &mat, const Weight &structEle);
Image fill(const Image &mat, const Weight &structEle, int x, int y);
Image floodFill(const Image &mat, int x, int y);
uchar doErosion(const Weight &vec);
uchar doDilation(const Weight &vec);

uchar viewBit(uchar val, int th);
uchar onBit(uchar val, int th);
uchar offBit(uchar val, int th);

uchar min(const Weight &vec);
uchar max(const Weight &vec);
uchar med(const Weight &vec);
uchar sum(const Weight &vec);
float getScore(const Image &image);

float avgThreshold(const Image &image);
std::pair<float,float> avgThreshold(const Image &image, uchar t);
uchar getT(const Image &image, float preT);
#endif // IMAGEPROCESSING_H
