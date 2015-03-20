#include <float.h>
#include "Class.h"
#include <iostream>
#include <cstdio>

inline bool inRange(float x, float min, float max) {
  return min <= x && x <= max;
}
inline float degToRad(float r) {
  return 2 * 3.14159265 / 360 * r;
}

Ray::Ray() {}
Ray::Ray(Vector3f p, Vector3f d, float t_min, float t_max) {
  pos = p;
  dir = d;
  this->t_min = t_min;
  this->t_max = t_max;
}
Vector3f Ray::evaluate(float t) {
  Vector3f ret;
  ret << pos(0) + t * dir(0), pos(1) + t * dir(1), pos(2) + t * dir(2);
  return ret;
}

LocalGeo::LocalGeo() {}
LocalGeo::LocalGeo(Vector3f p , Vector3f n) {
  pos = p;
  normal = n.normalized();
}

Matrix4f MatrixUtils::createTranslationMatrix(float tx, float ty, float tz) {
  Matrix4f ret;
  ret <<
    1, 0, 0, tx,
    0, 1, 0, ty,
    0, 0, 1, tz,
    0, 0, 0, 1;
  return ret;
}
 Matrix4f MatrixUtils::createScalingMatrix(float sx, float sy, float sz) {
  Matrix4f ret;
  ret <<
    sx, 0, 0, 0,
    0, sy, 0, 0,
    0, 0, sz, 0,
    0, 0, 0, 1;
  return ret;
}
 Matrix4f MatrixUtils::createRotationMatrix(float rx, float ry, float rz) {
  rx = degToRad(rx);
  ry = degToRad(ry);
  rz = degToRad(rz);
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

Transformation::Transformation() {
  this->m = this->minv = this->minvt = this->mt = (Matrix4f() << 1, 0, 0, 0,
						   0, 1, 0, 0,
						   0, 0, 1, 0,
						   0, 0, 0, 1).finished();
}
Transformation::Transformation(Matrix4f m) {
  this->m = m;
  this->minv = m.inverse();
  this->minvt = minv.transpose();
  this->mt = m.transpose();
}
Vector3f Transformation::doTransformation(Vector3f vec, Matrix4f mat, float ext) {
  Vector4f temp;
  temp << vec(0), vec(1), vec(2), ext;
  temp = mat * temp;
  Vector3f ret;
  ret << temp(0), temp(1), temp(2);
  return ret;
}
Vector3f Transformation::transformPoint(Vector3f vec) {
  return doTransformation(vec, m, 1);
}
Vector3f Transformation::transformNormal(Vector3f vec) {
  return doTransformation(vec, minvt, 0).normalized();
}
Ray Transformation::Transformation::transformRay(Ray ray) {
  Ray ret(transformPoint(ray.pos), transformPoint(ray.dir), ray.t_min, ray.t_max);
  return ret;
}
// Rather than transforming the shapes themselves, transform the rays into the shapes' local coords
Ray Transformation::transformRayToLocalCoords(Ray ray) {
  return Ray(doTransformation(ray.pos, minv, 1), doTransformation(ray.dir, mt, 0), ray.t_min, ray.t_max);
}
LocalGeo Transformation::transformLocalGeo(LocalGeo geo) {
  LocalGeo ret(transformPoint(geo.pos), transformNormal(geo.normal));
  return ret;
}
Transformation Transformation::chainTransformation(Matrix4f t) {
  return Transformation(m * t);
}

Color::Color() {
  r = g = b = 0;
}
Color::Color(float r, float g, float b) {
  this->r = r;
  this->g = g;
  this->b = b;
}
Color Color::add(Color o) {return Color(r + o.r, g + o.g, b + o.b); }
Color Color::sub(Color o) { return Color(r - o.r, g - o.g, b - o.b); }
Color Color::scale(float s) { return Color(r * s, g * s, b * s); }
Color Color::mul(Color o) { return Color(r * o.r, g * o.g, b * o.b); }

BRDF::BRDF() {
  kd = ks = ka = kr = Color();
  specExp = 1;
}
BRDF::BRDF(Color kd, Color ks, Color ka, Color kr, float ns) {
  this->kd = kd;
  this->ks = ks;
  this->ka = ka;
  this->kr = kr;
  specExp = ns;
}

int Shape::curID = 0;
Shape::Shape() {
  transform = Transformation();
  brdf = BRDF();
  id = curID;
  curID++;
}
Shape::Shape(Transformation t, BRDF brdf) {
  transform = t;
  this->brdf = brdf;
  id = curID;
  curID++;
}
bool Shape::intersectP(Ray& ray) {
  float* thit;
  LocalGeo* geo;
  return this->intersect(ray, thit, geo);
}

// Constructs a triangle with the default vertex normals (p2 - p1) x (p3 - p1) i.e. flat triangle
Triangle::Triangle(Vector3f p1, Vector3f p2, Vector3f p3, Transformation t, BRDF brdf) : Shape(t, brdf) {
  this->p1 = p1;
  this->p2 = p2;
  this->p3 = p3;
  n1 = n2 = n3 = (p2 - p1).cross(p3 - p1);
  this->n1.normalize();
  this->n2.normalize();
  this->n3.normalize();
}
// Constructs a triangle with the given vertex normals
Triangle::Triangle(Vector3f p1, Vector3f p2, Vector3f p3, Vector3f n1, Vector3f n2, Vector3f n3, Transformation t, BRDF brdf) : Shape(t, brdf) {
  this->p1 = p1;
  this->p2 = p2;
  this->p3 = p3;
  this->n1 = n1.normalized();
  this->n2 = n2.normalized();
  this->n3 = n3.normalized();
}
bool Triangle::intersect(Ray& world_ray, float* thit, LocalGeo* geo) {
  // TODO Test ray transformation
  Ray ray = transform.transformRayToLocalCoords(world_ray);
  Matrix3f mat;
  mat << -ray.dir, p2 - p1, p3 - p1;
  Vector3f sol = mat.householderQr().solve(ray.pos - p1);
  if (inRange(sol(0), ray.t_min, ray.t_max) && inRange(sol(1), 0, 1) && inRange(sol(2), 0, 1) && sol(1) + sol(2) <= 1) {
    // If this intersection occurs within the ray's lifespan, and is in the triangle proper
    *thit = sol(0);
    //*geo = LocalGeo(ray.evaluate(*thit), sol(1) * n2 + sol(2) * n3 + (1 - sol(1) - sol(2)) * n1);
    Vector3f pos = ray.evaluate(*thit);
    Vector3f normal = sol(1) * n2 + sol(2) * n3 + (1 - sol(1) - sol(2)) * n1;
    *geo = LocalGeo(transform.transformPoint(pos), transform.transformNormal(normal));
    return true;
  }
  return false;
}

Sphere::Sphere(Vector3f c, float r, Transformation t, BRDF brdf) : Shape(t, brdf) {
  center = c;
  radius = r;
}
bool Sphere::intersect(Ray& world_ray, float* thit, LocalGeo* geo) {
  // TODO Test ray transformation
  Ray ray = transform.transformRayToLocalCoords(world_ray);
  float a, b, c;
  a = ray.dir.dot(ray.dir);
  b = 2 * ray.dir.dot(ray.pos - center);
  c = (ray.pos - center).dot(ray.pos - center);
  c -= radius * radius;
  float t1, t2;
  t1 = (-b - sqrtf(b*b - 4 * a * c)) / (2 * a);
  t2 = (-b + sqrtf(b*b - 4 * a * c)) / (2 * a);
  if (isnan(t1)) {
    return false;
  }
  float trueHit = 0;
  bool goodHit = false;
  if (inRange(t1, ray.t_min, ray.t_max)) {
    trueHit = t1;
    goodHit = true;
  } else if (inRange(t2, ray.t_min, ray.t_max)) {
    trueHit = t2;
    goodHit = true;
  }
  if (goodHit) {
    *thit = trueHit;
    Vector3f pos = ray.evaluate(*thit);
    Vector3f normal;
    normal << pos(0) - center(0), pos(1) - center(1), pos(2) - center(2);
    normal.normalize();
    *geo = LocalGeo(transform.transformPoint(pos), transform.transformNormal(normal));
    return true;
  }
  return false;
}

PointLight::PointLight(Vector3f p, Color c, int f) {
  point = p;
  color = c;
  falloff = f;
}
void PointLight::generateLightRay(LocalGeo& local, Ray* ray, Color* color) {
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

DirectionalLight::DirectionalLight(Vector3f d, Color c) {
  direction = d;
  color = c;
}
void DirectionalLight::generateLightRay(LocalGeo& local, Ray* ray, Color* color) {
  *ray = Ray(local.pos, -direction, 0, FLT_MAX);
  *color = this->color;
}

Raytracer::Raytracer(vector<Shape*>* s, vector<Light*>* l, Color al) {
  shapes = *s;
  lights = *l;
  ambient_lights = al;
}
// axis must be a normalized vector
Vector3f Raytracer::reflectVector(Vector3f vec, Vector3f axis) {
  return 2 * (axis * axis.dot(vec)) - vec;
}
Color Raytracer::shade(LocalGeo& geo, Ray& lightRay, Color lightColor, Vector3f viewer, BRDF brdf) {
  viewer = viewer - geo.pos;
  Color res = Color();

  // Diffuse shading
  float base = lightRay.dir.normalized().dot(geo.normal);
  if (base < 0) base = 0;
  res = res.add(lightColor.mul(brdf.kd).scale(base));

  // Specular shading
  Vector3f reflection = reflectVector(lightRay.dir, geo.normal);
  base = reflection.normalized().dot(viewer.normalized());
  if (base < 0)  base = 0;
  base = pow(base, brdf.specExp);
  res = res.add(lightColor.mul(brdf.ks).scale(base));

  return res;
}
bool Raytracer::firstObjectHit(Ray& ray, Shape* ignore, Shape* shape, float* thit, LocalGeo* geo) {
  Shape first_hit;
  float first_hit_t = FLT_MAX;
  LocalGeo first_hit_geo;
  bool had_hit = false;
  for (unsigned int i = 0; i < shapes.size(); i++) {
    Shape* shape = shapes[i];
    if (ignore == NULL || shape->id != ignore->id) {
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
  }
  if (had_hit) {
    *shape = first_hit;
    *thit = first_hit_t;
    *geo = first_hit_geo;
    return true;
  }
  return false;
}
bool Raytracer::firstObjectHitP(Ray& ray, Shape* ignore) {
  Shape blahs;
  float blaht;
  LocalGeo blahg;
  return firstObjectHit(ray, ignore, &blahs, &blaht, &blahg);
}
Color Raytracer::trace(Ray& ray, unsigned int depth) { return trace(ray, depth, NULL); }
Color Raytracer::trace(Ray& ray, unsigned int depth, Shape* ignore) {
  if (depth == 0) return Color(0, 0, 0);
  Color ret = Color(0, 0, 0);
  // Find the first thing this ray hits
  Shape first_hit_shape;
  float first_hit_t;
  LocalGeo first_hit_geo;
  // If this ray never hits anything, return the color black
  if (!firstObjectHit(ray, ignore, &first_hit_shape, &first_hit_t, &first_hit_geo) || !inRange(first_hit_t, ray.t_min, ray.t_max)) {
    return Color(0, 0, 0);
  }
  // Shade this thing - for each light, if this point reaches the light, do shading
  for (unsigned int i = 0; i < lights.size(); i++) {
    Ray light_ray;
    Color light_color;
    Light* light = lights[i];
    light->generateLightRay(first_hit_geo, &light_ray, &light_color);
    // If there is no intervening object, let's do shading
    Shape blahs;
    float blaht;
    LocalGeo blahg;
    if (!firstObjectHitP(light_ray, &first_hit_shape/*, &blahs, &blaht, &blahg*/)) {
      ret = ret.add(shade(first_hit_geo, light_ray, light_color, ray.pos, first_hit_shape.brdf));
    }
  }
  ret = ret.add(first_hit_shape.brdf.ka.mul(ambient_lights));
  Vector3f reflectedVec = reflectVector(-ray.dir, first_hit_geo.normal);
  Ray reflectedRay = Ray(first_hit_geo.pos, reflectedVec, 0, FLT_MAX);
  Color reflected_color = trace(reflectedRay, depth - 1, &first_hit_shape);
  ret = ret.add(reflected_color.mul(first_hit_shape.brdf.kr));
  return ret;
}
