#ifndef FILESYS_HPP
#define FILESYS_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <memory>
#include <exception>
#include <cmath>
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
        std::ofstream jsonout(vp2idx, std::ios::out);
        if(jsonout.is_open()){
            json j(vpath2idx_);
            jsonout << j;
            nlohmann::from_json(j, vpath2idx_);
        }else{
            std::cerr << "vp2idx.json open failed.\n";
        }
        jsonout.close();
        jsonout.open(fp2vp, std::ios::out);
        if(jsonout.is_open()){
            json j(fpath2vpath_);
            jsonout << j;
            nlohmann::from_json(j, fpath2vpath_);
        }else{
            std::cerr << "fp2vp.json open failed.\n";
        }
        jsonout.close();
        jsonout.open(idx2blks, std::ios::out);
        if(jsonout.is_open()){
            json j(idx2blks_);
            jsonout << j;
            nlohmann::from_json(j, idx2blks_);
        }else{
            std::cerr << "idx2blks.json open failed.\n";
        }
        jsonout.close();
        jsonout.open(jfs, std::ios::out);
        if(jsonout.is_open()){
            jsonout << jFileSys;
        }else{
            std::cerr << "fileSys.json open failed.\n";
        }
        jsonout.close();
    }

    std::map<std::string, size_t>& getVp2idx(){
        return vpath2idx_;
    }

    std::map<std::string, std::string>& getFp2vp(){
        return fpath2vpath_;
    }

    std::map<size_t, size_t>& getIdx2blks(){
        return idx2blks_;
    }

    json& getJson(){
        return jFileSys;
    }

    ~FileSysInfo(){};

protected:
    FileSysInfo(std::string metaRoot = "/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/metaData/", size_t fileCount = 0): 
        metaRoot_(metaRoot), fileCount_(fileCount), vp2idx(metaRoot+"vp2idx.json"), fp2vp(metaRoot+"fp2vp.json"), idx2blks(metaRoot+"idx2blks.json"), jfs(metaRoot+"fileSys.json"){
        std::string cdcmd("cd "+metaRoot_);
        std::string mkcmd("mkdir -p "+metaRoot_);
        int ret = system(cdcmd.c_str());
        bool state = true;
        if(ret != 0){
            ret = system(mkcmd.c_str());
            jFileSys = {{"files", json::array()}, {"folders", json::array()}};
        }else{
            std::ifstream jsonin(vp2idx);
            if(jsonin.is_open()){
                json j;
                jsonin >> j;
                nlohmann::from_json(j, vpath2idx_);
            }else{
                std::cerr << "vp2idx.json open failed.\n";
                state = false;
            }
            jsonin.close();
            std::cout << "vp2idx.json opened.\n";
            jsonin.open(fp2vp);
            if(jsonin.is_open()){
                json j;
                jsonin >> j;
                nlohmann::from_json(j, fpath2vpath_);
            }else{
                std::cerr << "fp2vp.json open failed.\n";
                state = false;
            }
            jsonin.close();
            std::cout << "fp2vp.json opened.\n";
            jsonin.open(idx2blks);
            if(jsonin.is_open()){
                json j;
                jsonin >> j;
                nlohmann::from_json(j, idx2blks_);
                if(idx2blks_.size() > 0){
                    fileCount_ = idx2blks_.rbegin()->first+1;
                }
                cout << "Load fileCount: " << fileCount_ << endl;
            }else{
                std::cerr << "idx2blks.json open failed.\n";
                state = false;
            }
            jsonin.close();
            std::cout << "idx2blks.json opened.\n";
            jsonin.open(jfs);
            if(jsonin.is_open()){
                jsonin >> jFileSys;
            }else{
                std::cerr << "fileSys.json open failed.\n";
                state = false;
            }
            jsonin.close();
            std::cout << "fileSys.json opened.\n";
            if(!state){
                std::cerr << "MetaData init failed.\n";
                return;
            }
        }
        if(ret != 0){
            std::cerr << "MetaData init failed.\n";
            return;
        }
        
    };

    FileSysInfo(const FileSysInfo& ) = delete;

