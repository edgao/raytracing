#include <eigen3/Dense>
#include <float.h>
// for ging; remove later
#include <iostream>
#include <cstdio>

using namespace std;
using namespace Eigen;

inline bool inRange(float x, float min, float max) {
  return min <= x && x <= max;
}

class Ray {
public:
  Vector3f pos;
  Vector3f dir;
  float t_min, t_max;
  Ray() {}
  Ray(Vector3f p, Vector3f d, float t_min, float t_max) {
    pos = p;
    dir = d;
    this->t_min = t_min;
    this->t_max = t_max;
  }
  Vector3f evaluate(float t) {
    Vector3f ret;
    ret << pos(0) + t * dir(0), pos(1) + t * dir(1), pos(2) + t * dir(2);
    return ret;
  }
};

class LocalGeo {
public:
  Vector3f pos;
  Vector3f normal;
  LocalGeo() {}
  LocalGeo(Vector3f p , Vector3f n) {
    pos = p;
    normal = n.normalized();
  }
};

class MatrixUtils {
public:
  static Matrix4f createTranslationMatrix(float tx, float ty, float tz) {
    Matrix4f ret;
    ret <<
      1, 0, 0, tx,
      0, 1, 0, ty,
      0, 0, 1, tz,
      0, 0, 0, 1;
    return ret;
  }
  static Matrix4f createScalingMatrix(float sx, float sy, float sz) {
    Matrix4f ret;
    ret <<
      sx, 0, 0, 0,
      0, sy, 0, 0,
      0, 0, sz, 0,
      0, 0, 0, 1;
    return ret;
  }
  static Matrix4f buildRotationMatrix(float rx, float ry, float rz) {
    Matrix4f x, y, z;
    Matrix4f m;
    x <<
      1, 0, 0, 0,
      0, cos(rx), -sin(rx), 0,
      0, sin(rx), cos(rx), 0,
      0, 0, 0, 1;
    y <<
      cos(ry), 0, sin(ry), 0,
      0, 1, 0, 0,
      -sin(ry), 0, cos(ry), 0,
      0, 0, 0, 1;
    z <<
      cos(rz), -sin(rz), 0, 0,
      sin(rz), cos(rz), 0, 0,
      0, 0, 1, 0,
      0, 0, 0, 1;
    m = x * y * z;
    return m;
  }
  static Matrix4f buildTranslationMatrix(float tx, float ty, float tz) {
    Matrix4f ret;
    ret <<
      1, 0, 0, tx,
      0, 1, 0, ty,
      0, 0, 1, tz,
      0, 0, 0, 1;
    return ret;
  }
  static Matrix4f buildScaleMatrix(float sx, float sy, float sz) {
    Matrix4f ret;
    ret <<
      sx, 0, 0, 0,
      0, sy, 0, 0,
      0, 0, sz, 0,
      0, 0, 0, 1;
    return ret;
  }
};

class Transformation {
public:
  Matrix4f m, minv, minvt, mt;
  int type;
  Transformation() {
    m = minv = minvt = mt = (Matrix4f() << 1, 0, 0, 0,
                                           0, 1, 0, 0,
			                   0, 0, 1, 0,
			                   0, 0, 0, 1).finished();
  }
  Transformation(Matrix4f m) {
    this->m = m;
    this->minv = m.inverse();
    this->minvt = minv.transpose();
    this->mt = m.transpose();
  }
  static Vector3f doTransformation(Vector3f vec, Matrix4f mat, float ext) {
    Vector4f temp;
    temp << vec(0), vec(1), vec(2), ext;
    temp = mat * temp;
    Vector3f ret;
    ret << temp(0), temp(1), temp(2);
    return ret;
  }
  Vector3f transformPoint(Vector3f vec) {
    return doTransformation(vec, m, 1);
  }
  Vector3f transformNormal(Vector3f vec) {
    return doTransformation(vec, minvt, 0).normalized();
  }
  Ray transformRay(Ray ray) {
    Ray ret(transformPoint(ray.pos), transformPoint(ray.dir), ray.t_min, ray.t_max);
    return ret;
  }
  // Rather than transforming the shapes themselves, transform the rays into the shapes' local coords
  Ray transformRayToLocalCoords(Ray ray) {
    return Ray(doTransformation(ray.pos, minv, 1), doTransformation(ray.dir, mt, 0), ray.t_min, ray.t_max);
  }
  LocalGeo transformLocalGeo(LocalGeo geo) {
    LocalGeo ret(transformPoint(geo.pos), transformNormal(geo.normal));
    return ret;
  }
};

class Color {
public:
  float r, g, b;
  Color() {
    r = g = b = 0;
  }
  Color(float r, float g, float b) {
    this->r = r;
    this->g = g;
    this->b = b;
  }
  Color add(Color o) {return Color(r + o.r, g + o.g, b + o.b); }
  Color sub(Color o) { return Color(r - o.r, g - o.g, b - o.b); }
  Color scale(float s) { return Color(r * s, g * s, b * s); }
  Color mul(Color o) { return Color(r * o.r, g * o.g, b * o.b); }
};

