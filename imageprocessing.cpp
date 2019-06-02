#include "imageprocessing.h"
#include <QDebug>

static const int DROW3[] = {-1, -1, -1, +0, +0, +0, +1, +1, +1};
static const int DCOL3[] = {-1, +0, +1, -1, +0, +1, -1, +0, +1};
static const int DCNT3 = 9;

His calcHistogram(Image const &image)
{
    His histogram;
    for(int &h : histogram) h = 0;

    for(int row = 0; row < image.rows; ++row){
        for(int col = 0; col < image.cols; ++col){
            ++histogram[image.at<uchar>(row, col)];
        }
    }
    return histogram;
}

His calcSumOfHist(His const &histogram)
{
    His result = histogram;
    for(size_t i = 1; i < DM; ++i){
        result[i] += result[i - 1];
    }
    return result;
}

Image histogramTransforms(const Image &mat)
{
    const double M = 256.0 / (mat.rows * mat.cols);
    His sum = calcSumOfHist(calcHistogram(mat));
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            int val = sum[mat.at<uchar>(i, j)] * M;
            val = std::min(MAX_GRAY_LEVEL, val);
            result.at<uchar>(i, j) = cv::saturate_cast<uchar>(val);
        }
    }
    return result;
}

Image negativeTransforms(const Image &mat)
{
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            result.at<uchar>(i, j) =
                    cv::saturate_cast<uchar>(MAX_GRAY_LEVEL - mat.at<uchar>(i, j));
        }
    }
    return result;
}

Image thresholdingTransforms(const Image &mat, int threshold)
{
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            result.at<uchar>(i, j) =
                    (mat.at<uchar>(i, j) < threshold ? MIN_GRAY_LEVEL : MAX_GRAY_LEVEL);
        }
    }
    return result;
}
Image logaricTransforms(const Image &mat, float c)
{
    c *= MAX_GRAY_LEVEL;
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            result.at<uchar>(i, j) =
                    cv::saturate_cast<uchar>(c * std::log(mat.at<uchar>(i, j) / 255.0 + 1));
        }
    }
    return result;
}

Image powerlawTransforms(const Image &mat, float c, float y)
{
    c *= MAX_GRAY_LEVEL;
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            result.at<uchar>(i, j) =
                    cv::saturate_cast<uchar>(c * std::pow(mat.at<uchar>(i, j) / 255.0, y));
        }
    }
    return result;
}
int piecewise(int x, int r1, int s1, int r2, int s2)
{
    float result;
    if(0 <= x && x <= r1){
        result = s1/(r1 == 0 ? 0.001 : r1) * x;
    }else if(r1 < x && x <= r2){
        result = ((s2 - s1)/(r2 - r1 == 0 ? 0.001 : r2 - r1)) * (x - r1) + s1;
    }else if(r2 < x && x <= 255){
        result = ((255 - s2)/(255 - r2 == 0 ? 0.001 : 255 - r2)) * (x - r2) + s2;
    }
    return (int)result;
}

Image piecewiseLinearTransforms(const Image &mat, int r1, int s1, int r2, int s2)
{
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            result.at<uchar>(i, j) =
                    cv::saturate_cast<uchar>(piecewise(mat.at<uchar>(i, j), r1, s1, r2, s2));
        }
    }
    return result;
}
Image grayLevelSlicingTransforms(const Image &mat, int a, int b, int val)
{
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            uchar cur = mat.at<uchar>(i, j);
            if (cur >= a && cur <= b){
                result.at<uchar>(i, j) = cv::saturate_cast<uchar>(val);
            }
        }
    }
    return result;
}
uchar viewBit(uchar val, int th)
{
    return ((val >> th) & 1) ? MAX_GRAY_LEVEL : MIN_GRAY_LEVEL;
}
uchar onBit(uchar val, int th)
{
    return val |= (1 << th);
}
uchar offBit(uchar val, int th)
{
    return val &= ~(1 << th);
}

Image bitPlaneSlicingTransforms(const Image &mat, int th, std::function<uchar(uchar, int)> const &func)
{
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
            result.at<uchar>(i, j) = func(mat.at<uchar>(i, j), th);
        }
    }
    return result;
}

