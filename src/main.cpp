#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

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

/*
int* get_params(string line, int params[], int expected){
    // takes a string and expected number of params, returns an array of ints
    // if there are more params than expected, ignores the extra params
    //     and issues warning to stderr
    // EX: "0 4 -1 3", 4 => [0, 4, -1, 3]
    // EX: "1 2 3 4 5", 3 => [1, 2, 3]
    params[4];
    
    stringstream ss(line);
    string param;
    int i = 0;
    while (getline(ss, param, ' ')){
        params[i] = stoi(param);
        cout << param << ": " << params[i];
        i++;
    }
    return params;
}
*/

// TO IMPLEMENT: individual functions for handling each case
// each function should take a string listing the parameters

int case_obj(string file_name){
    // handles the obj case.
    // takes a file_name. Returns positive if successful, negative if not
    file_name = file_name.substr(1, file_name.length()-2); //remove quotes
    cout << file_name << endl;
    fstream input_file (file_name.c_str());
    if (input_file.is_open()){
        string line;
        /*
        while (getline(input_file, line)){

        }
        */
        input_file.close();
        return 0;
    }
    else cerr << "Unable to open file" << endl;
    return -1;
}

int handle_cases(string line){
    // Handles each line in the input file
    // if succesfully handled, returns positive int. Else negative int
    // returns 0 if unimplemented
    if (line.length() < 3){
        return -1;
    }
    string op = line.substr(0,3);
    string params = line.substr(4);

    // switch cannot take string, so hash it to enum
    switch (hash_it(op)) {
        case cam:
            return 0;
        case sph:
            return 0;
        case tri:
            return 0;
        case obj:
            return case_obj(params);
        case ltp:
            return 0;
        case ltd:
            return 0;
        case lta:
            return 0;
        case mat:
            return 0;
        case xft:
            return 0;
        case xfr:
            return 0;
        case xfs:
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
    cout << "The value you entered is " << file_name << endl;

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
