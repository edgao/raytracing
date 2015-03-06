class Vector {
public:
  float x, y, z;
  Vector(float x, float y, float z) {
    this.x = x;
    this.y = y;
    this.z = z;
  }
  Vector add(Vector b) { return new Vector(x + b.x, y + b.y, z + b.z); }
  Vector sub(Vector b) { return new Vector(x - b.x, y - b.y, z - b.z); }
  Vector scale(float s) { return new Vector(x * s, y * s, z * s); }
  Vector normalize() { return scale(1 / magnitude()); }
  float magnitude() { return sqrtf(x * x + y * y + z * z); }
  // Returns a Vector pointing from a to b
  static pointToPoint(Point a, Point b) { return new Vector(b.x - a.x, b.y - a.y, b.z - a.z); }
}

class Normal : public Vector {
public:
  x, y, z;
  Normal(float x, float y, float z) {
    this.x = x;
    this.y = y;
    this.z = z;
    normalizeThis();
  }
  Normal add(Vector b) { return new Normal(x + b.x, y + b.y, z + b.z); }
  Normal sub(Vector b) { return new Normal(x - b.x, y - b.y, z - b.z); }  
private:
  void normalizeThis() {
    float mag = magnitude();
    // because floats are dumb
    if (mag <= 0.00001) return;
    x /= mag;
    y /= mag;
    z /= mag;
  }
}

class Point {
public:
  float x, y, z;
  Point(float x, float y, float z) {
    this.x = x;
    this.y = y;
    this.z = z;
  }
  Point add(Vector v) { return new Point(x + v.x, y + v.y, z + v.z); }
  Point sub(Vector v) { return new Point(x - v.x, y - v.y, z - v.z); }
}

class Ray {
public:
  Point pos;
  Vector dir;
  float t_min, t_max;
  Ray(Point p, Vector d, float t_min, float t_max) {
    pos = p;
    dir = d;
    this.t_min = t_min;
    this.t_max = t_max;
  }
}

class Matrix {
public:
  float mat[4][4];
  Matrix multiply(Matrix o) {
    Matrix ret = new Matrix();
    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 4; c++) {
	float sum = 0F;
	for (int i = 0; i < 4; i++) {
	  sum += mat[r + i] * o.mat[c + i];
	}
	ret.mat[r][c] = sum;
      }
    }
  }
  Matrix invert() {
    // TODO Implement matrix inversion
    return NULL;
  }
  static createRotationMatrix(Vector axis, float radians) {
    // TODO Implement exponential map rotation
    return NULL;
  }
  static createTranslationMatrix(float tx, float ty, float tz) {
    Matrix ret = new Matrix();
    ret.mat = {{1, 0, 0, tx},
	       {0, 1, 0, ty},
	       {0, 0, 1, tz},
	       {0, 0, 0, 1}};
  }
  static createScalingMatrix(float sx, float sy, float sz) {
    Matrix ret = new Matrix();
    ret.mat = {{sx, 0, 0, 0},
	       {0, sy, 0, 0},
	       {0, 0, sz, 0},
	       {0, 0, 0, 1}};
  }
}

class Transformation {
public:
  Matrix m, minv;
  Vector transform(Vector v) {
    float x, y, z;
    x = v.x * m.mat[0][0] * v.y * mat[0][1] + v.z * mat[0][2] + mat[0][3];
    y = v.x * m.mat[1][0] * v.y * mat[1][1] + v.z * mat[1][2] + mat[1][3];
    z = v.x * m.mat[2][0] * v.y * mat[2][1] + v.z * mat[2][2] + mat[2][3];
    float bot = v.x * m.mat[3][0] * v.y * mat[3][1] + v.z * mat[3][2] + mat[3][3];
    x /= bot;
    y /= bot;
    z /= bot;
    return new Vector(x, y, z);
  }
  Point transform(Point v) {
    float x, y, z;
    x = v.x * m.mat[0][0] * v.y * mat[0][1] + v.z * mat[0][2] + mat[0][3];
    y = v.x * m.mat[1][0] * v.y * mat[1][1] + v.z * mat[1][2] + mat[1][3];
    z = v.x * m.mat[2][0] * v.y * mat[2][1] + v.z * mat[2][2] + mat[2][3];
    float bot = v.x * m.mat[3][0] * v.y * mat[3][1] + v.z * mat[3][2] + mat[3][3];
    x /= bot;
    y /= bot;
    z /= bot;
    return new Point(x, y, z);
  }
  Normal transform(Normal v) {
    float x, y, z;
    x = v.x * m.mat[0][0] * v.y * mat[0][1] + v.z * mat[0][2];
    y = v.x * m.mat[1][0] * v.y * mat[1][1] + v.z * mat[1][2];
    z = v.x * m.mat[2][0] * v.y * mat[2][1] + v.z * mat[2][2];
    float bot = v.x * m.mat[3][0] * v.y * mat[3][1] + v.z * mat[3][2];
    x /= bot;
    y /= bot;
    z /= bot;
    return new Normal(x, y, z);
  }
  Ray transform(Ray r) {
    return new Ray(transform(r.pos), transform(r.dir));
  }
  LocalGeo transform(LocalGeo l) {
    return new LocalGeo(transform(l.pos), transform(l.normal));
  }
}

class Color {
public:
  float r, g, b;
  Color(float r, float g, float b) {
    this.r = r;
    this.g = g;
    this.b = b;
  }
  Color add(Color o) { return new Color(r + o.r, g + o.g, b + o.b); }
  Color sub(Color o) { return new Color(r - o.r, g - o.g, b - o.b); }
  Color scale(float s) { return new Color(r * s, g * s, b * s); }
}

class BRDF {
public:
  Color kd, ks, ka, kr;
  BRDF(Color kd, Color ks, Color ka, Color kr) {
    this.kd = kd;
    this.ks = ks;
    this.ka = ka;
    this.kr = kr;
  }
}

class Sample {
public:
  float x, y;
  Sample(float x, float y) {
    this.x = x;
    this.y = y;
  }
}

class LocalGeo {
public:
  Point pos;
  Normal normal;
  LocalGeo(Point p, Normal n) {
    pos = p;
    normal = n;
  }
}

class Shape {
// TODO Implement Triangle and Shape, probably as subclasses of Shape?
public:
  bool intersect(Ray& ray, float* thit, LocalGeo* local) {
    // TODO Check intersection for triangle and sphere
    return false;
  }
  bool intersect(Ray& ray) {
    // TODO blah
    return false;
  }
}

//class Primitive {
//public:
//  bool intersect(Ray& ray, float* thit, Intersection* 
//}
