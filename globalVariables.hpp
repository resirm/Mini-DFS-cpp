#ifndef GLOBAL_MUTEX_HPP
#define GLOBAL_MUTEX_HPP

#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <vector>
#include <map>

class GlobalVariables {
public:
    static std::shared_ptr<GlobalVariables> getGlobalVariables(){
        if(globalVariables_ == nullptr){
            globalVariables_ = std::shared_ptr<GlobalVariables>(new GlobalVariables());
        }
        return globalVariables_;
    }

    std::mutex& getMutex(){
        return mtx_;
    }

    std::vector<std::condition_variable>& getCV(){
        return vcv_;
    }

    std::vector<bool>& getFlag(){
        return vflag_;
    }

    std::vector<std::string>& getCmd(){
        return vcmd_;
    }

    std::vector<std::vector<std::string>>& getPara(){
        return vpara_;
    }

    std::vector<bool>& getFinish(){
        return vfinish_;
    }

    std::vector<bool>& getState(){
        return vstate_;
    }

protected:
    GlobalVariables(size_t dataNodeNum = 4): dataNodeNum_(dataNodeNum){
        // std::vector<std::mutex> mtx(dataNodeNum_);
        for(auto idx = 0; idx < dataNodeNum_; ++idx){
            vflag_.push_back(true);
            vcmd_.push_back("");
            vpara_.push_back({});
            vfinish_.push_back(false);
            vstate_.push_back(false);
        }
        std::vector<std::condition_variable> cv(dataNodeNum_);
        // vmtx_.swap(mtx);
        vcv_.swap(cv);
        std::cout << "GlobalVariables inited.\n";
    }

    GlobalVariables(const GlobalVariables&) = delete;

    GlobalVariables& operator= (const GlobalVariables&) = delete;

private:
    static std::shared_ptr<GlobalVariables> globalVariables_;
    size_t dataNodeNum_;
    std::mutex mtx_;
    std::vector<std::condition_variable> vcv_;
    std::vector<bool> vflag_;
    std::vector<std::string> vcmd_;
    std::vector<std::vector<std::string>> vpara_;
    std::vector<bool> vfinish_;
    std::vector<bool> vstate_;
};

std::shared_ptr<GlobalVariables> GlobalVariables::globalVariables_ = nullptr;

#endif // !GLOBAL_MUTEX_HPP