private:
    static std::shared_ptr<FileSysInfo> fileSysInfo_;
    std::map<std::string, size_t> vpath2idx_;
    std::map<std::string, std::string> fpath2vpath_;
    std::map<size_t, size_t> idx2blks_;
    size_t fileCount_;
    std::string metaRoot_;
    const std::string vp2idx;
    const std::string fp2vp;
    const std::string idx2blks;
    const std::string jfs;
    json jFileSys;
};

std::shared_ptr<FileSysInfo> FileSysInfo::fileSysInfo_ = nullptr;

class FileSys: public std::enable_shared_from_this<FileSys>{
public:
    virtual void init(){};
    virtual bool mkdir(std::string name) { std::cerr << "Error! File can NOT mkdir.\n"; return false; }
    virtual bool touch(std::string name) { std::cerr << "Error! File can NOT touch.\n"; return false; }
    virtual bool rm(std::string name) { std::cerr << "Error! File can NOT rm.\n"; return false; }
    virtual bool rmdir(std::string name) { std::cerr << "Error! File can NOT rm.\n"; return false; }
    virtual bool ls(std::string name = "") const { std::cerr << "Error! File can NOT ls.\n"; return false; }
    virtual bool put(std::string fpath, std::string vpath) { std::cerr << "Error! File can NOT put.\n"; return false; }
    virtual std::shared_ptr<FileSys> cd(std::string name) { std::cerr << "Error! File can NOT cd.\n"; return shared_from_this(); }
    std::shared_ptr<FileSys> dir() { return parent_; }
    std::string pwd() {
        std::shared_ptr<FileSys> parent = dir();
        std::string path = name_;
        while(parent != nullptr && parent->getName() != "/"){
            path = parent->getName() + "/" + path;
            // std::cout << "pwd out: " << parent.use_count() << std::endl;
            // std::cout << parent->getName() << std::endl;
            parent = parent->dir();
        }
        path = path == "/" ? path : "/" + path + "/";
        return path;
    }
    std::string getRoot(){ return root_; }
    std::string getName() { return name_; }
    size_t fileCount(){ return meta_->getCount(); }
    std::shared_ptr<FileSysInfo> getMeta(){ return meta_; }
protected:
    FileSys(std::string name, std::shared_ptr<FileSys> parent = nullptr, std::string rootPath = "/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/data/")
            : name_(name), parent_(parent), root_(rootPath) {
                std::string cmd = "mkdir -p " + root_;
                system(cmd.c_str());
            }
    FileSys(const FileSys& filesys){
        name_ = filesys.name_;
        parent_ = filesys.parent_;
        root_ = filesys.root_;
        // cur_ = filesys.cur_;
        meta_ = filesys.meta_;
        std::string cmd = "mkdir -p " + root_;
        system(cmd.c_str());
    }
    FileSys& operator=(const FileSys& filesys){
        name_ = filesys.name_;
        parent_ = filesys.parent_;
        root_ = filesys.root_;
        // cur_ = filesys.cur_;
        meta_ = filesys.meta_;
        std::string cmd = "mkdir -p " + root_;
        system(cmd.c_str());
        return *this;
    }
    void countInc(){
        meta_->countInc();
    }
private:
    std::string name_;
    std::shared_ptr<FileSys> parent_;
    // std::shared_ptr<FileSys> cur_;
    std::string root_;
    std::shared_ptr<FileSysInfo> meta_;
};

class File: public FileSys{
public:
    File(std::string name, std::shared_ptr<FileSys> parent, size_t len = 0, size_t blkLen = 2048, std::string fpath = "", std::string vpath = "", size_t blks = 0): 
        FileSys(name, parent), len_(len), blkLen_(blkLen), fpath_(fpath), vpath_(vpath), blks_(blks) {

        }

    ~File(){}

    bool put(std::string fpath, std::string vpath) override{
        ifstream finput(fpath, std::ios::app);
        if(finput.is_open()){
            fpath_ = fpath;
            vpath_ = vpath;
            len_ = finput.tellg();
            cout << "file length: " << len_ << endl;
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
        getMeta()->getIdx2blks()[fileIdx_] = blks_;
        countInc();
        cout << fileCount() << endl;
        return true;
    }

private:
    size_t len_; // total file length
    size_t blkLen_; // block size
    size_t blks_; // number of blocks
    size_t fileIdx_; // file index
    std::string fpath_; // file path
    std::string vpath_; // virtual path
    std::vector<std::shared_ptr<DataBlock>> vblk_; // vector of pointers of DataBlock
};

class Folder: public FileSys{
public:
    Folder(std::string name, std::shared_ptr<FileSys> parent = nullptr, std::string rootPath = "/Users/yifengzhu/Code/Mini-DFS-cpp/miniDFS/data/"): FileSys(name, parent, rootPath){
        // auto meta = getMeta();
        // for(auto j : meta->getJson()["files"]){
        //     files_[j] = nullptr;
        // }
        // for(auto j : meta->getJson()["folders"]){
        //     folders_[j] = nullptr;
        // }
    }

