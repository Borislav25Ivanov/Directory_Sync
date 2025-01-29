#include <iostream>
#include <string>
#include <filesystem>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "fileOperations.hpp"
#include "analyzeFunctions.hpp"
#include "performFuncrtion.hpp"


namespace fs = std::filesystem;


int main(int argc ,char ** argv){
    std::string command(argv[1]);
    if(command == "analyze"){
        std::ofstream output("sync.txt");
        std::vector<std::string> arguments;
        fs::path leftDirectory(argv[argc-2]);
        fs::path rightDirectory(argv[argc-1]);
        for(int i = 1; i<argc-2;i++){
            arguments.push_back(argv[i]);
        }
        if(std::find(arguments.begin(),arguments.end(),std::string("block")) != arguments.end()){
            output<<"BLOCK-COPY"<<std::endl;
        }
        else{
            output<<"NO-BLOCK-COPY"<<std::endl;
        }
        output<<"LEFT IS: " << leftDirectory.string()<<std::endl; 
        output<<"RIGHT IS: " << rightDirectory.string()<<std::endl;
        analyze(leftDirectory,rightDirectory,output,arguments);
    }
    else if(command == "perform"){
        std::ifstream syncFile(argv[2]);
        perform(syncFile);
        if(!syncFile.is_open())
            throw std::invalid_argument("Unable to open sync instruction file!");
        syncFile.close();
    }
    else{
        throw std::invalid_argument("Unknown command!");
    }
}