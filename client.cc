#include "nameNode.hpp"

using namespace std;

bool startDFS(){
    shared_ptr<FileSys> fileSys = FileSys::getFileSys(); // give absolute path for MiniDFS.
    shared_ptr<NameNode> nameNode = NameNode::getNameNode();
    NameNode* pn = nameNode.get();
    if(fileSys == nullptr || nameNode == nullptr){
        cerr << "MiniDFS start faled.\n";
        return false;
    }else{
        thread nameThread(std::ref(*nameNode));
        nameThread.join();
    }
    return true;
}

int main(){
    startDFS();
    return 0;
}