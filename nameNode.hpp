#include <thread>
#include <condition_variable>
#include <mutex>
#include "commandParser.hpp"

class NameNode{
public:
    static std::shared_ptr<NameNode> getNameNode(){
        if(nameNode_ == nullptr){
            nameNode_ = std::shared_ptr<NameNode>(new NameNode());
            // nameNode_ = std::make_shared<NameNode>(); // constructor is protected, this won't work.
        }
        return nameNode_;
    }

    void operator() () const {
        cmdParser_->work();
    }

protected:
    NameNode(){
        cmdParser_ = CommandParser::getCommandParser();
        std::cout << "NameNode started.\n";
    }

    NameNode(const NameNode&) = delete;

    NameNode& operator= (const NameNode&) = delete;

private:
    static std::shared_ptr<NameNode> nameNode_;
    // commandParser, dataNode list
    std::shared_ptr<CommandParser> cmdParser_;
};

std::shared_ptr<NameNode> NameNode::nameNode_ = nullptr;