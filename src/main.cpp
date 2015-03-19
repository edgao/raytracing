#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <eigen3/Dense>
#define cimg_use_jpeg
#include "CImg.h"
#include "Class.h"

using namespace std;
using namespace Eigen;
using namespace cimg_library;

Transformation trans;
BRDF brdf;
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

void get_params(string line, vector<float> params, int expected){
    // takes a string, vector, and expected number of args, fills vector with params
    // if there are more params than expected, ignores the extra params
    //     and issues warning to stderr
    params.reserve(expected);
    stringstream ss(line);
    string param;
    unsigned int i = 0;
    while (getline(ss, param, ' ') and i<expected){
        i++;
    }

    // ERROR: unexpected behavior
    if (getline(ss, param, ' ')){
        cout << param << endl;
        cerr << "Extra parameters, ignoring" << endl;
    }
}

// TO IMPLEMENT: individual functions for handling each case
// each function should take a string listing the parameters
int case_sph(vector<float> params){
    float cx=params[0], cy=params[1], cz=params[2], r=params[3];
    Vector3f center;
    center << cx, cy, cz;
    Sphere sph = Sphere(center, r, trans, brdf);
    return 0;
}

int case_tri(vector<float> params){
    float ax=params[0], ay=params[1], az=params[2];
    float bx=params[3], by=params[4], bz=params[5];
    float cx=params[6], cy=params[7], cz=params[8];
    Vector3f a, b, c;
    a << ax, ay, az;
    b << bx, by, bz;
    c << cx, cy, cz;
    return 0;
} 

int case_obj(string file_name){
    // takes a file_name. Returns positive if successful, negative if not
    file_name = file_name.substr(1, file_name.length()-2); //remove quotes
    fstream input_file (file_name.c_str());
    if (input_file.is_open()){
        string line;

        while (getline(input_file, line)){
            istringstream iss(line);
            char defn;
            float args[3];
            iss >> defn >> args[0] >> args[1] >> args[2];

            // cannot handle comments!
            if (line[0] == '#'){
            }
            else if (defn == 'v' or defn == 'f'){
                Vector3f vertex;
                vertex << args[0], args[1], args[2];
            }
        }
        input_file.close();
        return 0;
    }
    else cerr << "Unable to open file" << endl;
    return -1;
}

int case_ltp(vector<float> params){
    float px=params[0], py=params[1], pz=params[2];
    float r=params[3], g=params[4], b=params[5], falloff = params[6];
    // Double check vector initializes to 0
    return 0;
}

int case_ltd(vector<float> params){
    float dx=params[0], dy=params[1], dz=params[2];
    float r=params[3], g=params[4], b=params[5];
    return 0;
}

int case_mat(vector<float> params){
    float kar=params[0], kag=params[1], kab=params[2];
    float kdr=params[3], kdg=params[4], kdb=params[5];
    float ksr=params[6], ksg=params[7], ksb=params[8], ksp=params[9];
    float krr=params[10], krg=params[11], krb=params[12];
    return 0;
}

int case_xft(vector<float> params){
    float tx=params[0], ty=params[1], tz=params[2];
    return 0;
}

int case_xfr(vector<float> params){
    float rx=params[0], ry=params[1], rz=params[2];
    return 0;
}

int case_xfs(vector<float> params){
    float sx=params[0], sy=params[1], rz=params[2];
    return 0;
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
            get_params(args, params, 15);
            return 0;
        case sph:
            get_params(args, params, 4);
            case_sph(params);
            return 0;
        case tri:
            get_params(args, params, 9);
            return 0;
        case obj:
            return case_obj(args);
        case ltp:
            get_params(args, params, 7);
            return 0;
        case ltd:
            get_params(args, params, 6);
            return 0;
        case lta:
            get_params(args, params, 3);
            return 0;
        case mat:
            get_params(args, params, 13);
            return 0;
        case xft:
            get_params(args, params, 3);
            return 0;
        case xfr:
            get_params(args, params, 3);
            return 0;
        case xfs:
            get_params(args, params, 3);
            return 0;
        default:
            return -1;
    }
    return 0;
}

int main()
{
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
    return 0;
}