uchar min(const Weight &vec)
{
    return cv::saturate_cast<uchar>(*std::min_element(vec.begin(), vec.end()));
}
uchar max(const Weight &vec)
{
    return cv::saturate_cast<uchar>(*std::max_element(vec.begin(), vec.end()));
}
uchar med(const Weight &vec)
{
    auto tmp = vec;
    std::nth_element(tmp.begin(), tmp.begin() + (tmp.size() >> 1), tmp.end());
    return cv::saturate_cast<uchar>(tmp[(tmp.size() >> 1)]);
}
uchar sum(const Weight &vec)
{
   float res = 0;
   for(auto x : vec) res += x;
   return cv::saturate_cast<uchar>(res);
}

bool safe(const Image &mat, int r, int c)
{
    return 0 <= r && r < mat.rows
        && 0 <= c && c < mat.cols;
}

Weight getMatrix(const Image &mat, int row, int col, const Weight &weight)
{
    Weight neighbour;
    neighbour.reserve(DCNT3);
    for(int dir = 0; dir < DCNT3; ++dir){
        int newRow = row + DROW3[dir];
        int newCol = col + DCOL3[dir];
        if (!safe(mat, newRow, newCol)) continue;
        neighbour.push_back((float)mat.at<uchar>(newRow, newCol) * weight[dir]);
    }
    return neighbour;
}

Weight getMatrixWithReplication(const Image &mat, int row, int col, const Weight &weight)
{
    auto reverseDir = [&](int r, int c) -> std::pair<int,int>{
        if (r < 0 && c < 0) return {0, 0};
        if (r < 0) return {0, c};
        if (c < 0) return {r, 0};
        if (r >= mat.rows && c >= mat.cols) return {r - 1, c - 1};
        if (r >= mat.rows) return {mat.rows - 1, c};
        if (c >= mat.cols) return {r, mat.cols - 1};
    };
    Weight neighbour;
    neighbour.reserve(DCNT3);
    for(int dir = 0; dir < DCNT3; ++dir){
        int newRow = row + DROW3[dir];
        int newCol = col + DCOL3[dir];
        if (!safe(mat, newRow, newCol))
            std::tie(newRow, newCol) = reverseDir(newRow, newCol);
        neighbour.push_back((float)mat.at<uchar>(newRow, newCol) * weight[dir]);
    }
    return neighbour;
}

Weight getMatrixMorphology(const Image &mat, int row, int col, const Weight &structEle){
    Weight neighbour;
    neighbour.reserve(DCNT3);
    for(int dir = 0; dir < DCNT3; ++dir){
        int newRow = row + DROW3[dir];
        int newCol = col + DCOL3[dir];
        if (!safe(mat, newRow, newCol) || structEle[dir] < 1) continue;
        if ((float)mat.at<uchar>(newRow, newCol) * structEle[dir] > 0)
            neighbour.push_back(1);
        else neighbour.push_back(0);
    }
    return neighbour;
}

Image neighBorTransformsOmit(const Image &mat, const Operator &op, const Weight &weight)
{
    Image result = mat.clone();
    for(int i = 1; i < mat.rows - 1; ++i){
        for(int j = 1; j < mat.cols - 1; ++j){
           result.at<uchar>(i, j) = op(getMatrix(mat, i, j, weight));
        }
    }
    return result;
}

Image neighBorTransformsPad(const Image &mat, const Operator &op, const Weight &weight)
{
    Image result(mat.rows + 2, mat.cols + 2, CV_8UC1, cv::Scalar(MAX_GRAY_LEVEL));
    for(int i = 1; i < result.rows - 1; ++i){
        for(int j = 1; j < result.cols - 1; ++j){
           result.at<uchar>(i, j) = op(getMatrix(mat, i - 1, j - 1, weight));
        }
    }
    return result;
}

Image neighBorTransformsRep(const Image &mat, const Operator &op, const Weight &weight)
{
    Image result(mat.rows, mat.cols, CV_8UC1);
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
           result.at<uchar>(i, j) = op(getMatrixWithReplication(mat, i, j, weight));
        }
    }
    return result;
}

Image neighBorTransformsTrunc(const Image &mat, const Operator &op, const Weight &weight)
{
    if (mat.rows <= 2 || mat.cols <= 2) return mat;
    Image result(mat.rows - 2, mat.cols - 2, CV_8UC1);
    for(int i = 1; i < mat.rows - 1; ++i){
        for(int j = 1; j < mat.cols - 1; ++j){
           result.at<uchar>(i - 1, j - 1) = op(getMatrix(mat, i, j, weight));
        }
    }
    return result;
}

