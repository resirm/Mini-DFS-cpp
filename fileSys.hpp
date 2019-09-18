#ifndef FILESYS_HPP
#define FILESYS_HPP

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <exception>
#include <cmath>
#include <utility>
#include "dataBlock.hpp"
#include "json.hpp"

using json = nlohmann::json;

class FileSysInfo{
public:
    using func = std::function<void(void)>;

    static std::shared_ptr<FileSysInfo> getFileSysInfo(){
        if(fileSysInfo_ == nullptr){
            fileSysInfo_ = std::shared_ptr<FileSysInfo>(new FileSysInfo());
        }
        return fileSysInfo_;
    }

    void countInc(){
        ++fileCount_;
    }

    size_t getCount(){
        return fileCount_;
    }

    void saveMeta(){
        saveJson(vp2idx, vpath2idx_);
        saveJson(fp2vp, fpath2vpath_);
        saveJson(idx2blks, idx2blks_);
        saveJson(ftree, fileTree_);
    }

    std::map<std::string, size_t>& getVp2idx(){
        return vpath2idx_;
    }

    std::map<std::string, std::string>& getFp2vp(){
        return fpath2vpath_;
    }

    std::map<size_t, std::pair<size_t, size_t>>& getIdx2blks(){
        return idx2blks_;
    }

    std::map<std::string, std::vector<std::pair<std::string, char>>>& getTree(){
        return fileTree_;
    }

    ~FileSysInfo(){};

protected:
    FileSysInfo(std::string metaRoot = "/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/metaData/"): 
        metaRoot_(metaRoot), fileCount_(0), vp2idx(metaRoot+"vp2idx.json"), 
        fp2vp(metaRoot+"fp2vp.json"), idx2blks(metaRoot+"idx2blks.json"), 
        ftree(metaRoot+"fileSys.json"){
        std::string cdcmd("cd "+metaRoot_);
        std::string mkcmd("mkdir -p "+metaRoot_);
        int ret = system(cdcmd.c_str());
        bool state = true;
        if(ret != 0){
            ret = system(mkcmd.c_str());
            std::cerr << "Root dir does NOT exist, creating new dir...\n";
            fileTree_["/"] = {};
        }else{
            bool state = true;
            state = state && loadJson(vp2idx, vpath2idx_);
            state = state && loadJson(fp2vp, fpath2vpath_);
            state = state && loadJson(idx2blks, idx2blks_);
            state = state && loadJson(ftree, fileTree_);
            if(fileTree_.find("/") == fileTree_.end()){
                fileTree_["/"] = {};
            }

            if(!state){
                std::cerr << "MetaData init failed.\n";
                return;
            }

            if(idx2blks_.size() > 0){
                fileCount_ = idx2blks_.rbegin()->first+1;
                cout << "Load fileCount: " << fileCount_ << endl;
            }else{
                cout << "No file found." << endl;
            }

        }
        if(ret != 0){
            std::cerr << "MetaData init failed.\n";
            return;
        }
        
    };

    FileSysInfo(const FileSysInfo& ) = delete;
private:
    template<typename T>
    bool loadJson(std::string name, T& dst){
        bool state = true;
        std::ifstream jsonin(name);
        if(jsonin.is_open()){
            json j;
            jsonin >> j;
            nlohmann::from_json(j, dst);
        }else{
            std::cerr << name + " open failed.\n";
            state = false;
        }
        jsonin.close();
        std::cout << name + " opened.\n";
        return state;
    }

    template<typename T>
    bool saveJson(std::string name, T& dst){
        bool state = true;
        std::ofstream jsonout(name, std::ios::out);
        if(jsonout.is_open()){
            json j(dst);
            jsonout << j;
            nlohmann::from_json(j, dst);
        }else{
            std::cerr << name + " open failed.\n";
            state = false;
        }
        jsonout.close();
        std::cout << name + " opened.\n";
        return state;
    }

private:
    static std::shared_ptr<FileSysInfo> fileSysInfo_;
    std::map<std::string, size_t> vpath2idx_;
    std::map<std::string, std::string> fpath2vpath_;
    std::map<size_t, std::pair<size_t, size_t>> idx2blks_;
    std::map<std::string, std::vector<std::pair<std::string, char>>> fileTree_;
    size_t fileCount_;
    std::string metaRoot_;
    const std::string vp2idx;
    const std::string fp2vp;
    const std::string idx2blks;
    const std::string ftree;
    // const std::string jfs;
    // json jFileSys;
};

