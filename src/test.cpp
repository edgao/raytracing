#include <float.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Class.h"

//int Shape::curID = 0;

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
  int width = 400, height = 400;
  Color pixels[width * height];
  Vector3f cam;
  Vector3f ul, ur, ll, lr;
  cam <<   0,   0, 50;
  ul  << -10,  10, 40;
  ur  <<  10,  10, 40;
  ll  << -10, -10, 40;
  lr  <<  10, -10, 40;
  /*
  cam << 2, 0, 1;
  ul << 3, -4, 2;
  ur << 3, 4, 2;
  ll << 3, -4, -2;
  lr << 3, 4, -2;*/

  Transformation identity;
  Transformation stretch = Transformation(MatrixUtils::createScalingMatrix(0.5, 0.5, 0.5));
  BRDF mat = BRDF(Color(0, 0, 0), Color(0, 0, 0), Color(1, 1, 1), Color(1, 1, 1), 64);
  // Set up some shapes
  vector<Shape*> shapes;
  shapes.push_back(new Sphere((Vector3f() << 0, 0, 0).finished(), 8, stretch, mat));
  //shapes.push_back(new Sphere((Vector3f() << 8, 8, 8).finished(), 4, identity, mat));
  //shapes.push_back(new Sphere((Vector3f() << 8, -8, 8).finished(), 4, identity, mat));
  //shapes.push_back(new Sphere((Vector3f() << -8, 8, 8).finished(), 4, identity, mat));
  //shapes.push_back(new Sphere((Vector3f() << -8, -8, 8).finished(), 4, identity, mat));
  //shapes.push_back(new Triangle((Vector3f() << 10, 0, 150).finished(), (Vector3f() << 10, 150, -1500).finished(), (Vector3f() << 10, -150, -1500).finished(), identity, BRDF(Color(0.0, 0.0, 0.0), Color(0.0, 0.0, 0.0), Color(1, 1, 1), Color(1, 1, 1), 64)));
  //shapes.push_back(new Triangle((Vector3f() << -10, 0, 150).finished(), (Vector3f() << -10, -150, -1500).finished(), (Vector3f() << -10, 150, -1500).finished(), identity, BRDF(Color(0.0, 0.0, 0.0), Color(0.0, 0.0, 0.0), Color(1, 1, 1), Color(1, 1, 1), 64)));
  // Set up some lights
  vector<Light*> lights;
  lights.push_back(new DirectionalLight((Vector3f() << -1, -1, -1).finished(), Color(0.5, 0.5, 0.5)));
  lights.push_back(new DirectionalLight((Vector3f() <<  1, -1, -1).finished(), Color(0.5, 0.5, 0.5)));
  lights.push_back(new DirectionalLight((Vector3f() <<  0,  1, -1).finished(), Color(0.5, 0.5, 0.5)));
  //lights.push_back(new PointLight((Vector3f() << 9, 0,  5).finished(), Color(1, 1, 1), PointLight::NO_FALLOFF));
  //lights.push_back(new PointLight((Vector3f() << 9, 0, -5).finished(), Color(1, 1, 1), PointLight::NO_FALLOFF));
  Color ambient = Color(0.1, 0.1, 0.1);

  Raytracer tracer = Raytracer(&shapes, &lights, ambient);
  for (int r = 0; r < height; r++) {
    if (r % 20 == 0) printf("%6.3f%%\n", 100.0 * r / height);
    for (int c = 0; c < width; c++) {
      float u, v;
      u = 1 - 1.0 * c / width;
      v = 1.0 * r / height;
      Vector3f pos = (u * (v * ll + ((1 - v) * ul))) + ((1 - u) * (v * lr + ((1 - v) * ur)));
      Ray ray = Ray(pos, pos - cam, 0, FLT_MAX);
      Color res = tracer.trace(ray, 5);
      pixels[r * width + c] = res;
    }
  }
  printf("Writing to file\n");
  writePPM(width, height, pixels, "output.ppm");
}
