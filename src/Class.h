#include <eigen3/Dense>
#include <vector>
using namespace Eigen;
using namespace std;

class Ray {
 public:
  Vector3f pos, dir;
  float t_min, t_max;
  Ray();
  Ray(Vector3f, Vector3f, float, float);
  Vector3f evaluate(float);
};

class LocalGeo {
 public:
  Vector3f pos;
  Vector3f normal;
  LocalGeo();
  LocalGeo(Vector3f p, Vector3f n);
};

class MatrixUtils {
 public:
  static Matrix4f createTranslationMatrix(float, float, float);
  static Matrix4f createScalingMatrix(float, float, float);
  static Matrix4f createRotationMatrix(float, float, float);
};

class Transformation {
 public:
  Matrix4f m, minv, minvt, mt;
  Transformation();
  Transformation(Matrix4f m);
  static Vector3f doTransformation(Vector3f vec, Matrix4f mat, float ext);
  Vector3f transformPoint(Vector3f vec);
  Vector3f transformNormal(Vector3f vec);
  Ray transformRay(Ray ray);
  Ray transformRayToLocalCoords(Ray ray);
  LocalGeo transformLocalGeo(LocalGeo geo);
  Transformation chainTransformation(Matrix4f);
};

class Color {
 public:
  float r, g, b;
  Color();
  Color(float, float, float);
  Color add(Color);
  Color sub(Color);
  Color scale(float);
  Color mul(Color);
};

class BRDF {
 public:
  Color kd, ks, ka, kr;
  float specExp;
  BRDF();
  BRDF(Color, Color, Color, Color, float);
};

class Shape {
 public:
  static int curID;
  Transformation transform;
  BRDF brdf;
  int id;
  Shape();
  Shape(Transformation, BRDF);
  virtual bool intersect(Ray&, float*, LocalGeo*) { return false; };
  bool intersectP(Ray&);
};

class Triangle : public Shape {
 public:
  Vector3f p1, p2, p3;
  Vector3f n1, n2, n3;
  Triangle(Vector3f, Vector3f, Vector3f, Transformation, BRDF);
  Triangle(Vector3f, Vector3f, Vector3f, Vector3f, Vector3f, Vector3f, Transformation, BRDF);
  bool intersect(Ray&, float*, LocalGeo*);
};

class Sphere : public Shape {
 public:
  Vector3f center;
  float radius;
  Sphere(Vector3f, float, Transformation, BRDF);
  bool intersect(Ray&, float*, LocalGeo*);
};

class Light {
 public:
  Color color;
  virtual void generateLightRay(LocalGeo&, Ray*, Color*) {};
};

class PointLight : public Light {
 public:
  static const int NO_FALLOFF = 0, LINEAR_FALLOFF = 1, QUADRATIC_FALLOFF = 2;
  Vector3f point;
  int falloff;
  PointLight(Vector3f, Color, int);
  void generateLightRay(LocalGeo&, Ray*, Color*);
};

class DirectionalLight : public Light {
 public:
  Vector3f direction;
  DirectionalLight(Vector3f, Color);
  void generateLightRay(LocalGeo&, Ray*, Color*);
};

class Raytracer {
 public:
  vector<Shape*> shapes;
  vector<Light*> lights;
  Color ambient_lights;
  Raytracer(vector<Shape*>*, vector<Light*>*, Color);
  Vector3f reflectVector(Vector3f vec, Vector3f axis);
  Color shade(LocalGeo& geo, Ray& lightRay, Color lightColor, Vector3f viewer, BRDF brdf);
  bool firstObjectHit(Ray& ray, Shape* ignore, Shape* shape, float* thit, LocalGeo* geo);
  bool firstObjectHitP(Ray& ray, Shape* ignore);
  Color trace(Ray& ray, unsigned int depth);
  Color trace(Ray& ray, unsigned int depth, Shape*);
};
