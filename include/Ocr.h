#ifndef OCR_H
#define OCR_H

#include "BotLib_global.h"
#include "Error.h"

class BOTLIB_EXPORT Ocr : public QObject {
    Q_OBJECT
public:
    Ocr(const path& train);
    ~Ocr() = default;

    struct Params {
        int gray_threshold = 242;
        int glyph_min_width = 6;//7
        int glyph_min_height = 10;//13
        int glyph_min_area = 59;//90
        int glyph_max_width = 16;//7
        int glyph_max_height = 16;//13
        int glyph_max_area = 256;//90

        //friend auto operator<=> (const Params& a, const Params& b) = default;
    };
    struct TestUnit {
        path path;
        size_t file_size = 0; 
        Mat img;
        int number = 0;
    }; // не надо?

    Ocr(const Ocr& other) = delete;
    Ocr(Ocr&& other) noexcept = delete;
    Ocr& operator=(const Ocr& other) = delete;
    Ocr& operator=(Ocr&& other) noexcept = delete;

    bool IsLoaded() const;

    int RecognizeDigit(const Mat& img) const;

    Mat Preprocess(const Mat& img) const;
    Mat DrawRects(const Mat& img, const vector<Rect>& rects) const;

    vector<Rect> FindGlyphs(const Mat& img) const;

public slots:
    void Recognize(const Mat& img, int& num) const; //к нему сигнал void **(const Mat& img, int& num);

    void Initialize();


private:
    void Train();

    struct TestUnitCmp {
        bool operator()(const TestUnit& a, const TestUnit& b) const {
            auto a_size = a.img.rows * a.img.cols * a.img.elemSize1();
            auto b_size = b.img.rows * b.img.cols * b.img.elemSize1();

            return (a.file_size < b.file_size) ||
                   ((!(b.file_size < a.file_size)) && (string_view(reinterpret_cast<const char*>(a.img.data), a_size) < string_view(reinterpret_cast<const char*>(b.img.data), b_size)));
        }
    }; // ?

    mutable mutex mKnearesMutex;
    Ptr<ml::KNearest> mKNearest;

    const path mTrainDir;

    set<TestUnit, TestUnitCmp> mTestUnits; // не надо?

    array<Mat, 10> mDigitsMat; // тоже?

    atomic<bool> mIsLoaded = false;

    Params mParams;
};

#endif // OCR_H
