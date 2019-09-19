#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "fileSys.hpp"
#include "globalVariables.hpp"
#include "nameNode.hpp"

using std::string; using std::vector; using std::map;
using std::cin; using std::cout; using std::cerr; using std::endl;

class CommandParser{
public:
    using func = std::function<void(void)>;
    static std::shared_ptr<CommandParser> getCommandParser(std::shared_ptr<NameNode>& nameNode){
        if(commandParser_ == nullptr){
            commandParser_ = std::shared_ptr<CommandParser>(new CommandParser(nameNode));
            init();
        }
        return commandParser_;
    }

    void operator() (){
        work();
    }

    static void init(){
        commandParser_->cmdHandler_["mkdir"] = mkdir;
        commandParser_->cmdHandler_["touch"] = touch;
        commandParser_->cmdHandler_["rm"] = rm;
        commandParser_->cmdHandler_["rmdir"] = rmdir;
        commandParser_->cmdHandler_["ls"] = ls;
        commandParser_->cmdHandler_["cd"] = cd;
        commandParser_->cmdHandler_["pwd"] = pwd;
        commandParser_->cmdHandler_["put"] = []{};
        commandParser_->cmdHandler_["quit"] = quit;
    }
    void work(){
        string cmd;
        while(run_){
            cerr << ">>  ";
            if(std::getline(cin, cmd)){
                auto rst = split(cmd);
                if(rst.size() <= 1 && rst[0] != "ls" && rst[0] != "pwd" && rst[0] != "quit"){
                    cerr << "Usage: \n>>  ";
                    continue;
                }
                parameters_ = rst;
                if(cmdHandler_.find(rst[0]) == cmdHandler_.end()){
                    cerr << "Usage: \n>>  ";
                    continue;
                }
                if(rst[0] != "put" && rst[0] != "rm"){
                    cmdHandler_[rst[0]]();
                }else{
                    assign_put();
                }
            }
        }
    }

    void stop(){
        run_ = false;
    }

    ~CommandParser(){};

private:
    vector<string> split(string str, char sep=' '){
        // cout << str << endl;
        vector<string> rst;
        string s = "";
        for(auto c : str){
            if(c == sep){
                rst.push_back(s);
                // cout << s << endl;
                s = "";
            }else{
                s += c;
            }
            // cout << c << endl;
        }
        rst.push_back(s);
        // cout << s << endl;
        // cout << rst.size() << endl;
        return rst;
    };

    static void mkdir(){
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->nameNode_->mkdir(*idx);
        }
    };

    static void touch(){
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->nameNode_->touch(*idx);
        }
    };

    static void rm(){
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->nameNode_->rm(*idx);
        }
    };

    static void rmdir(){
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->nameNode_->rmdir(*idx);
        }
    };

    static void ls(){
        if(commandParser_->parameters_.size() == 1){
            commandParser_->nameNode_->ls();
            return;
        }
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->nameNode_->ls(*idx);
        }
    };
    
    static void cd(){
        commandParser_->nameNode_->cd(commandParser_->parameters_.at(1));
    };

    static void pwd(){
        commandParser_->nameNode_->pwd();
    };

    // static void put(){
    //     commandParser_->nameNode_->put(commandParser_->parameters_.at(1), commandParser_->parameters_.at(2));
    // };

    static void quit(){
        commandParser_->nameNode_->getMeta()->saveMeta();
        commandParser_->stop();
    };

private:
    void assign_put(){
        {   
            // std::string fpath = parameters_.at(1);
            // size_t len = 0, blks = 0, fid;
            // ifstream finput(fpath, std::ios::app);
            // if(finput.is_open()){
            //     len = finput.tellg();
            //     std::cout << "file length: " << len << endl;
            //     blks = std::ceil(double(len) / double(blkLen_));
            //     fileIdx_ = fid;
            //     finput.close();
            // }
            std::unique_lock<std::mutex> ul(globalVariables_->getMutex());
            size_t fid = nameNode_->fileCount();
            for(auto idx = 0; idx < 4; ++idx){
                std::cout << "DataNode working.\n";
                
                globalVariables_->getCV()[idx].wait(ul, [&, this]()->bool{ return globalVariables_->getFlag()[idx]; });
                std::cout << "DataNode awake!\n";
                globalVariables_->getFlag()[idx] = false;
                globalVariables_->getCmd()[idx] = parameters_[0];
                globalVariables_->getPara()[idx] = vector<string>(parameters_.begin()+1,parameters_.end());
                if(parameters_.size() <= 3){
                    globalVariables_->getPara()[idx].push_back(std::to_string(fid));
                }else{
                    globalVariables_->getPara()[idx][2] = std::to_string(fid);
                }
                globalVariables_->getFlag()[idx] = true;
                globalVariables_->getCV()[idx].notify_all();
            }
            // bool state = true;
            // for(auto idx = 0; idx < 4; ++idx){
            //     state = state && globalVariables_->getState()[idx];
            // }
            // // if(state){
            // //     nameNode_->getMeta()->getIdx2blks()[fileIdx_] = std::make_pair(blks_, len_);
            // //     nameNode_->getMeta()->getVp2fp()[vpath_] = fpath_;
            // // }
            // if(!state){
            //     std::cerr << "Some or ALL DataNode didn't finish put, please check.\n";
            // }
        }
    }

protected:
    CommandParser(std::shared_ptr<NameNode>& nameNode){
        nameNode_ = nameNode;
        globalVariables_ = GlobalVariables::getGlobalVariables();
        run_ = true;
        std::cout << "CommandParser inited.\n";
    };

    CommandParser(const CommandParser& ) = delete;

    CommandParser& operator= (const CommandParser&) = delete;

private:
    static std::shared_ptr<CommandParser> commandParser_;
    vector<string> parameters_;
    map<string, func> cmdHandler_;
    std::shared_ptr<NameNode> nameNode_;
    std::shared_ptr<GlobalVariables> globalVariables_;
    bool run_;
};

std::shared_ptr<CommandParser> CommandParser::commandParser_ = nullptr;

#endif // !COMMAND_PARSER_HPP