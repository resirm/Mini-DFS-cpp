#include "nameNode.hpp"
#include "dataNode.hpp"
#include "commandParser.hpp"

using namespace std;

bool startDFS(){
    const size_t dataNodeNum = 4;
    shared_ptr<NameNode> nameNode = NameNode::getNameNode();
    shared_ptr<CommandParser> cmdParser = CommandParser::getCommandParser(nameNode);
    vector<shared_ptr<DataNode>> dataNodes;
    string d = "dataNode";
    for(size_t idx = 0; idx < dataNodeNum; ++idx){
        dataNodes.push_back(shared_ptr<DataNode>(new DataNode(d+to_string(idx), idx)));
    }
    if(nameNode == nullptr){
        cerr << "MiniDFS start faled.\n";
        return false;
    }else{
        thread nameThread(std::ref(*cmdParser));
        vector<thread> vth;
        for(size_t idx = 0; idx < dataNodeNum; ++idx){
            vth.push_back(thread(std::ref(*dataNodes[idx])));
            vth.back().detach();
        }
        nameThread.join();
    }
    return true;
}

int main(){
    startDFS();
    return 0;
}