std::shared_ptr<FileSysInfo> FileSysInfo::fileSysInfo_ = nullptr;

class FileSys{
public:
    static std::shared_ptr<FileSys> getFileSys(){
        if(fileSys_ == nullptr){
            fileSys_ = std::shared_ptr<FileSys>(new FileSys());
        }
        return fileSys_;
    }

    virtual bool mkdir(std::string name) {
        std::string path = (wd_ == "/" ? wd_ + name : wd_ + "/" + name);
        if(meta_->getTree().find(path) != meta_->getTree().end()){
            std::cerr << "Directory " + name + " already exists in " + wd_ + ", mkdir failed.\n";
            return false;
        }
        meta_->getTree()[path] = {};
        meta_->getTree()[wd_].push_back({name, 'd'});
        return true;
    }

    virtual bool touch(std::string name) {
        std::string path = (wd_ == "/" ? wd_ + name : wd_ + "/" + name);
        if(meta_->getVp2idx().find(path) != meta_->getVp2idx().end()){
            std::cerr << "File " + name + " already exists in " + wd_ + ", mkdir failed.\n";
            return false;
        }
        fileIdx_ = fileCount();
        meta_->getVp2idx()[path] = fileIdx_;
        meta_->getIdx2blks()[fileIdx_] = std::make_pair(0, 0);
        meta_->getTree()[wd_].push_back({name, 'f'});
        countInc();
        return true;
    }

    virtual bool rm(std::string name) { std::cerr << "Error! File can NOT rm.\n"; return false; }
    virtual bool rmdir(std::string name) { std::cerr << "Error! File can NOT rm.\n"; return false; }

    virtual bool ls(std::string name = "") const {
        bool state = true;
        std::string path = wd_;
        if(name == ""){
            ;
        }else if(name[0] == '/'){
            if(name.size() > 1 && name[name.size()-1] == '/'){
                name = name.substr(0, name.size()-1);
            }
            path = name;
        }else{
            if(name[name.size()-1] == '/'){
                name = name.substr(0, name.size()-1);
            }
            path += name;
        }

        if(meta_->getTree().find(path) == meta_->getTree().end()){
            std::cerr << path + " does NOT exist, will ls .\n";
            path = wd_;
            state = false;
        }
        std::cout << "ls ." << std::endl;

        std::vector<std::pair<std::string&, const char>> files, dirs;
        std::cout << std::setw(16) << "type" << std::setw(16) 
                  << "name" << std::setw(16) << std::setw(16) 
                  << "length" << std::setw(16) << "blocks" << endl;
        std::cout << std::setw(16) << "d" << std::setw(16) << "." 
                  << std::setw(16) << "-" << std::setw(16) << "-" << std::endl;
        std::cout << std::setw(16) << "d" << std::setw(16) << ".." 
                  << std::setw(16) << "-" << std::setw(16) << "-" << std::endl;
        for(auto& c : meta_->getTree()[path]){
            size_t id = meta_->getVp2idx()[path];
            std::string fname = c.first;
            const char ftype = c.second;
            size_t len = meta_->getIdx2blks()[id].second;
            size_t blks = meta_->getIdx2blks()[id].first;
            std::cout << std::setw(16) << ftype << std::setw(16) << fname 
                      << std::setw(16) << len << std::setw(16) << blks << std::endl;
        }
        return true;
    }

    bool cd(std::string name) {
        bool state = true;
        if(name.size() == 0){
            wd_ = "/";
            name_ = "/";
        }else if(name == "."){
            ;
        }else if(name == ".."){
            if(name_ == "/"){
                state = false;
            }else{
                std::string parentPath = wd_.substr(0, wd_.find_last_of("/"));
                wd_ = parentPath == "" ? "/" : parentPath;
                name_ = wd_.substr(wd_.find_last_of("/")+1);
            }
        }else if(name[0] == '/'){ // '/aaa/bbb/ccc', '/aaa/bbb/ccc'
            if(name.size() > 1 && name[name.size()-1] == '/'){
                // fomatting: remove '/' at the end.
                name = name.substr(0, name.size()-1);
            }
            if(meta_->getTree().find(name) == meta_->getTree().end()){
                std::cerr << name << " does NOT exist, cd to /\n";
                wd_ = "/";
                name_ = "/";
                state = false;
            }else{
                wd_ = name;
                name_ = wd_.substr(name.find_last_of("/")+1, std::string::npos);
            }
        }else{  // 'bbb/ccc/', 'bbb/ccc'
            if(name[name.size()-1] == '/'){
                // fomatting: remove '/' at the end.
                name = name.substr(0, name.size()-1);
            }
            name = (wd_ == "/" ? wd_ + name : wd_ + "/" + name);
            if(meta_->getTree().find(name) == meta_->getTree().end()){
                std::cerr << name << " does NOT exist, cd to /\n";
                wd_ = "/";
                name_ = "/";
                state = false;
            }else{
                wd_ = name;
                name_ = wd_.substr(name.find_last_of("/")+1, std::string::npos);
            }
        }
        return state;
    }
    
