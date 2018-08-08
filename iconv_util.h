#ifndef __ICONV_UTIL_H__
#define __ICONV_UTIL_H__

#include <iconv.h>
#include <string>
#include "base.h"
#include <string.h>

class iconv_util
{
public:
	iconv_util ()
	{
		it = iconv_open ((const char*)"gbk", (const char*)"utf8");
		if (it == iconv_t(-1)) {
			pr_error ("iconv_open error.\n");
			return;
		}
	}

	~iconv_util ()
	{
		iconv_close (it);
	}

	std::string gbk2utf8(const std::string& str)
	{
		char dst[1024];
		memset (dst, 0, 1024);
			
		char* src_ptr = const_cast<char*>(str.c_str ());
		size_t src_size = str.size ();
		size_t dst_size = 1024ul;
		size_t ret = iconv (it, &src_ptr, &src_size, (char **)&dst, &dst_size);
		if (ret == (size_t) -1) {
			pr_error ("iconv err.\n");		
			return str;
		}

		return std::string (dst);
	}
private:
	iconv_t it;
};

#endif
