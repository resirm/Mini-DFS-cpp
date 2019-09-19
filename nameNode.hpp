#ifndef NAMENODE_HPP
#define NAMENODE_HPP


// #include "commandParser.hpp"
#include "globalVariables.hpp"
#include "fileSys.hpp"

class NameNode: public FileSys, std::enable_shared_from_this<NameNode>{
public:
    static std::shared_ptr<NameNode> getNameNode(){
        if(nameNode_ == nullptr){
            nameNode_ = std::shared_ptr<NameNode>(new NameNode());
            // nameNode_ = std::make_shared<NameNode>(); // constructor is protected, this won't work.
        }
        return nameNode_;
    }

    // void operator() () const {
    //     cmdParser_ = CommandParser::getCommandParser(shared_from_this());
    //     cmdParser_->work();
    // }

protected:
    NameNode(string name = "nameNode", string rootPath = "/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/"):
            FileSys(name, rootPath), name_(name){
        fileSysInfo_ = FileSysInfo::getFileSysInfo("/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/nameNode/");
        // cmdParser_ = nullptr;
        globalVariables_ = GlobalVariables::getGlobalVariables();
        std::cout << "NameNode started.\n";
    }

    NameNode(const NameNode&) = delete;

    NameNode& operator= (const NameNode&) = delete;

private:
    std::string name_;
    static std::shared_ptr<NameNode> nameNode_;
    // commandParser, dataNode list
    // std::shared_ptr<CommandParser> cmdParser_;
    std::shared_ptr<GlobalVariables> globalVariables_;
    std::shared_ptr<FileSysInfo> fileSysInfo_;
};

std::shared_ptr<NameNode> NameNode::nameNode_ = nullptr;

#endif // !NAMENODE_HPP