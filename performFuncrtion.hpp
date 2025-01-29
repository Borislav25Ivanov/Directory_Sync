#ifndef __PERFORM_FUNC
#define __PERFORM_FUNC
#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "fileOperations.hpp"

namespace fs = std::filesystem;

void perform(std::ifstream& syncFile){
    std::string input;
    std::getline(syncFile,input);
    bool blockCopy = false;
    if(input == "BLOCK-COPY")
        blockCopy = true;
    syncFile>>input>>input>>input;
    fs::path leftDirectory(input);
    syncFile>>input>>input>>input;
    fs::path rightDirectory(input);
    std::string source;
    std::string destination;

    while (syncFile)
    {
        syncFile>>input;
        if(input == "CREATE"){
           syncFile>>input;
           while(input != "|"){
            source += ' ' + input;
            syncFile>>input;
           }
           syncFile>>input;
           while (input != "|")
           {
            destination+= ' ' + input;
            syncFile>>input;
           }
            if(destination[1] == 'R'){
                destination = destination.substr(3);
                createDir(rightDirectory/destination);
            }
            else{
                destination = destination.substr(3);
                createDir(leftDirectory/destination);
            }
        source.clear();
        destination.clear();
        std::getline(syncFile,input);
        }
        else if(input == "COPY"){
            syncFile>>input;
           while(input != "|"){
            source += ' ' + input;
            syncFile>>input;
           }
           syncFile>>input;
           while (input != "|")
           {
            destination+= ' ' + input;
            syncFile>>input;
           }
            if(destination[1] == 'R'){
                destination = destination.substr(3);
                source = source.substr(3);
                if(!std::filesystem::exists(rightDirectory/destination) || !blockCopy){
                    copyFile(leftDirectory/source,rightDirectory/destination);
                }
                else
                    copyFileBlock(leftDirectory/source,rightDirectory/destination);
            }
            else{
                destination = destination.substr(3);
                source = source.substr(3);
                 if(!std::filesystem::exists(leftDirectory/destination) || !blockCopy){
                    copyFile(rightDirectory/source,leftDirectory/destination);
                }
                else
                    copyFileBlock(rightDirectory/source,leftDirectory/destination);
            }
        source.clear();
        destination.clear();
        std::getline(syncFile,input);
        }
        else if(input == "DELETE"){
           syncFile>>input;
           while (input != "|")
           {
            destination+= ' ' + input;
            syncFile>>input;
           }
            if(destination[1] == 'R')
                deleteFileOrDirectory(rightDirectory/destination.substr(3));
            else 
                deleteFileOrDirectory(leftDirectory/destination.substr(3));
        source.clear();
        destination.clear();
        std::getline(syncFile,input);
        }
        else if(input == "RENAME"){
             syncFile>>input;
           while(input != "|"){
            source += ' ' + input;
            syncFile>>input;
           }
           syncFile>>input;
           while (input != "|")
           {
            destination+= ' ' + input;
            syncFile>>input;
           }
            if(destination[1] == 'R'){
                source = source.substr(3);
                destination = destination.substr(3);
                Rename(rightDirectory/destination,rightDirectory/source);
            }
            else 
                Rename(leftDirectory/destination,leftDirectory/source);
        source.clear();
        destination.clear();
        std::getline(syncFile,input);
        }
        else if(input == "MOVE"){
             syncFile>>input;
           while(input != "|"){
            source += ' ' + input;
            syncFile>>input;
           }
           syncFile>>input;
           while (input != "|")
           {
            destination+= ' ' + input;
            syncFile>>input;
           }
            if(destination[1] == 'R'){
                source = source.substr(3);
                destination = destination.substr(3);
                Rename(rightDirectory/destination,rightDirectory/source);
            }
            else 
                Rename(leftDirectory/destination,leftDirectory/source);
        source.clear();
        destination.clear();
        std::getline(syncFile,input);
        }
    }
    

}
#endif