class BRDF {
public:
  Color kd, ks, ka, kr;
  float specExp;
  BRDF() {
    kd = ks = ka = kr = Color();
    specExp = 1;
  }
  BRDF(Color kd, Color ks, Color ka, Color kr, float ns) {
    this->kd = kd;
    this->ks = ks;
    this->ka = ka;
    this->kr = kr;
    specExp = ns;
  }
};

class Shape {
public:
  Transformation transform;
  BRDF brdf;
  Shape() {
    transform = Transformation();
    brdf = BRDF();
  }
  Shape(Transformation t, BRDF brdf) {
    transform = t;
    this->brdf = brdf;
  }
  virtual bool intersect(Ray&, float*, LocalGeo*) {return false;}
  bool intersectP(Ray& ray) {
    float* thit;
    LocalGeo* geo;
    return intersect(ray, thit, geo);
  }
};

class Triangle : public Shape {
public:
  Vector3f p1, p2, p3;
  Vector3f n1, n2, n3;
  // Constructs a triangle with the default vertex normals (p2 - p1) x (p3 - p1) i.e. flat triangle
  Triangle(Vector3f p1, Vector3f p2, Vector3f p3, Transformation t, BRDF brdf) : Shape(t, brdf) {
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    n1 = n2 = n3 = (p2 - p1).cross(p3 - p1);
    this->n1.normalize();
    this->n2.normalize();
    this->n3.normalize();
  }
  // Constructs a triangle with the given vertex normals
  Triangle(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f n1, Vector3f n2, Vector3f n3, Transformation t, BRDF brdf) : Shape(t, brdf) {
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    this->n1 = n1.normalized();
    this->n2 = n2.normalized();
    this->n3 = n3.normalized();
  }
  bool intersect(Ray& world_ray, float* thit, LocalGeo* geo) {
    // TODO Test ray transformation
    Ray ray = transform.transformRayToLocalCoords(world_ray);
    Matrix3f mat;
    mat << -ray.dir, p2 - p1, p3 - p1;
    Vector3f sol = mat.householderQr().solve(ray.pos - p1);
    if (inRange(sol(0), ray.t_min, ray.t_max) && inRange(sol(1), 0, 1) && inRange(sol(2), 0, 1) && sol(1) + sol(2) <= 1) {
      // If this intersection occurs within the ray's lifespan, and is in the triangle proper
      *thit = sol(0);
      *geo = LocalGeo(ray.evaluate(*thit), sol(1) * n2 + sol(2) * n3 + (1 - sol(1) - sol(2)) * n1);
      return true;
    }
    return false;
  }
};

class Sphere : public Shape {
public:
  Vector3f center;
  float radius;
  Sphere(Vector3f c, float r, Transformation t, BRDF brdf) : Shape(t, brdf) {
    center = c;
    radius = r;
  }
  bool intersect(Ray& world_ray, float* thit, LocalGeo* geo) {
    // TODO Test ray transformation
    Ray ray = transform.transformRayToLocalCoords(world_ray);
    float a, b, c;
    a = b = c = 0;
    for (int i = 0; i < 3; i++) {
      float t = ray.pos(i) - center(i);
      a += ray.dir(i) * ray.dir(i);
      b += 2 * ray.dir(i) * t;
      c += t * t;
    }
    c -= radius;
    float t1, t2;
    t1 = (-b - sqrtf(b*b - 4 * a * c)) / (2 * a);
    t2 = (-b + sqrtf(b*b - 4 * a * c)) / (2 * a);
    printf("%f %f %f %f %f\n", a, b, c, t1, t2);
    if (isnan(t1)) {
      return false;
    }
    float trueHit = 0;
    bool goodHit = false;
    if (ray.t_min <= t1 && t1 <= ray.t_max) {
      trueHit = t1;
      goodHit = true;
    } else if (ray.t_min <= t2 && t2 <= ray.t_max) {
      trueHit = t2;
      goodHit = true;
    }
    if (goodHit) {
      *thit = trueHit;
      Vector3f pos = ray.evaluate(*thit);
      Vector3f normal;
      normal << pos(0) - center(0), pos(1) - center(1), pos(2) - center(2);
      normal.normalize();
      *geo = LocalGeo(pos, normal);
      return true;
    }
    return false;
  }
};

class Light {
public:
  Color color;
  virtual void generateLightRay(LocalGeo& local, Ray* ray, Color* color) {}
};

