//
//  read_text_file.cpp
//  02564base
//
//  Created by Jakob Andreas Bærentzen on 03/01/2019.
//  Copyright © 2019 Jakob Andreas Bærentzen. All rights reserved.
//

#include <fstream>
#include <iostream>
#include "read_text_file.h"

namespace  GLGraphics {
    using namespace std;

    const std::string read_text_file(const std::string& path, const std::string& file)
    {
        string fullpath = path + "/" + file;
        ifstream is(fullpath.c_str(), ios::binary);
		if (is) {
			string str, contents;
			while (getline(is, str))
			{
				contents += str;
				contents += "\n";
			}
			return contents;
		}
		else {
			cout << "Failed to read :" << path << file << endl;
			return "";
		}
    }
}
