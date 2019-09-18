#ifndef COMMAND_PARSER_HPP
#define COMMAND_PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include "fileSys.hpp"


using std::string; using std::vector; using std::map;
using std::cin; using std::cout; using std::cerr; using std::endl;

class CommandParser{
public:
    using func = std::function<void(void)>;
    static std::shared_ptr<CommandParser> getCommandParser(){
        if(commandParser_ == nullptr){
            commandParser_ = std::shared_ptr<CommandParser>(new CommandParser());
            init();
        }
        return commandParser_;
    }
    static void init(){
        commandParser_->cmdHandler_["mkdir"] = mkdir;
        commandParser_->cmdHandler_["touch"] = touch;
        commandParser_->cmdHandler_["rm"] = rm;
        commandParser_->cmdHandler_["rmdir"] = rmdir;
        commandParser_->cmdHandler_["ls"] = ls;
        commandParser_->cmdHandler_["cd"] = cd;
        commandParser_->cmdHandler_["pwd"] = pwd;
        commandParser_->cmdHandler_["put"] = put;
        commandParser_->cmdHandler_["quit"] = quit;
    }
    void work(){
        string cmd;
        while(run_){
            if(std::getline(cin, cmd)){
                auto rst = split(cmd);
                if(rst.size() <= 1 && rst[0] != "ls" && rst[0] != "pwd" && rst[0] != "quit"){
                    cerr << "Usage: \n";
                    continue;
                }
                parameters_ = rst;
                if(cmdHandler_.find(rst[0]) == cmdHandler_.end()){
                    cerr << "Usage: \n";
                    continue;
                }
                cmdHandler_[rst[0]]();
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
            commandParser_->filesys_->mkdir(*idx);
        }
    };

    static void touch(){
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->filesys_->touch(*idx);
        }
    };

    static void rm(){
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->filesys_->rm(*idx);
        }
    };

    static void rmdir(){
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->filesys_->rmdir(*idx);
        }
    };

    static void ls(){
        if(commandParser_->parameters_.size() == 1){
            commandParser_->filesys_->ls();
            return;
        }
        for(auto idx = commandParser_->parameters_.begin()+1; idx != commandParser_->parameters_.end(); ++idx){
            commandParser_->filesys_->ls(*idx);
        }
    };
    
    static void cd(){
        commandParser_->filesys_->cd(commandParser_->parameters_.at(1));
    };

    static void pwd(){
        commandParser_->filesys_->pwd();
    };

    static void put(){
        commandParser_->filesys_->put(commandParser_->parameters_.at(1), commandParser_->parameters_.at(2));
    };

    static void quit(){
        commandParser_->filesys_->getMeta()->saveMeta();
        commandParser_->stop();
    };

protected:
    CommandParser(){
        filesys_ = FileSys::getFileSys();
        run_ = true;
    };

    CommandParser(const CommandParser& ) = delete;

private:
    static std::shared_ptr<CommandParser> commandParser_;
    std::string dir_;
    vector<string> parameters_;
    map<string, func> cmdHandler_;
    std::shared_ptr<FileSys> filesys_;
    bool run_;
};

std::shared_ptr<CommandParser> CommandParser::commandParser_ = nullptr;

#endif // COMMAND_PARSER_HPP