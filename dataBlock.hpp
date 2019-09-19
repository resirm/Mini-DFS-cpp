#ifndef DATA_BLOCK_HPP
#define DATA_BLOCK_HPP

#include <fstream>
#include <iostream>
#include <string>

using std::string;
using std::ifstream; using std::ofstream; using std::fstream;
using std::cin; using std::cout; using std::cerr; using std::endl;

class DataBlock{
public:
    DataBlock(string path = "", string vpath = "", string innerPath = "", size_t max = 2048, size_t len = 0, size_t off = 0, size_t idx = 0): 
        file_(path), vpath_(vpath), innerPath_(innerPath), maxLen_(max), blkLen_(len), offset_(off), index_(idx){
            
        buffer_ = new char[maxLen_];
    }

    ~DataBlock(){
        delete[] buffer_;
    }

    void setPath(string fpath){
        file_ = fpath;
    }

    string getPath(){
        return file_;
    }

    void setIpath(string vpath){
        innerPath_ = vpath;
    }

    string getIpath(){
        return innerPath_;
    }

    size_t fromFile(){
        ifstream finput(file_, std::ios::binary | std::ios::in);
        if(finput.is_open() && finput.seekg(offset_)){
            finput.read(buffer_, maxLen_);
            blkLen_ = finput.gcount();
            finput.close();
        }else{
            cerr << "Open file " + file_ + " failed.\n";
        }
        return blkLen_;
    }

    void toFile(){
        ofstream foutput(innerPath_, std::ios::binary | std::ios::out);
        if(foutput.is_open()){
            foutput.write(buffer_, blkLen_);
            foutput.close();
        }else{
            cerr << "Open file " + innerPath_ + "_ failed.\n";
        }
    }
private:
    string file_; // origianl file full-path
    string innerPath_; // file block stored path
    string vpath_; // virtual path
    size_t maxLen_; // max length of a block
    size_t blkLen_; // blk length
    size_t offset_; // offset in the original file
    size_t index_; // block number 
    char* buffer_;
};

#endif // !DATA_BLOCK_HPP