#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <float.h>

#include <eigen3/Dense>
#include "Class.h"

using namespace std;
using namespace Eigen;

Transformation trans;
BRDF brdf(Color(1, 1, 1), Color(1, 1, 1), Color(1, 1, 1), Color(0, 0, 0), 64);
vector<Shape*> shapes;
vector<Light*> lights;
Color ambient = Color(); 
vector<Vector3f> camera;

enum string_code{
    cam, sph, tri, obj, ltp, ltd, lta, mat, xft, xfr, xfs, dud
};

string_code hash_it(string const& op){
    if (op == "cam") return cam;
    if (op == "sph") return sph;
    if (op == "tri") return tri;
    if (op == "obj") return obj;
    if (op == "ltp") return ltp;
    if (op == "ltd") return ltd;
    if (op == "lta") return lta;
    if (op == "mat") return mat;
    if (op == "xft") return xft;
    if (op == "xfr") return xfr;
    if (op == "xfs") return xfs;
    return dud; // literally a dud case, or unimplemented case
} 

vector<float> get_params(string line, int expected){
    // takes a string, vector, and expected number of args, fills vector with params
    // if there are more params than expected, ignores the extra params
    //     and issues warning to stderr
    vector<float> params(expected, 0.0);
    stringstream ss(line);
    string param;
    unsigned int i = 0;
    while (getline(ss, param, ' ') and i<expected){
        params[i] = stof(param);
        i++;
    }

    // ERROR: unexpected behavior
    if (getline(ss, param, ' ')){
        cout << param << endl;
        cerr << "Extra parameters, ignoring" << endl;
    }
    return params;
}

// TO IMPLEMENT: individual functions for handling each case
// each function should take a string listing the parameters
int case_cam(vector<float>* params){
    float ex=(*params)[0], ey=(*params)[1], ez=(*params)[2];
    float llx=(*params)[3], lly=(*params)[4], llz=(*params)[5];
    float lrx=(*params)[6], lry=(*params)[7], lrz=(*params)[8];
    float ulx=(*params)[9], uly=(*params)[10], ulz=(*params)[11];
    float urx=(*params)[12], ury=(*params)[13], urz=(*params)[14];
    Vector3f e, ll, lr, ul, ur;
    e << ex, ey, ez;
    ll << llx, lly, llz;
    lr << lrx, lry, lrz;
    ul << ulx, uly, ulz;
    ur << urx, ury, urz;
    camera.push_back(e);
    camera.push_back(ll);
    camera.push_back(lr);
    camera.push_back(ul);
    camera.push_back(ur);
    return 1;
}

int case_sph(vector<float>* params){
    float cx=(*params)[0], cy=(*params)[1], cz=(*params)[2], r=(*params)[3];
    Vector3f center;
    center << cx, cy, cz;
    Sphere* sph = new Sphere(center, r, trans, brdf);
    shapes.push_back(sph); 
    return 1;
}

int case_tri(vector<float>* params){
    float ax=(*params)[0], ay=(*params)[1], az=(*params)[2];
    float bx=(*params)[3], by=(*params)[4], bz=(*params)[5];
    float cx=(*params)[6], cy=(*params)[7], cz=(*params)[8];
    Vector3f a, b, c;
    a << ax, ay, az;
    b << bx, by, bz;
    c << cx, cy, cz;
    Triangle* tri = new Triangle(a, b, c, trans, brdf);
    shapes.push_back(tri);
    return 1;
} 

int case_obj(string file_name){
    // takes a file_name. Returns positive if successful, negative if not
    file_name = file_name.substr(1, file_name.length()-2); //remove quotes
    fstream input_file (file_name.c_str());
    if (input_file.is_open()){
        string line;
        vector<Vector3f> vectors;  // Only handles vectors of length 3!
        vector<Vector3f> faces;    // Only handles triangle polygons!

        while (getline(input_file, line)){
            istringstream iss(line);
            char defn;
            float args[3];
            iss >> defn >> args[0] >> args[1] >> args[2];

            if (line[0] == '#'){
            }
            else if (defn == 'v'){
                Vector3f vertex;
                vertex << args[0], args[1], args[2];
                vectors.push_back(vertex);
            }
            else if (defn == 'f'){
                Vector3f face;
                face << args[0], args[1], args[2];
                faces.push_back(face);
            }
        }
        input_file.close();

        // for each face, create a triangle
        for(vector<Vector3f>::iterator it=faces.begin(); it!=faces.end();++it){
            Vector3f face = *it;
            int i1=face(0), i2=face(1), i3=face(2);
            Vector3f v1=vectors[i1-1], v2=vectors[i2-1], v3=vectors[i3-1];
            Triangle* tri = new Triangle(v1, v2, v3, trans, brdf);
            shapes.push_back(tri);
        }
        return 1;
    }
    else cerr << "Unable to open file" << endl;
    return -1;
}

