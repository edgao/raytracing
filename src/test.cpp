#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "Class.cpp"

using namespace std;

void writePPM(int dimx, int dimy, Color* pixels, const char* filename) {
  int i, j;
  FILE *fp = fopen(filename, "wb"); /* b - binary mode */
  (void) fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);
  for (j = 0; j < dimy; ++j)
  {
    for (i = 0; i < dimx; ++i)
    {
      int pixInd = j * dimx + i;
      static unsigned char color[3];
      color[0] = (int) (255 * pixels[pixInd].r);  /* red */
      color[1] = (int) (255 * pixels[pixInd].g);  /* green */
      color[2] = (int) (255 * pixels[pixInd].b);  /* blue */
      (void) fwrite(color, 1, 3, fp);
    }
  }
  (void) fclose(fp);
}

int main() {
  int width = 100, height = 100;
  Color pixels[width * height];
  Vector3f cam;
  Vector3f ul, ur, ll, lr;
  cam << 0, 0, 0;
  ul << 2, 2, 2;
  ur << 2, -2, 2;
  ll << 2, 2, -2;
  lr << 2, -2, -2;

  Transformation identity;
  BRDF mat = BRDF(Color(1, 1, 1), Color(), Color(), Color(), 1);
  // Set up some shapes
  unsigned int shapes_c = 1;
  Shape** shapes = (Shape**) malloc(sizeof(Shape*) * 1);
  shapes[0] = new Sphere((Vector3f() << 5, 0, 0).finished(), 1, identity, mat);
  // Set up some lights
  unsigned int lights_c = 1;
  Light** lights = (Light**) malloc(sizeof(Light*) * lights_c);
  lights[0] = new DirectionalLight((Vector3f() << 1, 0, 0).finished(), Color(1, 1, 1));
  Color ambient = Color();

  Raytracer tracer = Raytracer(shapes_c, shapes, lights_c, lights, ambient);
  for (int r = 0; r < height; r++) {
    for (int c = 0; c < width; c++) {
      float u, v;
      u = 1.0 * c / width;
      v = 1.0 * r / height;
      Vector3f pos = (u * (ll + ((1 - v) * ul))) + ((1 - u) * (lr + ((1 - v) * ur)));
      if (r == height / 2 && c == width / 2) {
	printf("%d %d %f %f\n", r, c, u, v);
	cout << pos << endl;
	cout << (pos - cam) << endl;
      }
      Ray ray = Ray(pos, pos - cam, 0, FLT_MAX);
      Color res = tracer.trace(ray, 1);
      pixels[r * width + c] = res;
    }
    if (r % 25 == 0) printf("%2.3f%%\n", 100.0 * r / height);
  }
  writePPM(width, height, pixels, "output.ppm");
}
