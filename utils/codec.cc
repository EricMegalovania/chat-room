#include "./codec.hh"

#include <iostream>
#include <cassert>
#include <algorithm>

Codec::Codec() : _msg(""), _rest_msg_len(-1) {}
Codec::~Codec() {}

// encode ``str'' into res and return res
std::string Codec::encode(const std::string &str) {
	std::string metadata = [&](int x) -> std::string {
		std::string res = "";
		if (x < 128) {
			res += static_cast<char>(x | 128);
			return res;
		}
		for (int i = _metadata_len - 1; ~i; --i) {
			res += static_cast<char>(x % _div[i]);
			x /= _div[i];
		}
		assert(!x);
		std::reverse(res.begin(), res.end());
		return res;
	}(static_cast<int>(str.length()));
	return metadata + str;
}

// online decoding, and now take ``str'' as input
// return a vector showing the decoded msgs
std::vector<std::string> Codec::decode(const std::string &str) {
	std::vector<std::string> res;
	for (auto c : str) {
		_msg += c;
		if (_rest_msg_len == -1) {
			if (_msg.length() == 1) {
				int x = static_cast<unsigned char>(_msg[0]);
				if (x & 128) {
					_rest_msg_len = x & 127;
					_msg.clear();
				}
			}
			else if (_msg.length() == _metadata_len) {
				int x = 0;
				for (int i = 0; i < _metadata_len; ++i) {
					x = x * _div[i] + static_cast<unsigned char>(_msg[i]);
				}
				_rest_msg_len = x;
				_msg.clear();
			}
		}
		else {
			if (!--_rest_msg_len) {
				res.emplace_back(_msg);
				_rest_msg_len = -1;
				_msg.clear();
			}
		}
	}
	return res;
}