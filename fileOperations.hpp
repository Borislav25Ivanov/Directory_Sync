#ifndef __FILE_OPERATIONS
#define __FILE_OPERATIONS
#include<iostream>
#include<filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <vector>
#include "sha2.h"

std::string compute_sha256(std::ifstream& file) {
    sha256_ctx ctx;
    sha256_init(&ctx);

    const size_t buffer_size = 4096;
    std::vector<uint8_t> buffer(buffer_size);


    while (file.read(reinterpret_cast<char*>(buffer.data()), buffer_size)) {
        sha256_update(&ctx, buffer.data(), file.gcount()); 
    }

    if (file.gcount() > 0) {
        sha256_update(&ctx, buffer.data(), file.gcount());
    }

    uint8_t digest[SHA256_DIGEST_SIZE];
    sha256_final(&ctx, digest);

    std::string hash_string;
    for (int i = 0; i < SHA256_DIGEST_SIZE; i++) {
        char buf[3];
        snprintf(buf, sizeof(buf), "%02x", digest[i]);
        hash_string += buf;
    }
    return hash_string;
}

bool compareFilesHashOnly(std::filesystem::path file1, std::filesystem::path file2,std::ofstream & out){
    std::ifstream fileOne(file1);
    std::ifstream fileTwo(file2);
    if (!fileOne) {
        out<<"UNABLE TO OPEN: "<<file1<<std::endl;
        return false;
    }
    if(!fileTwo){
        out<<"UNABLE TO OPEN: "<<file2<<std::endl;
        return false;
    }
    std::string left = compute_sha256(fileOne);
    std::string right = compute_sha256(fileTwo);
    fileOne.close();
    fileTwo.close();
    return left == right;
    
}

bool compareFilesBit(std::filesystem::path file1,std::filesystem::path file2,std::ofstream & out){
    std::ifstream fileOne(file1);
    std::ifstream fileTwo(file2);
    if (!fileOne) {
        out<<"UNABLE TO OPEN: "<<file1<<std::endl;
        return false;
    }
    if(!fileTwo){
        out<<"UNABLE TO OPEN: "<<file2<<std::endl;
        return false;
    }
    uint8_t a,b;
    while(!fileOne.eof() && !fileTwo.eof()){
        a = fileOne.get();
        b = fileTwo.get();
        if(a != b){
            return false;
        }
    }
    if (!fileOne.eof()){
        fileOne.close();
        fileTwo.close();
        return false;
    }
    if (!fileTwo.eof()){
        fileOne.close();
        fileTwo.close();
        return false;
    }
    
    fileOne.close();
    fileTwo.close();
    return true;
    
}
std::vector<uint8_t> compute_block_sha_256(const std::vector<uint8_t>& data){
    std::vector<uint8_t> hash(SHA256_DIGEST_SIZE);
    sha256_ctx ctx;
    sha256_init(&ctx);
    sha224_update(&ctx,data.data(),data.size());
    sha256_final(&ctx,hash.data());
    return hash;
}

void copyFile(std::filesystem::path source,std::filesystem::path destination){
    try
    {
        if(std::filesystem::exists(source) && std::filesystem::is_regular_file(source)){
            std::filesystem::remove(destination);
            std::filesystem::copy(source,destination);
        }
        else{
            std::cout<<"File "<<source<<" does not exist or the program is unable to access it!"<<std::endl;
        }
    }
    catch(const std::exception& e)
    {
        std::cerr<<"Error occured copying file"<<source<<" to "<<destination<<" : " << e.what() << '\n';
    }
    
}

void copyFileBlock(std::filesystem::path sourcePath,std::filesystem::path destinationPath){
    if(!std::filesystem::exists(sourcePath) || !std::filesystem::is_regular_file(sourcePath)){
        std::cout<<"File "<<sourcePath<<" does not exist or the program is unable to access it!"<<std::endl;
        return;
    }

    if(!std::filesystem::exists(destinationPath) || !std::filesystem::is_regular_file(destinationPath)){
        std::cout<<"File "<<destinationPath<<" does not exist or the program is unable to access it!"<<std::endl;
        return;
    }
    std::filesystem::resize_file(destinationPath,std::filesystem::file_size(sourcePath));
    std::ifstream source(sourcePath,std::ios::binary);
    std::fstream destination(destinationPath , std::ios::binary | std::ios::out | std::ios::in);
    size_t blockSize = 4096;
    std::vector<uint8_t> sourceBuffer(blockSize);
    std::vector<uint8_t> destBuffer(blockSize);

    
    size_t sourcePos = 0;
    size_t destPos = 0;
    while (true) {
        source.read(reinterpret_cast<char*>(sourceBuffer.data()), sourceBuffer.size());
        size_t bytesRead = source.gcount();

        if (bytesRead == 0) {
            break;
        }

        destination.seekp(destPos, std::ios::beg);
        destination.read(reinterpret_cast<char*>(destBuffer.data()), bytesRead);
        size_t bytesReadDest = destination.gcount();

        if (bytesReadDest != bytesRead || compute_block_sha_256(sourceBuffer) != compute_block_sha_256(destBuffer)) {
            destination.seekp(destPos, std::ios::beg);
            for(int i =0;i<bytesRead;i++){
                destination<<sourceBuffer[i];
            }
        }

        sourcePos += bytesRead;
        destPos += bytesRead;
    }

    destination.flush();  
    destination.seekp(0, std::ios::end); 
    size_t destFileSize = destination.tellg();
    if (sourcePos < destFileSize) {
        destination.close(); 
        std::filesystem::resize_file(destinationPath, sourcePos); 
    }
        destination.close();
        source.close();

}

void createDir(std::filesystem::path destination){
    try
    {
        std::filesystem::create_directory(destination);
    }
    catch(const std::exception& e)
    {
        std::cerr<<"Error creating directory" << e.what() << std::endl;
    }
    
}

void Rename(std::filesystem::path source, std::filesystem::path destination){
    try {
        std::filesystem::rename(source,destination);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error renaming directory: " << e.what() << std::endl;
    }
}

void deleteFileOrDirectory(std::filesystem::path destination){
   if(!std::filesystem::exists(destination))
        return;
   try {
        std::filesystem::remove(destination);
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error deleting directory: " << e.what() << std::endl;
    }
}


#endif