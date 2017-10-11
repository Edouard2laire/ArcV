#include <array>
#include <fstream>
#include <sstream>
#include <iostream>

#include "png/png.h"
#include "png/zlib.h"
#include "ArcV/Processing/Image.hpp"

namespace Arcv {

namespace Image {

namespace {

const uint8_t PNG_HEADER_SIZE = 8;

const std::string extractFileExt(const std::string& fileName) {
  return (fileName.substr(fileName.find_last_of('.') + 1));
}

bool validatePng(std::istream& file) {
  std::array<png_byte, PNG_HEADER_SIZE> header;
  file.read(reinterpret_cast<char*>(header.data()), PNG_HEADER_SIZE);

  return (png_sig_cmp(header.data(), 0, PNG_HEADER_SIZE) == 0);
}

Matrix<> readJpeg(const std::string& fileName) {
  std::cerr << "Error: JPEG reading not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

Matrix<> readPng(const std::string& fileName) {
  std::ifstream file(fileName, std::ios_base::in | std::ios_base::binary);
  const bool valid = file.good() && validatePng(file);
  assert(("Error: Not a valid PNG", valid));

  png_structp pngReadStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  assert(("Error: Couldn't initialize PNG read struct", pngReadStruct));

  png_infop pngInfoStruct = png_create_info_struct(pngReadStruct);
  assert(("Error: Couldn't initialize PNG info struct", pngInfoStruct));

  png_set_read_fn(pngReadStruct, &file, [] (png_structp pngReadPtr, png_bytep data, png_size_t length) {
    png_voidp inPtr = png_get_io_ptr(pngReadPtr);
    static_cast<std::istream*>(inPtr)->read(reinterpret_cast<char*>(data), length);
  });

  // Setting the amount signature bytes we've already read
  png_set_sig_bytes(pngReadStruct, PNG_HEADER_SIZE);

  png_read_info(pngReadStruct, pngInfoStruct);

  uint32_t width = png_get_image_width(pngReadStruct, pngInfoStruct);
  uint32_t height = png_get_image_height(pngReadStruct, pngInfoStruct);
  uint8_t bitDepth = png_get_bit_depth(pngReadStruct, pngInfoStruct);
  uint8_t channels = png_get_channels(pngReadStruct, pngInfoStruct);
  uint8_t colorType = png_get_color_type(pngReadStruct, pngInfoStruct);
  Colorspace colorspace;

  switch (colorType) {
    case PNG_COLOR_TYPE_GRAY:
      if (bitDepth < 8)
        png_set_expand_gray_1_2_4_to_8(pngReadStruct);

      colorspace = ARCV_COLORSPACE_GRAY;
      bitDepth = 8;
      break;

    case PNG_COLOR_TYPE_GRAY_ALPHA:
      colorspace = ARCV_COLORSPACE_GRAY_ALPHA;
      break;

    case PNG_COLOR_TYPE_PALETTE:
      png_set_palette_to_rgb(pngReadStruct);
      colorspace = ARCV_COLORSPACE_RGB;
      channels = 3;
      break;

    case PNG_COLOR_TYPE_RGB:
    default:
      colorspace = ARCV_COLORSPACE_RGB;
      break;

    case PNG_COLOR_TYPE_RGB_ALPHA:
      colorspace = ARCV_COLORSPACE_RGBA;
      break;
  }

  png_set_scale_16(pngReadStruct);

  // Adding full alpha channel to the image if it possesses transparency
  if (png_get_valid(pngReadStruct, pngInfoStruct, static_cast<png_uint_32>(PNG_INFO_tRNS))) {
    png_set_tRNS_to_alpha(pngReadStruct);
    ++channels;
  }

  png_read_update_info(pngReadStruct, pngInfoStruct);

  Matrix<uint8_t> mat(width, height, channels, bitDepth, colorspace);

  std::vector<png_bytep> rowPtrs(height);

  // Mapping row's elements to data's
  for (unsigned int i = 0; i < height; ++i)
    rowPtrs[i] = &mat.getData()[width * channels * i];

  png_read_image(pngReadStruct, rowPtrs.data());
  png_read_end(pngReadStruct, pngInfoStruct);
  png_destroy_read_struct(&pngReadStruct, nullptr, &pngInfoStruct);

  return Matrix<>(mat);
}

Matrix<> readTga(const std::string& fileName) {
  std::cerr << "Error: TGA reading not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

Matrix<> readBmp(const std::string& fileName) {
  std::cerr << "Error: BMP reading not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

Matrix<> readBpg(const std::string& fileName) {
  std::cerr << "Error: BPG reading not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

void writeJpeg(const Matrix<>& mat, const std::string& fileName) {
  std::cerr << "Error: JPEG output not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

void writePng(const Matrix<>& mat, const std::string& fileName) {
  std::ofstream file(fileName, std::ios_base::out | std::ios_base::binary);
  const Matrix<uint8_t> matToWrite(mat);

  png_structp pngWriteStruct = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
  assert(("Error: Couldn't initialize PNG write struct", pngWriteStruct));

  png_infop pngInfoStruct = png_create_info_struct(pngWriteStruct);
  assert(("Error: Couldn't initialize PNG info struct", pngInfoStruct));

  uint32_t colorType;
  switch (matToWrite.getColorspace()) {
    case ARCV_COLORSPACE_GRAY:
      colorType = PNG_COLOR_TYPE_GRAY;
      break;

    case ARCV_COLORSPACE_GRAY_ALPHA:
      colorType = PNG_COLOR_TYPE_GRAY_ALPHA;
      break;

    case ARCV_COLORSPACE_RGB:
    case ARCV_COLORSPACE_HSV:
    default:
      colorType = PNG_COLOR_TYPE_RGB;
      break;

    case ARCV_COLORSPACE_RGBA:
      colorType = PNG_COLOR_TYPE_RGBA;
      break;
  }

  png_set_compression_level(pngWriteStruct, 6);

  if (matToWrite.getChannelCount() * matToWrite.getImgBitDepth() >= 16) {
    png_set_compression_strategy(pngWriteStruct, Z_FILTERED);
    png_set_filter(pngWriteStruct, 0, PNG_FILTER_NONE | PNG_FILTER_SUB | PNG_FILTER_PAETH);
  } else {
    png_set_compression_strategy(pngWriteStruct, Z_DEFAULT_STRATEGY);
  }

  png_set_IHDR(pngWriteStruct,
               pngInfoStruct,
               static_cast<png_uint_32>(matToWrite.getWidth()),
               static_cast<png_uint_32>(matToWrite.getHeight()),
               matToWrite.getImgBitDepth(),
               colorType,
               PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE,
               PNG_FILTER_TYPE_BASE);

  png_set_write_fn(pngWriteStruct, &file, [] (png_structp pngWritePtr, png_bytep data, png_size_t length) {
    png_voidp outPtr = png_get_io_ptr(pngWritePtr);
    static_cast<std::ostream*>(outPtr)->write(reinterpret_cast<const char*>(data), length);
  }, nullptr);
  png_write_info(pngWriteStruct, pngInfoStruct);

  for (unsigned int i = 0; i < matToWrite.getHeight(); ++i)
    png_write_row(pngWriteStruct, &matToWrite.getData()[matToWrite.getWidth() * matToWrite.getChannelCount() * i]);

  png_write_end(pngWriteStruct, pngInfoStruct);
  png_destroy_write_struct(&pngWriteStruct, &pngInfoStruct);
}

void writeTga(const Matrix<>& mat, const std::string& fileName) {
  std::cerr << "Error: TGA output not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

void writeBmp(const Matrix<>& mat, const std::string& fileName) {
  std::cerr << "Error: BMP output not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

void writeBpg(const Matrix<>& mat, const std::string& fileName) {
  std::cerr << "Error: BPG output not yet implemented" << std::endl;
  exit(EXIT_FAILURE);
}

} // namespace

Matrix<> read(const std::string& fileName) {
  const std::string format = extractFileExt(fileName);

  if (format == "jpg" || format == "jpeg" || format == "JPG" || format == "JPEG")
    return readJpeg(fileName);
  else if (format == "png" || format == "PNG")
    return readPng(fileName);
  else if (format == "tga" || format == "TGA")
    return readTga(fileName);
  else if (format == "bmp" || format == "BMP")
    return readBmp(fileName);
  else if (format == "bpg" || format == "BPG")
    return readBpg(fileName);
  else
    std::cerr << "Error: '" << format << "' format is not supported" << std::endl;

  exit(EXIT_FAILURE);
}

void write(const Matrix<>& mat, const std::string& fileName) {
  const std::string format = extractFileExt(fileName);

  if (format == "jpg" || format == "jpeg" || format == "JPG" || format == "JPEG")
    writeJpeg(mat, fileName);
  else if (format == "png" || format == "PNG")
    writePng(mat, fileName);
  else if (format == "tga" || format == "TGA")
    writeTga(mat, fileName);
  else if (format == "bmp" || format == "BMP")
    writeBmp(mat, fileName);
  else if (format == "bpg" || format == "BPG")
    writeBpg(mat, fileName);
  else
    std::cerr << "Error: '" << format << "' format is not supported" << std::endl;
}

} // namespace Image

} // namespace Arcv