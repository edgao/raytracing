#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main() {
    fstream myfile;
    myfile.open("example.txt");
    myfile << "Writing to this file.\n";
    myfile.close();
    
    myfile.open("example.txt", ios::app);
    if (myfile.is_open()){
        myfile << "This is a line.\n";
        myfile.close();
    }
    else cout << "Unable to open file";
    
    myfile.open("example.txt");
    string line;
    if (myfile.is_open()){
        while (getline(myfile, line)){
            cout << line;
        }
        myfile.close();
    }
    return 0;
}
