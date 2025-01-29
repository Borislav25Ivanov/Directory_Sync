#ifndef __ANALYZE_FUNC
#define __ANALYZE_FUNC
#include<fstream>
#include<iostream>
#include"fileOperations.hpp"
#include<filesystem>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <set>

namespace fs = std::filesystem;

void mirror(const fs::path leftDirectory,const fs::path rightDirectory,std::ofstream& out,std::vector<std::string>& arguments){
    std::vector<fs::path> leftChildrenPaths;
    std::unordered_map<std::string,fs::path> hashToPathRightChildren;
    std::unordered_set<fs::path> rightChildrenPaths;

    for(auto & child : fs::recursive_directory_iterator(rightDirectory)){
        if(fs::is_directory(child)){
        rightChildrenPaths.insert(child.path().lexically_relative(rightDirectory));
        }
        else{
        rightChildrenPaths.insert(child.path().lexically_relative(rightDirectory));
        std::ifstream in(child.path());
        hashToPathRightChildren[compute_sha256(in)] = child.path().lexically_relative(rightDirectory);
        in.close();
        }

    }
    for (auto& child : fs::recursive_directory_iterator(leftDirectory)){
        leftChildrenPaths.push_back(child.path().lexically_relative(leftDirectory));
    }
    bool hashOnly = std::find(arguments.begin(),arguments.end(),std::string("hash-only")) != arguments.end();

        for(fs::path & child : leftChildrenPaths){
            if(fs::is_directory(leftDirectory / child) && rightChildrenPaths.find(child) == rightChildrenPaths.end()){
                 out<<"CREATE L\\"<<child.string()<<" | R\\"<<child.string()<<" | #Directory missing in RIGHT"<<std::endl;
            }
            else if(!fs::is_directory(child) && rightChildrenPaths.find(child) == rightChildrenPaths.end()){
                std::ifstream childLeft(leftDirectory / child);
                auto  it = hashToPathRightChildren.find(compute_sha256(childLeft));
                childLeft.close();
                if(it == hashToPathRightChildren.end()){
                    out<<"COPY L\\"<<child.string()<<" | R\\"<<child.string()<<" |  #File missing in RIGHT"<<std::endl;
                }
                else{
                    if(child.parent_path() == it->second.parent_path()){
                        if(hashOnly){
                           out<<"RENAME L\\"<<child.string()<<" | R\\"<<it->second.string()<<" | #File with different name in RIGHT"<<std::endl;
                            rightChildrenPaths.erase(it->second);
                        }
                        else if(compareFilesBit(leftDirectory / child, rightDirectory / (it->second),out)){
                                out<<"RENAME L\\"<<child.string()<<" | R\\"<<it->second.string()<<" | #File with different name in RIGHT comapred byte by byte"<<std::endl;
                                rightChildrenPaths.erase(it->second);
                            }
                    }
                    else
                    {
                        if(hashOnly){
                            out<<"MOVE L\\"<<child.string()<<" | R\\"<<it->second.string()<<" | #File in different location in RIGHT"<<std::endl;
                            rightChildrenPaths.erase(it->second);    
                        }
                        else if(compareFilesBit(leftDirectory / child, rightDirectory / (it->second),out)){
                            out<<"MOVE L\\"<<child.string()<<" | R\\"<<it->second.string()<<" | #File in different location in RIGHT compared byte by byte"<<std::endl;
                            rightChildrenPaths.erase(it->second);    
                        }
                    }
                }

            }
            else{
                if(fs::is_directory(leftDirectory/child)){
                    rightChildrenPaths.erase(child);
                    continue;
                }
                if(hashOnly){
                    if(compareFilesHashOnly(leftDirectory/child,rightDirectory/child,out))
                        rightChildrenPaths.erase(child);
                    else{
                        out<<"COPY L\\"<<child.string()<<" | R\\"<<child.string()<<" | #File different in RIGHT"<<std::endl;
                        rightChildrenPaths.erase(child);
                    }
                }
                else{
                     if(compareFilesBit(leftDirectory/child,rightDirectory/child,out))
                        rightChildrenPaths.erase(child);
                    else{
                        out<<"COPY L\\"<<child.string()<<" | R\\"<<child.string()<<" | #File different in RIGHT compared byte by byte"<<std::endl;
                        rightChildrenPaths.erase(child);
                    }
                }
            }
        }
        for(fs::path child : rightChildrenPaths){
            out<<"DELETE R\\"<<child.string()<<" | #Missing in LEFT"<<std::endl;
        }    
}

