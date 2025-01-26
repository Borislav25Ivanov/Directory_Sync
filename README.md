This project uses std::filesystem cpp library that requires c++17 to work.
The app syncronizes directories that are in the same file system, meaning it wont work if one directory is in disk C:\ and other in disk D:\
The app supports two modes:
  analyze 
  perform
Analyze can accept three main arguments
  mirror - makes the right directory the exact same as the left
  standart - no files are deleted, if a file is missing from one it is coppied that way and if a file has two different versions the newer one overwrites the old one
  safe - no files are changed or deleted, missing files are copied
    when folders are missing , all three modes create them
  Analzye can also accept two more arguments which are optional
    hash-only - it compares files only by their hashes so it can fail (if ommited files are compared bit by bit)
    block - if this argument is used durring the perform function all files are copied in 4kb chuncks intead of the whole file
  Analyze writes all neccessary steps needed for the syncronization in a file sync.txt which is created at the same location that the exe file is.

  The perform mode takes as argument the sync.exe file and executes all commands in it

  the arguments are added when executing the exe file from the console for example:
  ./ sync.exe analyze hash-only /dir-left /dir-right


  hashing algorithm implemeted by Oliver Gay 
 link to his work: https://github.com/ogay/sha2/blob/master/sha2.c
  
  
