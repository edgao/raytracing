#include <eigen3/Dense>

using namespace Eigen;

inline bool inRange(float x, float min, float max) {
  return min <= x && x <= max;
}

class Ray {
public:
  Vector3f pos;
  Vector3f dir;
  float t_min, t_max;
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
  LocalGeo(Vector3f p , Vector3f n) {
    pos = p;
    normal = n;
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
  Matrix4f m, minvt;
  int type;
  Transformation(Matrix4f m) {
    this->m = m;
    this->minvt = m.inverse().transpose();
  }
  Vector3f transformPoint(Vector3f vec) {
    Vector4f temp;
    temp << vec(0), vec(1), vec(2), 1;
    temp = m * temp;
    Vector3f ret;
    ret << temp(0) / temp(3), temp(1) / temp(3), temp(2) / temp(3);
    return ret;
  }
  Vector3f transformNormal(Vector3f vec) {
    Vector4f temp;
    temp << vec(0), vec(1), vec(2), 0;
    temp = minvt * temp;
    Vector3f ret;
    ret << temp(0), temp(1), temp(2);
    ret.normalize();
    return ret;
  }
  Ray transformRay(Ray ray) {
    Ray ret(transformPoint(ray.pos), transformPoint(ray.dir), ray.t_min, ray.t_max);
    return ret;
  }
  LocalGeo transformLocalGeo(LocalGeo geo) {
    LocalGeo ret(transformPoint(geo.pos), transformNormal(geo.normal));
    return ret;
  }
};

class Color {
public:
  float r, g, b;
  Color(float r, float g, float b) {
    this->r = r;
    this->g = g;
    this->b = b;
  }
  Color* add(Color* o) { return new Color(r + o->r, g + o->g, b + o->b); }
  Color* sub(Color* o) { return new Color(r - o->r, g - o->g, b - o->b); }
  Color* scale(float s) { return new Color(r * s, g * s, b * s); }
};

class BRDF {
public:
  Color kd, ks, ka, kr;
};

class Sample {
public:
  float x, y;
};

class Shape {
public:
  virtual bool intersect(Ray&, float*, LocalGeo*);
  bool intersectP(Ray& ray) {
    float* thit;
    LocalGeo* geo;
    return intersect(ray, thit, geo);
  }
};

class Triangle : Shape {
public:
  Vector3f p1, p2, p3;
  Vector3f normal;
  Triangle(Vector3f p1, Vector3f p2, Vector3f p3) {
    this->p1 = p1;
    this->p2 = p2;
    this->p3 = p3;
    this->normal = (p3 - p1).cross(p2 - p1);
  }
  bool intersect(Ray& ray, float* thit, LocalGeo* geo) {
    Matrix3f mat;
    mat << ray.dir, p2 - p1, p3 - p1;
    Vector3f sol = mat.householderQr().solve(ray.pos - p1);
    if (inRange(sol(0), ray.t_min, ray.t_max) && inRange(sol(1), 0, 1) && inRange(sol(2), 0, 1) && sol(1) + sol(2) <= 1) {
      // If this intersection occurs within the ray's lifespan, and is in the triangle proper
      *thit = sol(0);
      geo = new LocalGeo(ray.evaluate(*thit), normal);
      return true;
    }
    thit = NULL;
    geo = NULL;
    return false;
  }
};

class Sphere : Shape {
public:
  Vector3f center;
  float radius;
  bool intersect(Ray& ray, float* thit, LocalGeo* geo) {
    float a, b, c;
    a = ray.dir(0)*ray.dir(0) + ray.dir(1)*ray.dir(1) + ray.dir(2)*ray.dir(2);
    b = 2 * (ray.pos(0) - center(0) + ray.pos(1) - center(1) + ray.pos(2) - center(2));
    c = (ray.pos(0) - center(0)) * (ray.pos(0) - center(0)) + (ray.pos(1) - center(1)) * (ray.pos(1) - center(1)) + (ray.pos(2) - center(2)) * (ray.pos(2) - center(2));
    float t1, t2;
    t1 = (-b - sqrtf(b*b - 4 * a * c)) / (2 * a);
    t2 = (-b + sqrtf(b*b - 4 * a * c)) / (2 * a);
    if (isnan(t1)) {
      thit = NULL;
      geo = NULL;
      return false;
    }
    float trueHit = 0;
    bool goodHit = false;
    if (ray.t_min <= t1 && t1 <= ray.t_max) {
      trueHit = t1;
    } else if (ray.t_min <= t2 && t2 <= ray.t_max) {
      trueHit = t2;
    }
    if (goodHit) {
      *thit = trueHit;
      Vector3f pos = ray.evaluate(*thit);
      Vector3f normal;
      normal << pos(0) - center(0), pos(1) - center(1), pos(2) - center(2);
      normal.normalize();
      geo = new LocalGeo(pos, normal);
      return true;
    }
    thit = NULL;
    geo = NULL;
    return false;
  }
};
