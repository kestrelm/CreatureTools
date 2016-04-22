//
//  main.cpp
//  CreatureFlatData
//
//  Created by Kestrel Moon Studios on 10/8/15.
//  Copyright (c) 2015 Kestrel Moon Studios. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <vector>
#include <ConvertFlatData.h>


int main(int argc, const char * argv[]) {    
    if(argc != 3)
    {
        std::cerr<<"Runtime arguments: <Input JSON File> <Output FBB File>"<<std::endl;
        return 0;
    }
    
    // testing
    std::string src_filename(argv[1]);
    std::string dst_filename(argv[2]);
    ConvertToFlatData(src_filename, dst_filename);

    return 0;
}
