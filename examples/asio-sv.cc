/*
 * nghttp2 - HTTP/2 C Library
 *
 * Copyright (c) 2014 Tatsuhiro Tsujikawa
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
// We wrote this code based on the original code which has the
// following license:
//
// main.cpp
// ~~~~~~~~
//
// Copyright (c) 2003-2013 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <string>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <nghttp2/asio_http2_server.h>
#include <gzip.h>
#include <pmrsg.h>

using namespace nghttp2::asio_http2;
using namespace nghttp2::asio_http2::server;

int main(int argc, char *argv[]) {
  try {
    // Check command line arguments.
    if (argc < 4) {
      std::cerr
          << "Usage: asio-sv <address> <port> <threads> [<private-key-file> "
          << "<cert-file>]\n";
      return 1;
    }

    boost::system::error_code ec;

    std::string addr = argv[1];
    std::string port = argv[2];
    std::size_t num_threads = std::stoi(argv[3]);

    boost::iostreams::mapped_file_source file("index");
    std::string fileContent(file.data(), file.size());

    std::string top = R"treta(<!DOCTYPE html>
<html>
<head>
)treta";

    std::string head = R"treta(<title>Dummy content page</title>
<script>
  document.addEventListener("DOMContentLoaded", function(event) {
    console.log(Date.now() + " - DOM fully loaded and parsed !!!!!!!!!!!!!");
  });
  window.onload = function(){
	setTimeout(function(){
		var data ="";
		var t = window.performance.timing;
		var start = t.navigationStart;		
		for(value in t){
			if (t[value] != 0){
				data += value +": "+ (t[value] - start) + "\n";
			}else{
				data += value +": 0\n";
			}
		}
		//alert("domComplete: " + (t.domComplete - start) +"\nNow: " + ((new Date).getTime() - start));
		console.log(window.performance.timing.toJSON());
		console.log(data);
	}, 0);
}
 
</script>
)treta";

     std::string body = R"treta(</head>
<body>

<h1>My First Heading</h1>

<p>My first paragraph.</p>
<p>Lorem ipsum dolor sit amet, consectetur adipiscing elit. Integer nec odio. Praesent libero. Sed cursus ante dapibus diam. Sed nisi. Nulla quis sem at nibh elementum imperdiet. Duis sagittis ipsum. Praesent mauris. Fusce nec tellus sed augue semper porta. <b>Lorem ipsum dolor sit amet, consectetur adipiscing elit</b>. Mauris massa. Vestibulum lacinia arcu eget nulla. Class aptent taciti sociosqu ad litora torquent per conubia nostra, per inceptos himenaeos. </p>

)treta";

     std::string footer = R"treta(</body>
</html>)treta";

    http2 server;

    server.num_threads(num_threads);

    server.handle("/", [&top, &head, &body, &footer, &fileContent](const request &req, const response &res) {
      std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
      std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
      boost::system::error_code ec;
      std::string query = req.uri().raw_query;      
std::cout << "Start " << query << std::endl;
      std::vector<std::string> gets;    
      boost::split(gets, query, boost::is_any_of("?&"));
      bool no_push = (query.find("no_push") != std::string::npos);
      std::size_t have_multi = 0;
      std::string jssize = "1024";
      std::string neck = "";
      std::string belly = "";
      int bsize = 0;
      if (query.find("&js=") != std::string::npos) {
        jssize = query.substr(query.find("&js=") + 4 , query.find(",") - (query.find("&js=") + 4) );
std::cout << jssize << std::endl;
      }
      std::string jdata (fileContent.begin(), fileContent.begin()+ std::stoi(jssize));
      std::string jdatagz = Gzip::compress(jdata);
      
      for ( std::vector<std::string>::iterator i = gets.begin(); i != gets.end(); i++ ) {
        if (i->find("js") != std::string::npos) {
          have_multi = i->find(",");

	  if (have_multi != std::string::npos && std::stoi(i->substr(have_multi + 1)) ) {
	    for ( auto j = 0; j < std::stoi(i->substr(have_multi + 1)); j++ ) {
              neck += "<script src=\"/JS/" + std::to_string(j) + ".js?size=" + jssize + "\"></script>\n";
	      if (!no_push){
end = std::chrono::steady_clock::now();
std::cout << "PUSH " << std::to_string(j) << ".js?size=" << jssize << " [" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "]" << std::endl;
	        auto push = res.push(ec, "GET", "/JS/" + std::to_string(j) + ".js?size=" + jssize);
	        if (!ec) {
		  push->write_head(200, { {"Content-Encoding", {"gzip"}}, {"Content-type", {"application/javascript"}}, {"x-http2-push", {"1"}} });
                  push->end(jdatagz);
                }else{
                   std::cerr << "error: " << ec.message() << std::endl;
	        }
	      }
            }
	  } else {
	    neck += "<script src=\"JS/js.js?size=" + jssize + "\"></script>\n";
            if (!no_push){
end = std::chrono::steady_clock::now();
std::cout << "PUSH " << "js.js?size=" << jssize << " [" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "]" << std::endl;   
	      auto push = res.push(ec, "GET", "/JS/js.js?size=" + jssize);
                if (!ec) {
                  push->write_head(200, { {"Content-Encoding", {"gzip"}}, {"Content-type", {"application/javascript"}}, {"x-http2-push", {"1"}}});
                  push->end(jdatagz);
                }else{
                   std::cerr << "error: " << ec.message() << std::endl;
                }
	   }

	  }          
        }

	if(i->find("body") != std::string::npos) {
	  belly = "</head>\n";
	  bsize = std::stoi(i->substr(i->find("=") + 1));
	  std::string data (fileContent.begin(),fileContent.begin() + bsize);
	  belly += "<body><pre>"+ data + "</pre>\n";
	}
	
      }
// Google Chrome bug!!
//    res.write_head(200, { {"Content-Encoding", {"gzip"}}, {"Link",{"</JS/0.js?size=100>; rel=preload"}} });
      res.write_head(200, { {"Content-Encoding", {"gzip"}}});
      std::string all = top + head + neck + body + belly + footer;
    res.end(Gzip::compress(all));
end = std::chrono::steady_clock::now();
std::cout << "End " << query << "[" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "]" << std::endl << std::endl;
    });

    server.handle("/JS/", [&fileContent](const request &req, const response &res) {
     std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
     std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

     int jssize = 1024;
     std::string query = req.uri().raw_query;
std::cout << "Start " << query << std::endl;

     jssize = std::stoi(query.substr(query.find("=") + 1));
     std::string data (fileContent.begin(), fileContent.begin()+jssize);

     res.write_head(200, {{"Content-Encoding", {"gzip"}}, {"Content-type", {"application/javascript"}} });
     res.end(Gzip::compress('\"' + data + '\"'));
end = std::chrono::steady_clock::now();
std::cout << "End " << query << "[" << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "]" << std::endl << std::endl;

    });

    server.handle("/favicon.ico", [](const request &req, const response &res) {
      res.write_head(200, {{"Content-Type", {"application/octet-stream"}}});
      res.end(file_generator("favicon.ico"));
    });

    server.handle("/secret/", [](const request &req, const response &res) {
      res.write_head(200);
      res.end("under construction!\n");
    });

    server.handle("/push", [](const request &req, const response &res) {
      boost::system::error_code ec;
      auto push = res.push(ec, "GET", "/push/1");
      if (!ec) {
        push->write_head(200);
        push->end("server push FTW!\n");
      }

      res.write_head(200);
      res.end("you'll receive server push!\n");
    });
    server.handle("/delay", [](const request &req, const response &res) {
      res.write_head(200);

      auto timer = std::make_shared<boost::asio::deadline_timer>(
          res.io_service(), boost::posix_time::seconds(3));
      auto closed = std::make_shared<bool>();

      res.on_close([timer, closed](uint32_t error_code) {
        timer->cancel();
        *closed = true;
      });

      timer->async_wait([&res, closed](const boost::system::error_code &ec) {
        if (ec || *closed) {
          return;
        }

        res.end("finally!\n");
      });
    });
    server.handle("/trailer", [](const request &req, const response &res) {
      // send trailer part.
      res.write_head(200, {{"trailers", {"digest"}}});

      std::string body = "nghttp2 FTW!\n";
      auto left = std::make_shared<size_t>(body.size());

      res.end([&res, body, left](uint8_t *dst, std::size_t len,
                                 uint32_t *data_flags) {
        auto n = std::min(len, *left);
        std::copy_n(body.c_str() + (body.size() - *left), n, dst);
        *left -= n;
        if (*left == 0) {
          *data_flags |=
              NGHTTP2_DATA_FLAG_EOF | NGHTTP2_DATA_FLAG_NO_END_STREAM;
          // RFC 3230 Instance Digests in HTTP.  The digest value is
          // SHA-256 message digest of body.
          res.write_trailer(
              {{"digest",
                {"SHA-256=qqXqskW7F3ueBSvmZRCiSwl2ym4HRO0M/pvQCBlSDis="}}});
        }
        return n;
      });
    });

    file.close();

    if (argc >= 6) {
      boost::asio::ssl::context tls(boost::asio::ssl::context::sslv23);
      tls.use_private_key_file(argv[4], boost::asio::ssl::context::pem);
      tls.use_certificate_chain_file(argv[5]);

      configure_tls_context_easy(ec, tls);

      if (server.listen_and_serve(ec, tls, addr, port)) {
        std::cerr << "error: " << ec.message() << std::endl;
      }
    } else {
      if (server.listen_and_serve(ec, addr, port)) {
        std::cerr << "error: " << ec.message() << std::endl;
      }
    }
  } catch (std::exception &e) {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}
