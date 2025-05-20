#include "Ocr.h"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/ml/ml.hpp>
#include <fstream>
#include <random>

#include <QDebug>
namespace
{
    const Size gTemplateSymbolSize = { 16,16 }; // mean: 16, 21  20,30
}
namespace
{
    inline vector<int> GlyphsCount(size_t number) {
        vector<int> out;
        string data = to_string(number);
        for (auto c : data) {
            if (c >= '0' && c <= '9')
            out.push_back(c - '0');
            return out;
        }
    }
}

Ocr::Ocr(const path& train)
    :mTrainDir(train)
{}
void Ocr::Initialize() {
    Train();
    mIsLoaded = true;
}
void Ocr::Train() {
    array<vector<Mat>, 10> glyphs;

    for (auto& it : directory_iterator(mTrainDir)) {
        if (it.path().has_extension() && it.path().extension() == ".png") {
            const string file_name = it.path().filename().replace_extension().string();
            if (!file_name.empty() && file_name[0] >= '0' && file_name[0] <= '9') {
                glyphs[file_name[0] - '0'].emplace_back(imread((it.path()).generic_string(), IMREAD_GRAYSCALE));
            }
            else {
                // TODO: warning
            }
        }
    }

    // if (false) {
    //     Size mean_size;
    //     int glyphs_count = 0;
    //     for (const auto& v : glyphs)
    //         for (const auto& m : v)
    //         {
    //             mean_size += m.size();
    //             ++glyphs_count;
    //         }
    //     if (glyphs_count)
    //         mean_size = mean_size /= glyphs_count;
    // }

    Mat array_chars;
    Mat array_images;
    for (size_t w = 0; w < glyphs.size(); ++w) {
        for (const auto& img : glyphs[w]) {
            Mat template_img;
            resize(img, template_img, gTemplateSymbolSize);
            Mat template_img_float;
            template_img.convertTo(template_img_float, CV_32FC1);
            Mat template_img_reshape = template_img_float.reshape(1, 1);

            array_images.push_back(template_img_reshape);
            array_chars.push_back('0' + static_cast<int>(w));
        }
    }

    scoped_lock lock(mKnearesMutex);
    if (!array_images.empty()) {
        mKNearest = ml::KNearest::create();
        mKNearest->setDefaultK(1);
        if (!mKNearest->train(array_images, ml::ROW_SAMPLE, array_chars)) {
            // TODO: warning
        }
    }
}
int Ocr::RecognizeDigit(const Mat& img) const {
    if (img.empty()) return false;

    Mat template_img;
    resize(img, template_img, gTemplateSymbolSize);
    Mat template_img_float;
    template_img.convertTo(template_img_float, CV_32FC1);
    Mat template_img_reshape = template_img_float.reshape(1, 1);

    float res = -1;
    {
        scoped_lock lock(mKnearesMutex);
        if (!mKNearest){
            return -2;
        }
        try {
            Mat finded_symbol_img(0, 0, CV_32F);
            res = mKNearest->findNearest(template_img_reshape, 1, finded_symbol_img);
        }
        catch (const exception&) {
            return -3;
        }
    }

    auto symbol = static_cast<char>(res);
    if (symbol >= '0' && symbol <= '9') return static_cast<int>(symbol - '0');

    return -1;
}
void Ocr::Recognize(const Mat& img,int &num) const {
    if (img.empty()) num = -1;

    auto preprocessed = Preprocess(img);
    auto glyphs = FindGlyphs(preprocessed);

    if (glyphs.empty()) num = -1;

    int number = 0;
    for (size_t i = 0; i < glyphs.size(); ++i) {
        auto digit = RecognizeDigit(Mat(preprocessed, glyphs[glyphs.size() - i - 1]));

        if (digit < 0) {
            num = -1;
        } else if (digit > 9){
            num = -1;
        }

        int power = static_cast<int>(std::pow(10, i));
        number += digit * power;
    }
    num = number;
}

bool Ocr::IsLoaded() const {
    return mIsLoaded;
}

Mat Ocr::DrawRects(const Mat& img, const vector<Rect>& rects) const {
    Mat out;
    img.copyTo(out);
    for (const auto& rect : rects) rectangle(out, rect, Scalar(255, 0, 0), FILLED);

    return out;
}
Mat Ocr::Preprocess(const Mat& img) const {
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);

    Mat out;
    threshold(gray, out, 230, 255, THRESH_BINARY);

    return out;
}

vector<Rect> Ocr::FindGlyphs(const Mat& img) const {
    vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours(img, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

    vector<Rect> out;
    for (const auto& contour : contours) {
        auto brect = boundingRect(contour);

        if (brect.size().width < mParams.glyph_min_width) continue;

        if (brect.size().height < mParams.glyph_min_height) continue;

        if (brect.area() < mParams.glyph_min_area) continue;

        if (brect.size().width > mParams.glyph_max_width) continue;

        if (brect.size().height > mParams.glyph_max_height) continue;

        if (brect.area() > mParams.glyph_max_area) continue;

        out.push_back(brect);
    }
    Mat temp = DrawRects(img,out);
    //imshow("001",temp);

    sort(out.begin(), out.end(), [](const Rect& a, const Rect& b) {
             return (a.br() + a.tl()).x / 2 < (b.br() + b.tl()).x / 2;
         });
    return out;
}
