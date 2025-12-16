#ifndef _CODEC_H_
#define _CODEC_H_

#include <string>
#include <vector>
#include <bitset>

class Codec {
  private:
	static constexpr int _metadata_len = 4;
	static constexpr int _div[4]={128,256,256,256};
    std::string _msg;
	long long _rest_msg_len;

  public:
    Codec();
    ~Codec();

    std::string encode(const std::string &);
    std::vector<std::string> decode(const std::string &);
};

#endif