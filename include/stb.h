
#ifndef STB_IMAGE_WRITE_H
#define STB_IMAGE_WRITE_H

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef STB_IMAGE_WRITE_STATIC
#define STBIWDEF static
#else
#define STBIWDEF extern
#endif

STBIWDEF int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data);

#ifdef __cplusplus
}
#endif

#endif // STB_IMAGE_WRITE_H

#ifdef STB_IMAGE_WRITE_IMPLEMENTATION

STBIWDEF int stbi_write_bmp(char const *filename, int x, int y, int comp, const void *data)
{
   FILE *f = fopen(filename, "wb");
   if (!f) return 0;

   int pad = (4 - (x * 3) % 4) % 4;
   int filesize = 54 + (x * 3 + pad) * y;

   // BMP Header
   const unsigned char header[54] = {
      'B','M',  // Magic
      filesize&0xFF, (filesize>>8)&0xFF, (filesize>>16)&0xFF, (filesize>>24)&0xFF,  // File size
      0,0,0,0,  // Reserved
      54,0,0,0, // Pixel data offset
      40,0,0,0, // DIB header size
      x&0xFF, (x>>8)&0xFF, (x>>16)&0xFF, (x>>24)&0xFF,  // Width
      y&0xFF, (y>>8)&0xFF, (y>>16)&0xFF, (y>>24)&0xFF,  // Height
      1,0,      // Planes
      24,0,     // Bits per pixel
      0,0,0,0,  // Compression (none)
      0,0,0,0,  // Image size (can be 0 for uncompressed)
      0,0,0,0,  // X pixels per meter
      0,0,0,0,  // Y pixels per meter
      0,0,0,0,  // Colors in palette
      0,0,0,0   // Important colors
   };

   fwrite(header, 1, 54, f);

   // Write pixel data (BMP is BGR and bottom-to-top)
   const unsigned char padding[3] = {0,0,0};
   for (int j = y-1; j >= 0; j--) {
      for (int i = 0; i < x; i++) {
         const unsigned char *pixel = (const unsigned char*)data + (j * x + i) * comp;
         // Write BGR
         fputc(pixel[2], f);  // B
         fputc(pixel[1], f);  // G
         fputc(pixel[0], f);  // R
      }
      fwrite(padding, 1, pad, f);
   }

   fclose(f);
   return 1;
}

#endif // STB_IMAGE_WRITE_IMPLEMENTATION
