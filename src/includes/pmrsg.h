#include <sstream>

class pmrsg {
public:
	static std::string rsg(const int size){
		std::string output;
    int N = size, M = 32;
    string alphabet("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
    for (int index = 0; index < N; index++)
    {
      string str;
      str.reserve(M);
      for (int i = 0; i < M; ++i)
      {
        output += alphabet[rand() % alphabet.length()];
      }
//    cout << str << endl;
    } 

		return output;
	}
};

