#ifndef __GZIP_H__
#define __GZIP_H__

#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>

#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>


class Gzip {
public:
	static std::string compress(const std::string& data)
	{
		namespace bio = boost::iostreams;

		std::stringstream compressed;
		std::stringstream origin(data);

		bio::filtering_streambuf<bio::input> out;
		out.push(bio::gzip_compressor(bio::gzip_params(bio::gzip::best_compression)));
		out.push(origin);
		bio::copy(out, compressed);
		return compressed.str();
	}

	static std::string decompress(const std::string& data)
	{
		namespace bio = boost::iostreams;

		std::stringstream compressed(data);
		std::stringstream decompressed;

		bio::filtering_streambuf<bio::input> out;
		out.push(bio::gzip_decompressor());
		out.push(compressed);
		bio::copy(out, decompressed);

		return decompressed.str();
	}

	static std::string data_compress(const int size)
	{
		std::string data = rsg(size);
		return compress(data);
	}

private:

        static std::string rsg(const int size){
                std::stringstream output;
                std::string chars(
                        "abcdefghijklmnopqrstuvwxyz"
                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        "1234567890"
                        "!@#$%^&*()"
                        "`~-_=+[{]}\\|;:'\",<.>/? ");
                boost::random::random_device rng;
                boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
                for(int i = 0; i < size; ++i) {
                        output << chars[index_dist(rng)];
                }
                return output.str();
        }
};

#endif // __GZIP_H__
