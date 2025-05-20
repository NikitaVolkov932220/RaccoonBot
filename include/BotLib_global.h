#ifndef BOTLIB_GLOBAL_H
#define BOTLIB_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QString>
#include <QImage>

#include <iostream>
#include <filesystem>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <utility>
#include <atomic>
#include <thread>
#include <mutex>
#include <set>
#include <optional>

#include <windows.h>
#include <WinUser.h>
#include <regex>
#include <tchar.h>
#include <psapi.h>
#include <comdef.h>
#include <Wbemidl.h>

#include <opencv2/opencv.hpp> // opencv
#include <boost/format.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/core/mat.hpp>

using namespace std;
using namespace cv;
using namespace std::filesystem;
#pragma comment(lib, "wbemuuid.lib")


#if defined(BOTLIB_LIBRARY)
#define BOTLIB_EXPORT Q_DECL_EXPORT
#else
#define BOTLIB_EXPORT Q_DECL_IMPORT
#endif

#endif // BOTLIB_GLOBAL_H
