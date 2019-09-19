#ifndef DATANODE_HPP
#define DATANODE_HPP

#include "fileSys.hpp"
#include "globalVariables.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <thread>
#include <sstream>

using std::cin; using std::cout; using std::cerr; using std::endl;
using std::string; 
using std::vector;

class DataNode: public FileSys, public std::enable_shared_from_this<DataNode>{
public:
    DataNode(string name, size_t id, string rootPath = "/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/"):
            FileSys(name, rootPath), name_(name), id_(id), ready_(true){
        globalVariables_ = GlobalVariables::getGlobalVariables();
        cout << "DataNode " + name + " inited.\n";
    }

    void operator() () {
        cout << "DataNode " + name_ + " started, thread_id: " << std::this_thread::get_id() << endl;
        run();
    }

    void mkdir(std::vector<std::string> paras) {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        for(auto idx = paras.begin(); idx != paras.end(); ++idx){
            FileSys::mkdir(*idx);
        }
        ready_ = true;
    };

    void touch(std::vector<std::string> paras) {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        for(auto idx = paras.begin(); idx != paras.end(); ++idx){
            FileSys::touch(*idx);
        }
        ready_ = true;
    };

    void rm(std::vector<std::string> paras) {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        for(auto idx = paras.begin(); idx != paras.end(); ++idx){
            FileSys::rm(*idx);
        }
        ready_ = true;
    };

    void rmdir(std::vector<std::string> paras) {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        for(auto idx = paras.begin(); idx != paras.end(); ++idx){
            FileSys::rmdir(*idx);
        }
        ready_ = true;
    };

    void ls(std::vector<std::string> paras) {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        if(paras.empty()){
            FileSys::ls();
            return;
        }
        for(auto idx = paras.begin(); idx != paras.end(); ++idx){
            FileSys::ls(*idx);
        }
        ready_ = true;
    };
    
    void cd(std::vector<std::string> paras) {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        FileSys::cd(paras.front());
        ready_ = true;
    };

    void pwd() {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        FileSys::pwd();
        ready_ = true;
    };

    bool put(std::vector<std::string> paras) {
        {
            std::unique_lock<std::mutex> ul(mtx_);
            cv_.wait(ul, [&, this]()->bool{ return ready_; });
            ready_ = false;
        }
        std::istringstream ss(paras.at(2));
        size_t fid;
        ss >> fid;
        std::cout << "fid: " << fid << std::endl;
        bool ret = FileSys::put(paras.at(0), paras.at(1), fid);
        ready_ = true;
        return ret;
    };

private:
    void run(){
        while(true){
            {
                std::unique_lock<std::mutex> ul(globalVariables_->getMutex());
                globalVariables_->getCV()[id_].wait(ul, [&, this]()->bool{ return globalVariables_->getFlag()[id_]; });
                // std::cout << "DataNode awake! ,thread_id: " << std::this_thread::get_id() << std::endl;
                globalVariables_->getFlag()[id_] = false;
                cmd_ = globalVariables_->getCmd()[id_];
                paras_ = globalVariables_->getPara()[id_];
                globalVariables_->getCmd()[id_].clear();
                globalVariables_->getPara()[id_].clear();
            

            // std::cout << "DataNode working! ,thread_id: " << std::this_thread::get_id() << std::endl;
            
                if(cmd_ == ""){
                    cmd_.clear();
                    paras_.clear();
                }else if(cmd_ == "rm"){
                    rm(paras_);
                    cmd_.clear();
                    paras_.clear();
                }else if(cmd_ == "put"){
                    bool state = put(paras_);
                    cmd_.clear();
                    paras_.clear();
                    globalVariables_->getState()[id_] = state;
                    globalVariables_->getFinish()[id_] = true;
                    if(!state){
                        std::cerr << "DataNode " + std::to_string(id_) + " put failed.\n";
                    }
                }else{
                    cerr << "Unknown command!\n";
                    cmd_.clear();
                    paras_.clear();
                }

                globalVariables_->getFlag()[id_] = true;
                globalVariables_->getCV()[id_].notify_all();

            }
        }
    }

private:
    string name_;
    size_t id_;
    std::mutex mtx_;
    std::condition_variable cv_;
    bool ready_;
    std::shared_ptr<GlobalVariables> globalVariables_;
    string cmd_;
    vector<string> paras_;
};

#endif // !DATANODE_HPP