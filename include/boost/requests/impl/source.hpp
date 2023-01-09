// Copyright (c) 2022 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_REQUESTS_IMPL_SOURCE_HPP
#define BOOST_REQUESTS_IMPL_SOURCE_HPP

#include <boost/requests/source.hpp>
#include <boost/requests/detail/async_coroutine.hpp>
#include <boost/requests/detail/pmr.hpp>
#include <boost/asio/consign.hpp>
#include <boost/asio/prepend.hpp>
#include <boost/beast/http/write.hpp>

namespace boost
{
namespace requests
{


namespace detail
{


struct fixed_source_body
{
  static std::size_t size(source & src)
  {
    return *src.size();
  }
  using value_type = source&;
  struct writer
  {
    source & src;
    char buf_[BOOST_REQUESTS_CHUNK_SIZE];
    using const_buffers_type = asio::const_buffer;

    template<bool isRequest, class Fields>
    explicit
    writer(beast::http::header<isRequest, Fields> const&,
           source & src) : src(src)
    {
    }

    void
    init(system::error_code& ec)
    {
      ec = {};
    }

    boost::optional<
        std::pair<const_buffers_type, bool>>
    get(system::error_code& ec)
    {
      const_buffers_type res;
      auto read_some = src.read_some(buf_, sizeof(buf_), ec);
      if (ec)
        return boost::none;

      return std::pair<const_buffers_type, bool>{const_buffers_type{buf_, read_some.first}, read_some.second};
    }
  };
};


struct source_body
{
  struct value_type
  {
    source &source;
    asio::const_buffer prefetched;
    bool more;
  };

  struct writer
  {
    value_type v;
    char buf_[BOOST_REQUESTS_CHUNK_SIZE];
    using const_buffers_type = asio::const_buffer;

    template<bool isRequest, class Fields>
    explicit
    writer(beast::http::header<isRequest, Fields> const&,
           value_type value) : v(std::move(value))
    {
    }

    void
    init(system::error_code& ec)
    {
      ec = {};
    }

    boost::optional<
        std::pair<const_buffers_type, bool>>
    get(system::error_code& ec)
    {
      if (v.prefetched.size() > 0)
        return std::pair<const_buffers_type, bool>{exchange(v.prefetched, asio::const_buffer("", 0u)), v.more};
      auto read_some = v.source.read_some(buf_, sizeof(buf_), ec);

      if (ec)
        return boost::none;
      return std::pair<const_buffers_type, bool>{const_buffers_type{buf_, read_some.first}, read_some.second};
    }
  };
};

}

template<typename Stream>
std::size_t write_request(
    Stream & stream,
    http::request_header hd,
    source &src,
    system::error_code & ec)
{
  const auto itr = hd.find(beast::http::field::content_type);
  if (itr == hd.end())
  {
    auto def = src.default_content_type();
    if (!def.empty())
      hd.set(beast::http::field::content_type, def);
  }
  if (auto sz = src.size())
  {
    if (*sz == 0)
    {
      http::request<http::empty_body> req(std::move(hd));
      req.prepare_payload();
      return beast::http::write(stream, req, ec);
    }
    else
    {
      http::request<detail::fixed_source_body> req(std::move(hd), src);
      req.prepare_payload();
      return beast::http::write(stream, req, ec);
    }
  }
  else
  {
    char prebuffer[BOOST_REQUESTS_CHUNK_SIZE];
    auto init = src.read_some(prebuffer, sizeof(prebuffer), ec);
    http::request<detail::source_body> req(std::move(hd),
                                           detail::source_body::value_type{src,
                                                                           asio::const_buffer(prebuffer, init.first),
                                                                           init.second});

    if (!init.second)
      req.set(beast::http::field::content_length, std::to_string(init.first));
    else
      req.prepare_payload();

    return beast::http::write(stream, req);
  }
}

namespace detail
{

template<typename Stream>
struct async_write_request_op : asio::coroutine
{
  using executor_type = typename Stream::executor_type;
  executor_type get_executor() const {return stream.get_executor();}

  using completion_signature_type = void(system::error_code, std::size_t);
  using step_signature_type       = void(system::error_code, std::size_t);


  Stream & stream;
  http::request_header hd;
  source &src;

  optional<std::size_t> sz = src.size();
  variant2::variant<variant2::monostate,
                    http::request<http::empty_body>,
                    http::request<detail::fixed_source_body>,
                    http::request<detail::source_body>> freq;

  char prebuffer[BOOST_REQUESTS_CHUNK_SIZE];
  std::pair<std::size_t, bool> init;

  async_write_request_op(Stream & stream, http::request_header hd, source &src)
    : stream(stream), hd(std::move(hd)), src(src)
  {}

  std::size_t resume(requests::detail::co_token_t<step_signature_type> self,
                     system::error_code ec = {}, std::size_t n = 0u)
  {
    BOOST_ASIO_CORO_REENTER(this)
    {
      {
        const auto itr = hd.find(beast::http::field::content_type);
        if (itr == hd.end())
        {
          auto def = src.default_content_type();
          if (!def.empty())
            hd.set(beast::http::field::content_type, def);
        }
      }
      if (sz)
      {
        if (*sz == 0)
        {
          freq.emplace<1>(std::move(hd)).prepare_payload();
          BOOST_ASIO_CORO_YIELD beast::http::async_write(stream, variant2::get<1>(freq), std::move(self));
        }
        else
        {
          freq.emplace<2>(std::move(hd), src).prepare_payload();
          BOOST_ASIO_CORO_YIELD beast::http::async_write(stream, variant2::get<2>(freq), std::move(self));
        }

      }
      else
      {
        init = src.read_some(prebuffer, sizeof(prebuffer), ec);
        if (ec)
          return 0u;

        freq.emplace<3>(std::move(hd),
                     detail::source_body::value_type{src,
                                                     asio::buffer(prebuffer, init.first),
                                                     init.second});

        if (!init.second)
          variant2::get<3>(freq).set(beast::http::field::content_length, std::to_string(init.first));
        else
          variant2::get<3>(freq).prepare_payload();

        BOOST_ASIO_CORO_YIELD beast::http::async_write(stream, variant2::get<3>(freq), std::move(self));
      }
    }
    return n;
  }
};


}

template<typename Stream,
    BOOST_ASIO_COMPLETION_TOKEN_FOR(void(system::error_code, std::size_t)) CompletionToken>
BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void(system::error_code, std::size_t))
async_write_request(
    Stream & stream,
    http::request_header hd,
    source &src,
    CompletionToken && token)
{
  return detail::co_run<detail::async_write_request_op<Stream>>(
      std::forward<CompletionToken>(token), std::ref(stream), std::move(hd), std::ref(src));
}


}
}

#endif //BOOST_REQUESTS_IMPL_SOURCE_HPP