    Folder(const Folder& folder): FileSys(folder) {
        files_ = folder.files_;
        folders_ = folder.folders_;
    }

    Folder& operator=(const Folder& folder){
        FileSys::operator=(folder);
        files_ = folder.files_;
        folders_ = folder.folders_;
        return *this;
    }

    ~Folder(){};

    void init() override{
        folders_["."] = std::dynamic_pointer_cast<Folder>(shared_from_this());
        folders_[".."] = std::dynamic_pointer_cast<Folder>(dir());
    }

    bool mkdir(std::string name) override {
        if(folders_.find(name) == folders_.end()){
            folders_[name] = std::make_shared<Folder>(name, std::dynamic_pointer_cast<FileSys>(shared_from_this()));
            folders_[name]->init();
            getMeta()->getJson()["folders"].push_back(folders_[name]->pwd());
        }else{
            std::cerr << "Directory " + name + " already exists, mkdir failed.\n";
            return false;
        }
        return true;
    }

    bool touch(std::string name) override {
        if(files_.find(name) == files_.end()){
            files_[name] = std::make_shared<File>(name, std::dynamic_pointer_cast<FileSys>(shared_from_this()));
            getMeta()->getJson()["files"].push_back(files_[name]->pwd());
        }else{
            std::cerr << "File " + name + " already exists, touch failed.\n";
            return false;
        }
        return true;
    }

    bool rm(std::string name) override {
        size_t ret = files_.erase(name);
        if(ret == 0){
            std::cerr << "File " + name + " does NOT exist, rm failed.\n";
            return false;
        }
        std::cerr << "File " + name + " removed.\n";
        return true;
    }

    bool rmdir(std::string name) override {
        size_t ret = folders_.erase(name);
        if(ret == 0){
            std::cerr << "Directory " + name + " does NOT exist, rmdir failed.\n";
            return false;
        }
        std::cerr << "Directory " + name + " removed.\n";
        return true;
    }

    bool ls(std::string name = "") const override {
        if(name == ""){
            std::cout << "file:\n";
            for(auto f : files_){
                std::cout << f.first+" ";
            }
            std::cout << "\n\ndirectory:\n";
            for(auto d : folders_){
                std::cout << d.first+" ";
            }
            std::cout << std::endl;
            return true;
        }
        return folders_.at(name)->ls();
    }

    std::shared_ptr<FileSys> cd(std::string name) override {
        if(folders_.find(name) != folders_.end()){
            try{
                std::cout << "Enter directory " + name + "\n";
                if(folders_[name] == nullptr && name == ".."){
                    std::cerr << "Now at root. cd .. failed.\n";
                }else{
                    return std::dynamic_pointer_cast<FileSys>(folders_[name]);
                }
            }catch(std::bad_weak_ptr& e){
                std::cerr << e.what() << std::endl;
                return std::dynamic_pointer_cast<FileSys>(shared_from_this());
            }
        }//else if(getMeta()->getJson().find(pwd()+name+"/") != getMeta()->getJson().end()){
            //
        //}
        std::cerr << "Directory " + name + " does NOT exist, cd failed.\n";
        return std::dynamic_pointer_cast<FileSys>(shared_from_this());
    }

    bool put(std::string fpath, std::string name) override {
        // fpath: real path, name: virtual name in pwd
        if(files_.find(name) == files_.end()){
            std::cerr << "File " + name + " does NOT exist, creat it.\n";
            bool ret = touch(name);
            if(!ret){
                return false;
            }
        }
        std::string vpath = pwd() + name;
        bool ret = files_[name]->put(fpath, vpath);
        return ret;
    }
private:
    std::map<std::string, std::shared_ptr<File>> files_;
    std::map<std::string, std::shared_ptr<Folder>> folders_;
};

#endif // FILESYS_HPP