#include <iostream>
#include <string>
#include "dataBlock.hpp"

using namespace std;

int main(){
    DataBlock db("/Users/yifengzhu/Code/Mini-DFS-cpp/README.md");
    cout << "read " << db.fromFile() << " bytes." << endl;
    db.toFile();
    cout << "file written." << endl;
    return 0;
}