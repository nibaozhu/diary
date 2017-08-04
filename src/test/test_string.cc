#include <iostream>
#include <string>
#include <vector>

void string2vector(const std::string &str, const char &delim, std::vector<std::string> &vec)
{
    int i;
    std::string::const_iterator it;
    for(it = str.begin(), i = 0, vec.push_back(""); it != str.end(); it++) {
        if(*it == delim) {
            vec.push_back("");
            i++;
            continue;
        }
        vec.at(i).append(1, *it);
    }
}


int main(int argc, char **argv) {
	std::string s1("tcp:1.2.3.4:5:6");
	std::vector<std::string> v1;

	string2vector(s1, ':', v1);

	int i;
	for(i = 0; i < v1.size(); i++)
		std::cout << "i: "<< i << ",\t" << v1.at(i) << std::endl;

	return 0;
}