    void pwd() {
        std::cout << wd_ << std::endl;
    }

    bool put(std::string fpath, std::string name) {
        // fpath: real path, name: virtual name in pwd
        // to do, now is wrong
        std::string path = wd_ + name;
        if(meta_->getVp2idx().find(path) == meta_->getVp2idx().end()){
            std::cerr << "File " + name + " does NOT exist, creat it.\n";
            bool ret = touch(name);
            if(!ret){
                return false;
            }
        }
        bool ret = put_work(fpath, path);
        return ret;
    }

    std::string getRoot(){
        return root_;
    }

    std::string getName() {
        return name_;
    }

    
    std::shared_ptr<FileSysInfo>& getMeta(){
        return meta_;
    }

protected:
    FileSys(std::string rootPath = "/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/data/")
            : name_("/"), wd_("/"), root_(rootPath), len_(0), blkLen_(2048), fpath_(""), vpath_(""), blks_(0) {
                std::string cmd = "mkdir -p " + root_;
                system(cmd.c_str());
                meta_ = FileSysInfo::getFileSysInfo();
            }
    FileSys(const FileSys& filesys) = delete;

    FileSys& operator=(const FileSys& filesys) = delete;

    void countInc(){
        meta_->countInc();
    }

private:
    bool put_work(std::string fpath, std::string vpath) { 
        ifstream finput(fpath, std::ios::app);
        if(finput.is_open()){
            fpath_ = fpath;
            vpath_ = vpath;
            len_ = finput.tellg();
            std::cout << "file length: " << len_ << endl;
            blks_ = std::ceil(double(len_) / double(blkLen_));
            fileIdx_ = fileCount();
            finput.close();
        }else{
            std::cerr << "Error! Cannot get length of " + fpath + "\n";
            return false;
        }
        if(getMeta()->getVp2idx().find(vpath_) != getMeta()->getVp2idx().end()){
            cerr << "File " + vpath_ + " already exists, overwritting is not allowed.\n";
            return false;
        }
        size_t idx = 0;
        while(idx < blks_){
            ++idx;
            int blklen = blkLen_;
            if(idx == blks_){
                blklen = len_ % blkLen_;
            }
            vblk_.push_back(std::make_shared<DataBlock>(fpath_, vpath_, getRoot()+std::to_string(fileIdx_)+"-"+std::to_string(idx-1), blkLen_, blklen, idx*blkLen_, idx));
            vblk_.back()->fromFile();
            vblk_.back()->toFile();
        }
        cout << "put finished, " << blks_ << " blocks written.\n";
        getMeta()->getVp2idx()[fpath_] = fileIdx_;
        getMeta()->getFp2vp()[fpath_] = vpath_;
        getMeta()->getIdx2blks()[fileIdx_] = std::make_pair(blks_, len_);
        countInc();
        cout << fileCount() << endl;
        return true;
    }

    size_t fileCount(){ return meta_->getCount(); }

private:
    static std::shared_ptr<FileSys> fileSys_;
    std::string name_; // current directory
    std::string wd_; // working directory
    std::shared_ptr<FileSys> parent_;
    // std::shared_ptr<FileSys> cur_;
    std::string root_; // real path for filesys
    std::shared_ptr<FileSysInfo> meta_;

    // for put
    size_t len_; // total file length
    size_t blkLen_; // block size
    size_t blks_; // number of blocks
    size_t fileIdx_; // file index
    std::string fpath_; // file path
    std::string vpath_; // virtual path
    std::vector<std::shared_ptr<DataBlock>> vblk_; // vector of pointers of DataBlock
};

std::shared_ptr<FileSys> FileSys::fileSys_ = nullptr;

#endif // FILESYS_HPP