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
#include <algorithm>
#include "dataBlock.hpp"
#include "json.hpp"

using json = nlohmann::json;

class FileSysInfo{
public:
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
        bool state = true;
        state = state && saveJson(vp2idx, vpath2idx_);
        state = state && saveJson(vp2fp, vpath2fpath_);
        state = state && saveJson(idx2blks, idx2blks_);
        state = state && saveJson(ftree, fileTree_);
        if(state){
            std::cout << "MetaData saved.\n";
        }else{
            std::cout <<"MetaData save failed.\n";
        }
    }

    std::map<std::string, size_t>& getVp2idx(){
        return vpath2idx_;
    }

    std::map<std::string, std::string>& getVp2fp(){
        return vpath2fpath_;
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
        vp2fp(metaRoot+"vp2fp.json"), idx2blks(metaRoot+"idx2blks.json"), 
        ftree(metaRoot+"fileSys.json"){
        std::cout << "Initing, please wait...\n";
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
            state = state && loadJson(vp2fp, vpath2fpath_);
            state = state && loadJson(idx2blks, idx2blks_);
            state = state && loadJson(ftree, fileTree_);
            if(fileTree_.find("/") == fileTree_.end()){
                fileTree_["/"] = {};
            }

            if(!state){
                std::cerr << "MetaData init failed.\n";
                return;
            }

            if(vpath2fpath_.size() > 0){
                fileCount_ = vpath2fpath_.size();
                std::cout << "Load fileCount: " << fileCount_ << endl;
            }else{
                std::cout << "No file found." << endl;
            }
            std::cout << "Init finished.\n";
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
    std::map<std::string, std::string> vpath2fpath_;
    std::map<size_t, std::pair<size_t, size_t>> idx2blks_;
    std::map<std::string, std::vector<std::pair<std::string, char>>> fileTree_;
    size_t fileCount_;
    std::string metaRoot_;
    const std::string vp2idx;
    const std::string vp2fp;
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

    bool mkdir(std::string name) {
        bool state = true;
        std::string path(wd_);
        state = parsePath(path, name);
        if(state){
            std::cerr << "Directory " + name + " already exists in " + wd_ + ", mkdir failed.\n";
            return false;
        }
        path = name;

        std::vector<std::string> dirs = split(path, '/');
        std::string p = "";
        for(auto d : dirs){
            if(d != ""){
                std::string pre = (p == "" ? "/" : p);
                std::string cur = d;
                p = p + "/" + d;
                if(meta_->getTree().find(p) == meta_->getTree().end()){
                    meta_->getTree()[p] = {};
                    meta_->getTree()[pre].push_back({d, 'd'});
                }
            }
        }

        return true;
    }

    bool touch(std::string name) {
        bool state = true;
        std::string path(wd_);
        state = parsePath(path, name, 'f');
        
        if(state){
            std::cerr << "File " + name + " already exists in " + path + ", touch failed.\n";
            state = false;
        }else{
            std::string parent = path;
            path = (path == "/" ? path + name : path + "/" + name);

            fileIdx_ = fileCount();
            meta_->getVp2idx()[path] = fileIdx_;
            meta_->getIdx2blks()[fileIdx_] = std::make_pair(0, 0);
            meta_->getTree()[parent].push_back({name, 'f'});
            countInc();
        }

        return true;
    }

    bool rm(std::string name) {
        if(name == "" || name == "." || name == ".."){
            std::cerr << "rm " + name + " is NOT allowed, rm failed.\n";
            return false;
        }
        bool state = true;
        std::string path(wd_);
        state = parsePath(path, name, 'f');
        vpath_ = (path == "/" ? path + name : path + "/" + name);
        if(!state){
            std::cerr << "File " + vpath_ + " does NOT exist, rm failed.\n";
        }else{
            size_t fileIdx_ = meta_->getVp2idx()[vpath_];
            auto blksAndLen = meta_->getIdx2blks()[fileIdx_];
            fpath_ = meta_->getVp2fp()[vpath_];
            size_t blks_ = blksAndLen.first;
            size_t len_ = blksAndLen.second;
            size_t idx = 0;

            std::string rmcmd("rm " + getRoot()+std::to_string(fileIdx_)+"-*");

            size_t ret = system(rmcmd.c_str());
            if(ret == 0){
                std::cout << "rm finished, " << blks_ << " blocks removed.\n";
                meta_->getTree()[path].erase(std::find(meta_->getTree()[path].begin(), meta_->getTree()[path].end(), std::make_pair(name, 'f')));
                meta_->getIdx2blks().erase(fileIdx_);
                meta_->getVp2idx().erase(vpath_);
                meta_->getVp2fp().erase(vpath_);
                state = true;
            }else{
                std::cerr << "rm failed.\n";
                state = false;
            }
        }

        return state;
    }

    bool rmdir(std::string name) {
        if(name == "" || name == "." || name == ".."){
            std::cerr << "rm " + name + " is NOT allowed, rm failed.\n";
            return false;
        }
        bool state = true;
        std::string path(wd_);
        state = parsePath(path, name);
        if(!state){
            std::cerr << "Dir " + name + " does NOT exist, rm failed.\n";
        }else{
            bool empty = meta_->getTree()[path].empty();
            if(!empty){
                std::cerr << "Dir " + path + " is NOT empty, rm failed.\n";
                state = false;
            }else{
                meta_->getTree().erase(path);
                std::string parent = path.substr(0, path.find_last_of("/"));
                parent = (parent == "" ? "/" : parent);
                meta_->getTree()[parent].erase(std::find(meta_->getTree()[parent].begin(), meta_->getTree()[parent].end(), std::make_pair(name, 'd')));
            }
        }
        return state;
    }

    bool ls(std::string name = "") const {
        bool state = true;
        std::string path(wd_);
        state = parsePath(path, name);
        if(!state){
            if(name != ""){
                std::cerr << name + " does NOT exist, will ls .\n";
            }
            path = wd_;
        }

        std::cout << "ls " + path << std::endl;

        std::cout << std::setw(16) << "type" << std::setw(16) 
                  << "name" << std::setw(16) << std::setw(16) 
                  << "length" << std::setw(16) << "blocks" << endl;
        std::cout << std::setw(16) << "d" << std::setw(16) << "." 
                  << std::setw(16) << "-" << std::setw(16) << "-" << std::endl;
        std::cout << std::setw(16) << "d" << std::setw(16) << ".." 
                  << std::setw(16) << "-" << std::setw(16) << "-" << std::endl;
        for(auto& c : meta_->getTree()[path]){
            std::string fullPath = (path == "/" ? path + c.first : path + "/" + c.first);
            std::string fname = c.first;
            char ftype = c.second;
            size_t len = 0;
            size_t blks = 0;
            size_t id = 0;
            
            if(ftype == 'f'){
                // std::cout << "if ftype: " << ftype << endl;
                id = meta_->getVp2idx().at(fullPath);
                size_t len = meta_->getIdx2blks().at(id).second;
                size_t blks = meta_->getIdx2blks().at(id).first;
                std::cout << std::setw(16) << ftype << std::setw(16) << fname 
                          << std::setw(16) << len << std::setw(16) << blks << std::endl;
            }else{
                // std::cout << "else ftype: " << ftype << endl;
                std::cout << std::setw(16) << ftype << std::setw(16) << fname 
                          << std::setw(16) << "-" << std::setw(16) << "-" << std::endl;
            }
            
        }

        return state;
    }

    bool cd(std::string name) {
        bool state = true;
        std::string wd(wd_);
        state = parsePath(wd, name);
        if(!state){
            std::cerr << name + " does NOT exist, will cd /\n";
            wd = "/";
            name = "/";
        }
        wd_ = wd;
        name_ = name;
        return state;
    }
    
    void pwd() {
        std::cout << wd_ << std::endl;
    }

    bool put(std::string fpath, std::string name) {
        // fpath: real path, name: virtual name in pwd
        if(name == "" || name == "." || name == ".."){
            std::cerr << "You must put into a file!\n";
            return false;
        }
        bool state = true;
        std::string path(wd_);
        state = parsePath(path, name, 'f');
        if(!state){
            bool ret = true;
            if(meta_->getTree().find(path) == meta_->getTree().end()){
                std::cerr << "Dir " + path + " does NOT exist, creat it.\n";
                ret = mkdir(path);
            }
            if(!ret){
                std::cerr << "mkdir " + path + " failed, put failed.\n";
                return false;
            }
            path = (path == "/" ? path + name : path + "/" + name);
            std::cerr << "File " + path + " does NOT exist, creat it.\n";
            ret = touch(path);
            if(!ret){
                std::cerr << "touch " + path + " failed, put failed.\n";
                return false;
            }
        }

        bool ret = put_work(fpath, path);
        if(ret){
            meta_->getIdx2blks()[fileIdx_] = std::make_pair(blks_, len_);
            meta_->getVp2fp()[vpath_] = fpath_;
        }

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

            if((getMeta()->getVp2idx().find(vpath_) != getMeta()->getVp2idx().end()) && (getMeta()->getIdx2blks()[getMeta()->getVp2idx()[vpath_]].first > 0)){
                cerr << "File " + vpath_ + " already exists, overwritting is not allowed.\n";
                return false;
            }else{
                if((getMeta()->getVp2idx().find(vpath_) != getMeta()->getVp2idx().end()) && (getMeta()->getIdx2blks()[getMeta()->getVp2idx()[vpath_]].first == 0)){
                    fileIdx_ = getMeta()->getVp2idx()[vpath_];
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
            }
        }else{
            std::cerr << "Error! Cannot get length of " + fpath + "\n";
            return false;
        }
        
        return true;
    }

    bool parsePath(std::string&wd, std::string& name, char type = 'd') const{
        bool state = true;
        if(name.size() == 0){
            state = false;
        }else if(name == "."){
            ;
        }else if(name == ".."){
            if(name_ == "/"){
                state = false;
            }else{
                std::string parentPath = wd.substr(0, wd.find_last_of("/"));
                wd = (parentPath == "" ? "/" : parentPath);
                name = wd.substr(wd.find_last_of("/")+1);
            }
        }else if(name[0] == '/'){ // '/aaa/bbb/ccc', '/aaa/bbb/ccc'
            if(name.size() > 1 && name[name.size()-1] == '/'){
                // fomatting: remove '/' at the end.
                name = name.substr(0, name.size()-1);
            }
            if(type == 'd'){
                if(meta_->getTree().find(name) == meta_->getTree().end()){
                    state = false;
                }else{
                    wd = name;
                    name = wd.substr(name.find_last_of("/")+1, std::string::npos);
                    name = name == "" ? "/" : name;
                }
            }else{ // type == 'f'
                std::string path(name);
                name = path.substr(path.find_last_of("/")+1, std::string::npos);
                wd = path.substr(0, path.find_last_of("/"));
                wd = wd == "" ? "/" : wd;
                if(meta_->getTree().find(wd) == meta_->getTree().end()){
                    state = false;
                }else if(std::find(meta_->getTree()[wd].begin(), meta_->getTree()[wd].end(), std::make_pair(name, 'f')) == meta_->getTree()[wd].end()){
                    state = false;
                }
            }
        }else{  // 'bbb/ccc/', 'bbb/ccc'
            if(name[name.size()-1] == '/'){
                // fomatting: remove '/' at the end.
                name = name.substr(0, name.size()-1);
            }
            name = (wd == "/" ? wd + name : wd + "/" + name);
            if(type == 'd'){
                if(meta_->getTree().find(name) == meta_->getTree().end()){
                    state = false;
                }else{
                    wd = name;
                    name = wd.substr(name.find_last_of("/")+1, std::string::npos);
                    name = name == "" ? "/" : name;
                }
            }else{ // type == 'f'
                std::string path(name);
                name = path.substr(path.find_last_of("/")+1, std::string::npos);
                wd = path.substr(0, path.find_last_of("/"));
                wd = wd == "" ? "/" : wd;
                if(meta_->getTree().find(wd) == meta_->getTree().end()){
                    state = false;
                }else if(std::find(meta_->getTree()[wd].begin(), meta_->getTree()[wd].end(), std::make_pair(name, 'f')) == meta_->getTree()[wd].end()){
                    state = false;
                }
            }
        }
        return state;
    }

    std::vector<std::string> split(string str, char sep=' '){
        // cout << str << endl;
        std::vector<std::string> rst;
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

    size_t fileCount(){
        return meta_->getCount();
    }

private:
    static std::shared_ptr<FileSys> fileSys_;
    std::string name_; // current directory
    std::string wd_; // working directory
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