void standart(const fs::path leftDirectory,const fs::path rightDirectory, std::ofstream & out,std::vector<std::string>& arguments){
    bool hashOnly = std::find(arguments.begin(),arguments.end(),std::string("hash-only")) != arguments.end();
    std::set<fs::path> leftChildren;
    std::set<fs::path> rightChildren;
    for(auto & child : fs::recursive_directory_iterator(leftDirectory)){
        leftChildren.insert(child.path().lexically_relative(leftDirectory));
    }
    for(auto & child : fs::recursive_directory_iterator(rightDirectory)){
        rightChildren.insert(child.path().lexically_relative(rightDirectory));
    }
    for(fs::path child : leftChildren){
        auto it = rightChildren.find(child);
        if(it == rightChildren.end()){
            if(!fs::is_directory(leftDirectory/child)){
                out<<"COPY L\\"<<child.string()<<" | R\\"<<child.string()<<" | #Missing in RIGHT"<<std::endl;
                rightChildren.erase(child);
            }
            else{
                out<<"CREATE L\\"<<child.string()<<" | R\\"<<child.string()<<" | #Missing in RIGHT"<<std::endl;
                rightChildren.erase(child);
            } 

        }
        else if(!fs::is_directory(leftDirectory/child) && !fs::is_directory(rightDirectory/child)){
            if(hashOnly){
                if(!compareFilesHashOnly(leftDirectory/child,rightDirectory/child,out)){
                    if(fs::last_write_time(leftDirectory/child) > fs::last_write_time(rightDirectory/child))
                        out<<"COPY L\\"<<child.string()<<" | R\\"<<child.string()<<" | #Left copy is a newer version"<<std::endl;
                    else if(fs::last_write_time(leftDirectory/child) < fs::last_write_time(rightDirectory/child))
                        out<<"COPY R\\"<<child.string()<<" | L\\"<<child.string()<<" | #Right copy is a newer version"<<std::endl;
                }
                rightChildren.erase(child);
            }
            else{
                if(!compareFilesBit(leftDirectory/child,rightDirectory/child,out)){
                    if(fs::last_write_time(leftDirectory/child) > fs::last_write_time(rightDirectory/child))
                        out<<"COPY L\\"<<child.string()<<" | R\\"<<child.string()<<" | #Left copy is a newer version compared byte by byte"<<std::endl;
                    else
                        out<<"COPY R\\"<<child.string()<<" | L\\"<<child.string()<<" | #Right copy is a newer version compared byte by byte"<<std::endl;
                }
                rightChildren.erase(child);
            }
        }
        else{
            rightChildren.erase(child);
        } 

    }
    for(fs::path child : rightChildren){
        auto it = leftChildren.find(child);
        if(!fs::is_directory(rightDirectory/child)){
                out<<"COPY R\\"<<child.string()<<" | R\\"<<child.string()<<" | #Missing in LEFT"<<std::endl;
                rightChildren.erase(child);
            }
            else{
                out<<"CREATE R\\"<<child.string()<<" | R\\"<<child.string()<<" | #Missing in LEFT"<<std::endl;
                rightChildren.erase(child);
            } 
    }
}

void safe(const fs::path leftDirectory,const fs::path rightDirectory, std::ofstream & out,std::vector<std::string>& arguments){
    std::set<fs::path> leftChildren;
    std::set<fs::path> rightChildren;
    for(auto & child : fs::recursive_directory_iterator(leftDirectory)){
        leftChildren.insert(child.path().lexically_relative(leftDirectory));
    }
    for(auto & child : fs::recursive_directory_iterator(rightDirectory)){
        rightChildren.insert(child.path().lexically_relative(rightDirectory));
    }
    for(fs::path child : leftChildren){
        auto it = rightChildren.find(child);
        if(it == rightChildren.end()){
            if(fs::is_directory(leftDirectory/child)){
                out<<"CREATE L\\"<<child.string()<<" | R\\"<<child.string()<<" | #Missing in RIGHT"<<std::endl;
                rightChildren.erase(child);
            }   
            else{
                out<<"COPY L\\"<<child.string()<<" | R\\"<<child.string()<<" | #Missing in RIGHT"<<std::endl;
                rightChildren.erase(child);
            }
        }
    }
    for(fs::path child : rightChildren){
        auto it = leftChildren.find(child);
        if(it == leftChildren.end()){
            if(fs::is_directory(rightDirectory/child)){
                out<<"CREATE | R\\"<<child.string()<<" L\\"<<child.string()<<" | #Missing in LEFT"<<std::endl;
                rightChildren.erase(child);
            }   
            else{
                out<<"COPY | R\\"<<child.string()<<" L\\"<<child.string()<<" | #Missing in LEFT"<<std::endl;
                rightChildren.erase(child);
            }
        }
    }
}

void analyze(const fs::path leftDirecotry,const fs::path rightDirectory,std::ofstream& out,std::vector<std::string>& arguments){

    if(std::find(arguments.begin(),arguments.end(),std::string("mirror")) != arguments.end()){
        mirror(leftDirecotry,rightDirectory,out,arguments);
    }
    else if(std::find(arguments.begin(),arguments.end(),std::string("standart")) != arguments.end()){
        standart(leftDirecotry,rightDirectory,out,arguments);
    }
    else if(std::find(arguments.begin(),arguments.end(),std::string("safe")) != arguments.end()){
        safe(leftDirecotry,rightDirectory,out,arguments);
    }
    else
    throw std::invalid_argument("Unknown analyze argument/s");
}
#endif