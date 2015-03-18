class Ray {
    public: 
        Vector3f pos;
        Vector3f dir;
        float t_min, t_max;
        void Ray();
        Vector3f evaluate(float t);
};

class Sphere : public Shape {
    public:
        