int case_ltp(vector<float>* params){
    float px=(*params)[0], py=(*params)[1], pz=(*params)[2];
    float r=(*params)[3], g=(*params)[4], b=(*params)[5];
    // possible error, what if no falloff?
    int falloff = int((*params)[6]);
    Vector3f p;
    p << px, py, pz;
    Color c(r, g, b);
    PointLight* pl = new PointLight(p, c, falloff);
    lights.push_back(pl);
    return 1;
}

int case_lta(vector<float>* params){
    float r=(*params)[0], g=(*params)[1], b=(*params)[2];
    Color c(r,g,b);
    ambient = ambient.add(c);
    return 1;
}

int case_ltd(vector<float>* params){
    float dx=(*params)[0], dy=(*params)[1], dz=(*params)[2];
    float r=(*params)[3], g=(*params)[4], b=(*params)[5];
    Vector3f d;
    d << dx, dy, dz;
    Color c(r,g,b);
    DirectionalLight* dl = new DirectionalLight(d, c);
    lights.push_back(dl);
    return 1;
}

int case_mat(vector<float>* params){
    float kar=(*params)[0], kag=(*params)[1], kab=(*params)[2];
    float kdr=(*params)[3], kdg=(*params)[4], kdb=(*params)[5];
    float ksr=(*params)[6], ksg=(*params)[7], ksb=(*params)[8];
    float ksp=(*params)[9];
    float krr=(*params)[10], krg=(*params)[11], krb=(*params)[12];
    // d, s, a, r
    Color ka(kar,kag,kab), kd(kdr,kdg,kdb), ks(ksr,ksg,ksb), kr(krr,krg,krb);
    // possible error, ensure that I am actual replacing brdf
    brdf = BRDF(kd, ks, ka, kr, ksp);
    return 1;
}

int case_xft(vector<float>* params){
    float tx=(*params)[0], ty=(*params)[1], tz=(*params)[2];
    Matrix4f tm = MatrixUtils::createTranslationMatrix(tx, ty, tz);
    trans = trans.chainTransformation(tm);
    return 1;
}

int case_xfr(vector<float>* params){
    float rx=(*params)[0], ry=(*params)[1], rz=(*params)[2];
    Matrix4f rm = MatrixUtils::createTranslationMatrix(rx, ry, rz);
    trans = trans.chainTransformation(rm);
    return 1;
}

int case_xfs(vector<float>* params){
    float sx=(*params)[0], sy=(*params)[1], sz=(*params)[2];
    Matrix4f sm = MatrixUtils::createTranslationMatrix(sx, sy, sz);
    trans = trans.chainTransformation(sm);
    return 1;
}
    
int handle_cases(string line){
    // Handles each line in the input file
    // if succesfully handled, returns positive int. Else negative int
    // returns 0 if unimplemented
    if (line.length() < 3){
        return -1;
    }
    string op = line.substr(0,3);
    string args = line.substr(4);
    vector<float> params;

    // switch cannot take string, so hash it to enum
    switch (hash_it(op)) {
        case cam:
            params = get_params(args, 15);
            return case_cam(&params);
        case sph:
            params = get_params(args, 4);
            return case_sph(&params);
        case tri:
            params = get_params(args, 9);
            return case_tri(&params);
        case obj:
            return case_obj(args);
        case ltp:
            params = get_params(args,7);
            return case_ltp(&params);
        case lta:
            params = get_params(args,3);
            return case_lta(&params);
        case ltd:
            params = get_params(args,6);
            return case_ltd(&params);;
        case mat:
            params = get_params(args,13);
            return case_mat(&params);
        case xft:
            params = get_params(args,3);
            return case_xft(&params);
        case xfr:
            params = get_params(args,3);
            return case_xfr(&params);
        case xfs:
            params = get_params(args,3);
            return case_xfs(&params);
        default:
            return -1;
    }
}

void writePPM(int dimx, int dimy, Color* pixels, const char* filename) {
    int i, j;
    FILE *fp = fopen(filename, "wb"); /* b - binary mode */
    (void) fprintf(fp, "P6\n%d %d\n255\n", dimx, dimy);
    for (j = 0; j < dimy; ++j){
        for (i = 0; i < dimx; ++i){
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


int main(){
    string file_name;
    cout << "Specify input file: ";
    cin >> file_name;

    fstream input_file (file_name.c_str());
    if (input_file.is_open()){
        cout << "File opened successfully!" << endl;
        string line;        
        while (getline(input_file, line)){
            int status = handle_cases(line);
            if (status == 0){
                cout << "Unimplemented case!\n     " << line << endl;
            }
            else if (status < 0) {
                cerr << "Warning: Unexpected feature, ignoring line\n    "
                << line  << endl;
            }
        }
        input_file.close();
    }
    else cout << "Unable to open file" << endl;

    int width=400, height=400;
    Vector3f cam=camera[0], ll=camera[1], lr=camera[2], ul=camera[3], ur=camera[4];
    
    Color pixels[width * height];
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
    string output_file = "../image/"+file_name+".ppm";
    writePPM(width, height, pixels, output_file.c_str());
    return 0;
}
