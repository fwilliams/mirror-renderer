#include <SOIL/SOIL.h>
#include <GL/glew.h>

#include <string>
#include <exception>
#include <cstdint>
#include <array>


#ifndef TEXTURE_H_
#define TEXTURE_H_

uint8_t* load_ppm(const std::string& filename, size_t& outWidth, size_t& outHeight,
    size_t& outBytesPerChannel) {
  FILE* fp;
  int i, w, h, d;
  uint8_t* image;
  char head[70];   // max line <= 70 in PPM (per spec).

  fp = fopen(filename.c_str(), "rb");
  if (!fp) {
    throw std::runtime_error(
        std::string("Error: Failed to open file, ") + filename + std::string(" with error: ")
            + std::string(strerror(errno)));
  }

  // Grab first two chars of the file and make sure that it has the
  // correct magic cookie for a raw PPM file.
  fgets(head, 70, fp);
  if (strncmp(head, "P6", 2)) {
    throw std::runtime_error(
        std::string("Error: ") + filename
            + std::string(": Not a raw PPM file. First bytes must be \"P6\"."));
  }

  // Grab the three elements in the header (width, height, maxval).
  i = 0;
  while (i < 3) {
    fgets(head, 70, fp);
    if (head[0] == '#') {  // skip comments.
      continue;
    }
    if (i == 0) {
      i += sscanf(head, "%d %d %d", &w, &h, &d);
    } else if (i == 1) {
      i += sscanf(head, "%d %d", &h, &d);
    } else if (i == 2) {
      i += sscanf(head, "%d", &d);
    }
  }

  size_t sizeofChannel = 0;
  if (d < 256) {
    sizeofChannel = sizeof(uint8_t);
  } else if (d < 65536) {
    sizeofChannel = sizeof(uint16_t);
  } else if (d < 4294967296) {
    sizeofChannel = sizeof(uint32_t);
  } else {
    throw std::runtime_error(
        std::string("Error: PPM channel depth, ") + std::to_string(d)
            + std::string(", requires greater than 32 bits of space."));
  }

  const size_t imageSize = sizeofChannel * w * h * 3;

  // Grab all the image data in one fell swoop.
  image = (uint8_t*) malloc(imageSize);
  if (image == NULL) {
    throw std::runtime_error(
        std::string("Critical Error: malloc of size ") + std::to_string(imageSize)
            + std::string(" returned NULL."));
  }
  fread(image, sizeof(unsigned char), imageSize, fp);
  fclose(fp);

  outWidth = w;
  outHeight = h;
  outBytesPerChannel = sizeofChannel;
  return image;
}

void load_ppm_to_gl_tex2darray(const std::string& filename, GLuint texarray, size_t arrayIndex) {
  // Load the image into memory
  size_t w, h, channelSize;
  unsigned char* img = load_ppm(filename, w, h, channelSize);

  GLenum type;
  switch (channelSize) {
  case sizeof(uint8_t):
    type = GL_UNSIGNED_BYTE;
    break;
  case sizeof(uint16_t):
    type = GL_UNSIGNED_SHORT;
    break;
  case sizeof(uint32_t):
    type = GL_UNSIGNED_INT;
    break;
  default:
    throw(std::runtime_error(
        std::string("Error: Invalid image channel size. Expecting, 8, 16, or 32 bits, got ")
            + std::to_string(channelSize * 8) + std::string(" bits.")));
  }

  glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_TRUE);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texarray);
  check_gl_error();
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, // Mipmap level
      0, 0, arrayIndex, // x-offset, y-offset, z-offset
      w, h, 1, // width, height, depth
      GL_RGB, type, img);
  glPixelStorei(GL_UNPACK_SWAP_BYTES, GL_FALSE);
  check_gl_error();
  free(img);
}

void load_image_to_gl_tex2darray(const std::string& filename, GLuint tex, size_t arrayIndex) {
  // Load the image into memory
  int w, h, channels;
  unsigned char* img = SOIL_load_image(filename.c_str(), &w, &h, &channels, SOIL_LOAD_AUTO);

  if (img == 0) {
    // TODO: Autoload PPM image if we get one
    throw std::runtime_error(
        std::string("SOIL Failed to load texture: ") + filename + std::string(": ")
            + std::string(SOIL_last_result()));
  }

  GLenum format = 0;
  switch (channels) {
  case 4:
    format = GL_RGBA;
    break;
  case 3:
    format = GL_RGB;
    break;
  case 2:
    format = GL_RG;
    break;
  case 1:
    format = GL_RED;
    break;
  default:
    throw(std::runtime_error(
        std::string("Invalid number of image channels. Expecting 1, 2, 3, or 4, got ")
            + std::to_string(channels)));
  }

  // SOIL only supports textures with 8 bits per channel so we can safely use
  // GL_UNSIGNED_BYTE as the type argument
  glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, // Mipmap level
      0, 0, arrayIndex, // x-offset, y-offset, z-offset
      w, h, 1, // width, height, depth
      format, GL_UNSIGNED_BYTE, img);
  SOIL_free_image_data(img);
}

GLuint make_gl_tex2darray(size_t w, size_t h, size_t n, GLenum interalFormat = GL_RGBA8) {
  GLuint texArray;
  glGenTextures(1, &texArray);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, interalFormat, w, h, n);

  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

  return texArray;
}

template <size_t N>
std::array<GLuint, N> make_gl_tex2darrays(size_t w, size_t h, size_t n, GLenum interalFormat = GL_RGBA8) {
  std::array<GLuint, N> texArrays;
  glGenTextures(N, &texArrays[0]);

  for(auto texArray : texArrays) {
    glBindTexture(GL_TEXTURE_2D_ARRAY, texArray);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, interalFormat, w, h, n);
  }

  glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

  return texArrays;
}

#endif /* TEXTURE_H_ */