class PointLight : public Light {
public:
  static const int NO_FALLOFF = 0, LINEAR_FALLOFF = 1, QUADRATIC_FALLOFF = 2;
  Vector3f point;
  int falloff;
  PointLight(Vector3f p, Color c, int f) {
    point = p;
    color = c;
    falloff = f;
  }
  void generateLightRay(LocalGeo& local, Ray* ray, Color* color) {
    Vector3f direction = this->point - local.pos;
    *ray = Ray(local.pos, direction, 0, 1);
    switch (this->falloff) {
    case NO_FALLOFF:
      *color = this->color;
      break;
    case LINEAR_FALLOFF:
      *color = this->color.scale(1.0 / direction.norm());
      break;
    case QUADRATIC_FALLOFF:
      *color = this->color.scale(1.0 / direction.dot(direction));
      break;
    }
  }
};

class DirectionalLight : public Light {
public:
  Vector3f direction;
  DirectionalLight(Vector3f d, Color c) {
    direction = d;
    color = c;
  }
  void generateLightRay(LocalGeo& local, Ray* ray, Color* color) {
    *ray = Ray(local.pos, direction, 0, FLT_MAX);
    *color = this->color;
  }
};

class Raytracer {
public:
  // Pointer to array of pointers to Shapes
  unsigned int shapes_c;
  Shape** shapes;
  // Pointer to array of pointers to Lights
  unsigned int lights_c;
  Light** lights;
  // Ambient lighting is just the sum of all ambient lights
  Color ambient_lights;
  Raytracer(unsigned int sc, Shape** s, unsigned int lc, Light** l, Color al) {
    shapes_c = sc;
    shapes = s;
    lights_c = lc;
    lights = l;
    ambient_lights = al;
  }
  // axis must be a normalized vector
  Vector3f reflectVector(Vector3f vec, Vector3f axis) {
    return 2 * (axis * axis.dot(vec)) - vec;
  }
  Color shade(LocalGeo& geo, Light& light, Vector3f viewer, BRDF brdf) {
    Color res = Color();
    Ray lightRay;
    Color lightColor;
    light.generateLightRay(geo, &lightRay, &lightColor);
    // Specular shading
    float base = lightRay.dir.normalized().dot(geo.normal);
    if (base < 0) base = 0;
    res = res.add(lightColor.mul(brdf.kd).scale(base));
    // Diffuse shading
    Vector3f reflection = reflectVector(lightRay.dir, geo.normal);
    base = reflection.normalized().dot((geo.normal - viewer).normalized());
    if (base < 0)  base = 0;
    // TODO Figure out what the specular exponent is
    base = pow(base, 1);
    res = res.add(lightColor.mul(brdf.ks).scale(base));
  }
  bool firstObjectHit(Ray& ray, Shape* ignore, Shape* shape, float* thit, LocalGeo* geo) {
    Shape first_hit = Shape();
    float first_hit_t = FLT_MAX;
    LocalGeo first_hit_geo;
    bool had_hit = false;
    for (unsigned int i = 0; i < shapes_c; i++) {
      Shape* shape = shapes[i];
      if (shape == ignore) continue;
      float thit;
      LocalGeo geo;
      if (shape->intersect(ray, &thit, &geo) && inRange(thit, ray.t_min, ray.t_max)) {
	had_hit = true;
	if (thit < first_hit_t) {
	  first_hit_t = thit;
	  first_hit = *shape;
	  first_hit_geo = geo;
	}
      }
    }
    if (had_hit) {
      *shape = first_hit;
      *thit = first_hit_t;
      *geo = first_hit_geo;
    }
    return false;
  }
  bool firstObjectHitP(Ray& ray, Shape* ignore) {
    Shape blahs;
    float blaht;
    LocalGeo blahg;
    return firstObjectHit(ray, ignore, &blahs, &blaht, &blahg);
  }
  Color trace(Ray& ray, unsigned int depth) {
    if (depth == 0) return Color(0, 0, 0);
    Color ret = Color(0, 0, 0);
    // Find the first thing this ray hits
    Shape first_hit_shape;
    float first_hit_t;
    LocalGeo first_hit_geo;
    // If this ray never hits anything, return the color black
    if (!firstObjectHit(ray, NULL, &first_hit_shape, &first_hit_t, &first_hit_geo)) {
      return Color(0, 0, 0);
    }
    // TODO Remove debugging return statements
    return Color(1, 1, 1);
    // Shade this thing - for each light, if this point reaches the light, do shading
    for (unsigned int i = 0; i < lights_c; i++) {
      Ray light_ray;
      Color light_color;
      Light* light = lights[i];
      light->generateLightRay(first_hit_geo, &light_ray, &light_color);
      // If there is no intervening object, let's do shading
      if (!firstObjectHitP(ray, &first_hit_shape)) {
	ret = ret.add(shade(first_hit_geo, *light, ray.pos, first_hit_shape.brdf));
      }
    }
    // TODO Add ambient lighting - ret = ret.add(brdf.ka.mul(ambient_lights))
    // TODO Recurse for reflection and add the resulting colors
    return ret;
  }
};
