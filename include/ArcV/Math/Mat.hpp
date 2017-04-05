#pragma once

#ifndef ARCV_MAT_HPP
#define ARCV_MAT_HPP

#include <vector>

#include "Vec.hpp"

namespace Arcv {

enum Colorspace { ARCV_COLORSPACE_GRAY = 0,
                  ARCV_COLORSPACE_GRAY_ALPHA,
                  ARCV_COLORSPACE_RGB,
                  ARCV_COLORSPACE_RGBA,
                  ARCV_COLORSPACE_HSV };

class Mat {
public:
  Mat(const unsigned int width,
      const unsigned int height,
      const unsigned int channels,
      const unsigned short bitDepth,
      const Colorspace colorspace)
    : width{ width }, height{ height }, imgBitDepth{ bitDepth }, colorspace{ colorspace }, data(width * height * channels) {}

  const unsigned int getWidth() const { return width; }
  const unsigned int getHeight() const { return height; }
  const unsigned short getImgBitDepth() const { return imgBitDepth; }
  const Colorspace getColorspace() const { return colorspace; }
  const std::vector<unsigned char>& getData() const { return data; }
  std::vector<unsigned char>& getData() { return data; }

  void changeColorspace();
  Mat& convolve();

  Mat& operator+(Mat& mat) { return *this += mat; }
  Mat& operator+(const float& val) { return *this += val; }
  Mat& operator-(Mat& mat) { return *this -= mat; }
  Mat& operator-(const float& val) { return *this -= val; }
  Mat& operator*(Mat& mat) { return *this *= mat; }
  Mat& operator*(const float& val) { return *this *= val; }
  Mat& operator/(Mat& mat) { return *this /= mat; }
  Mat& operator/(const float& val) { return *this /= val; }
  Mat& operator+=(Mat& mat);
  Mat& operator+=(const float& val);
  Mat& operator-=(Mat& mat);
  Mat& operator-=(const float& val);
  Mat& operator*=(Mat& mat);
  Mat& operator*=(const float& val);
  Mat& operator/=(Mat& mat);
  Mat& operator/=(const float& val);
  unsigned char& operator[](const unsigned int index) { return data[index]; }

private:
  unsigned int width, height;
  unsigned short imgBitDepth;
  Colorspace colorspace;
  std::vector<unsigned char> data;
};

} // namespace Arcv

#endif // ARCV_MAT_HPP