Image neighBorTransformsAllow(const Image &mat, const Operator &op, const Weight &weight)
{
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
           result.at<uchar>(i, j) = op(getMatrix(mat, i, j, weight));
        }
    }
    return result;
}

float getScore(Image const &image)
{        
    His his = calcHistogram(image);
    int area = (image.rows * image.cols);
    int expected = area / DM;
    float diff = 0.1f;
    for(int grayLevelCnt : his){
        if (grayLevelCnt == 0)
            diff += expected;
        else if (grayLevelCnt < expected)
            diff += std::abs(expected - grayLevelCnt);
    }
    return area / diff;
}

uchar getT(const Image &image, float preT)
{
    float firstT = avgThreshold(image);
    while (1){
        float u1, u2;
        std::tie(u1, u2) = avgThreshold(image, firstT);
        float secondT = (u1 + u2) / 2.f;
        if (std::abs(secondT - firstT) < preT) break;
        firstT = secondT;
    }
    return cv::saturate_cast<uchar>(firstT);
}

std::pair<float,float> avgThreshold(const Image &image, uchar t)
{
    His his = calcHistogram(image);
    float val1 = 0, val2 = 0;
    int sum1 = 0, sum2 = 0;
    for(int i = 0; i < t; ++i){
        val1 += i * his[i];
        sum1 += his[i];
    }
    for(int i = t; i < DM; ++i){
        val2 += i * his[i];
        sum2 += his[i];
    }
    return {val1 / sum1, val2 / sum2};
}

float avgThreshold(const Image &image)
{
    His his = calcHistogram(image);
    float val = 0;
    for(int i = 0; i < DM; ++i){
        val += i * his[i];        
    }
    return val / (image.rows * image.cols);
}

Image morphology(const Image &mat, const Operator &op, const Weight &structEle)
{
    Image result = mat.clone();
    for(int i = 0; i < mat.rows; ++i){
        for(int j = 0; j < mat.cols; ++j){
           result.at<uchar>(i, j) = op(getMatrixMorphology(mat, i, j, structEle));
        }
    }
    return result;
}

uchar doErosion(const Weight &vec)
{
    for(float num : vec){
        if (num < 1)
            return MIN_GRAY_LEVEL;
    }
    return MAX_GRAY_LEVEL;
}

uchar doDilation(const Weight &vec)
{
    for(float num : vec){
        if (num >= 1)
            return MAX_GRAY_LEVEL;
    }
    return MIN_GRAY_LEVEL;
}

Image open(const Image &mat, const Weight &structEle)
{
    return morphology(morphology(mat, doErosion, structEle), doDilation, structEle);
}

Image close(const Image &mat, const Weight &structEle)
{
    return morphology(morphology(mat, doDilation, structEle), doErosion, structEle);
}

Image bound(const Image &mat, const Weight &structEle)
{
    return mat - morphology(mat, doErosion, structEle);
}

Image fill(const Image &mat, const Weight &structEle, int x, int y)
{
    // TODO: make it faster
    Image Ac;
    cv::bitwise_not(mat, Ac);
    Image x0, x1(mat.rows, mat.cols, CV_8UC1, cv::Scalar(MIN_GRAY_LEVEL));
    x1.at<uchar>(y, x) = MAX_GRAY_LEVEL;

    int cnt = 0;
    while (1){
        cv::bitwise_and(morphology(x1, doDilation, structEle), Ac, x0);
        if (std::equal(x1.begin<uchar>(), x1.end<uchar>(), x0.begin<uchar>())) break;
        x1 = x0.clone();
        ++cnt;
    }    
    cv::bitwise_or(x1, mat, x0);
    return x0;
}

Image floodFill(const Image &mat, int x, int y)
{
    Image result = mat.clone();
    std::vector<std::pair<int,int>> st;
    st.emplace_back(y, x);

    const uchar fillColor = MAX_GRAY_LEVEL;

    auto isOk = [&](int row, int col){
        return row > 0 && row < result.rows
            && col > 0 && col < result.cols
            && result.at<uchar>(row, col) != fillColor;
    };

    while (!st.empty()){
        auto cur = st.back();
        st.pop_back();

        std::pair<int,int> next;
        for(int dir = 0; dir < 9; ++dir){
            next = {cur.first + DROW3[dir], cur.second + DCOL3[dir]};
            if (isOk(next.first, next.second)){
                result.at<uchar>(next.first, next.second) = fillColor;
                st.emplace_back(next);
            }
        }
    }
    return result;
}
