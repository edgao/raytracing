#include <float.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Class.h"

int Shape::curID = 0;

void writePPM(int dimx, int dimy, Color* pixels, const char* filename) {
  int i, j;
  FILE *fp = fopen(filename, "wb"); /* b - binary mode */
  (void) fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);
  for (j = 0; j < dimy; ++j)
  {
    for (i = 0; i < dimx; ++i)
    {
      int pixInd = j * dimx + i;
      if (pixels[pixInd].r > 1) pixels[pixInd].r = 1;
      if (pixels[pixInd].g > 1) pixels[pixInd].g = 1;
      if (pixels[pixInd].b > 1) pixels[pixInd].b = 1;
      static unsigned char color[3];
      color[0] = (unsigned char) (255 * pixels[pixInd].r);  /* red */
      color[1] = (unsigned char) (255 * pixels[pixInd].g);  /* green */
      color[2] = (unsigned char) (255 * pixels[pixInd].b);  /* blue */
      (void) fwrite(color, 1, 3, fp);
    }
  }
  (void) fclose(fp);
}

int main() {
  int width = 800, height = 800;
  Color pixels[width * height];
  Vector3f cam;
  Vector3f ul, ur, ll, lr;
  cam <<   0,   0, 50;
  ul  << -10,  10, 10;
  ur  <<  10,  10, 10;
  ll  << -10, -10, 10;
  lr  <<  10, -10, 10;

  Transformation identity;
  BRDF floorMat = BRDF(Color(0.1, 0.1, 0.1), Color(0.5, 0.5, 0.5), Color(1, 1, 1), Color(0, 0, 0), 64);
  // Set up some shapes
  unsigned int shapes_c = 2;
  Shape** shapes = (Shape**) malloc(sizeof(Shape*) * shapes_c);
  shapes[0] = new Triangle((Vector3f() <<  -2, 2, 2).finished(), (Vector3f() <<  0, -2, 2).finished(), (Vector3f() <<  2, 2, 2).finished(), identity, floorMat);
  shapes[1] = new Triangle((Vector3f() <<  -4, 4, 0).finished(), (Vector3f() <<  0, -4, 0).finished(), (Vector3f() <<  4, 4, 0).finished(), identity, floorMat);
  // Set up some lights
  unsigned int lights_c = 1;
  Light** lights = (Light**) malloc(sizeof(Light*) * lights_c);
  lights[0] = new DirectionalLight((Vector3f() << -1, -1, -1).finished(), Color(1, 1, 1));
  Color ambient = Color(0.05, 0.05, 0.05);

  Raytracer tracer = Raytracer(shapes_c, shapes, lights_c, lights, ambient);
  for (int r = 0; r < height; r++) {
    if (r % 20 == 0) printf("%6.3f%%\n", 100.0 * r / height);
    for (int c = 0; c < width; c++) {
      float u, v;
      u = 1 - 1.0 * c / width;
      v = 1.0 * r / height;
      Vector3f pos = (u * (v * ll + ((1 - v) * ul))) + ((1 - u) * (v * lr + ((1 - v) * ur)));
      Ray ray = Ray(pos, pos - cam, 0, FLT_MAX);
      Color res = tracer.trace(ray, 1);
      pixels[r * width + c] = res;
    }
  }
  printf("Writing to file\n");
  writePPM(width, height, pixels, "output.ppm");
}
