// Copyright (c) 2021 Klemens D. Morgenstern
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#ifndef BOOST_REQUESTS_POOL_HPP
#define BOOST_REQUESTS_POOL_HPP

#include <boost/requests/connection.hpp>
#include <boost/asem/st.hpp>
#include <list>
#include <boost/blank.hpp>
#include <boost/core/empty_value.hpp>

namespace boost {
namespace requests {

namespace detail {

struct endpoint_hash
{
  std::size_t operator()(const asio::generic::stream_protocol::endpoint & be) const
  {
    return hash_range(reinterpret_cast<const char*>(be.data()), // yuk
                      reinterpret_cast<const char*>(be.data()) + be.size());
  }
};


}

struct connection_pool
{
    /// The type of the executor associated with the object.
    typedef asio::any_io_executor executor_type;

    /// This type with a defaulted completion token.
    template<typename Token>
    struct defaulted;

    /// Rebinds the socket type to another executor.
    template<typename Executor1>
    struct rebind_executor
    {
        /// The socket type when rebound to the specified executor.
        using other = connection_pool;
    };

    /// Get the executor
    executor_type get_executor() noexcept
    {
        return mutex_.get_executor();
    }

    /// The protocol-type of the lowest layer.
    using protocol_type = asio::generic::stream_protocol;

    /// The endpoint of the lowest lowest layer.
    using endpoint_type = typename protocol_type::endpoint;

    /// Construct a stream.
    /**
     * @param exec The executor or execution_context.
     *
     * Everything else will be default constructed
     */
    explicit connection_pool(asio::any_io_executor exec,
                             std::size_t limit = BOOST_REQUESTS_DEFAULT_POOL_SIZE)
        : use_ssl_(false),
          context_(
              asio::use_service<detail::ssl_context_service>(
                  asio::query(exec, asio::execution::context)
              ).get()), mutex_(exec), limit_(limit) {}

    template<typename ExecutionContext>
    explicit connection_pool(ExecutionContext &context,
                             typename asio::constraint<
                                 asio::is_convertible<ExecutionContext &, asio::execution_context &>::value,
                                 std::size_t
                             >::type limit = BOOST_REQUESTS_DEFAULT_POOL_SIZE)
        : use_ssl_(false),
          context_(
              asio::use_service<detail::ssl_context_service>(context).get()),
              mutex_(context), limit_(limit) {}

    /// Construct a stream.
    /**
     * @param exec The executor or execution_context.
     *
     * Everything else will be default constructed
     */
    template<typename Exec>
    explicit connection_pool(Exec && exec,
                             asio::ssl::context & ctx,
                             std::size_t limit = BOOST_REQUESTS_DEFAULT_POOL_SIZE)
        : use_ssl_(true), context_(ctx), mutex_(std::forward<Exec>(exec)), limit_(limit) {}

    /// Move constructor
    connection_pool(connection_pool && ) = default;

    void lookup(urls::url_view av)
    {
      boost::system::error_code ec;
      lookup(av, ec);
      if (ec)
        urls::detail::throw_system_error(ec);
    }
    BOOST_REQUESTS_DECL void lookup(urls::url_view sv, system::error_code & ec);

    template<BOOST_ASIO_COMPLETION_TOKEN_FOR(void (boost::system::error_code)) CompletionToken>
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
                                       void (boost::system::error_code))
    async_lookup(urls::url_view av,
                 CompletionToken && completion_token );


    std::size_t limit() const {return limit_;}
    std::size_t active() const {return conns_.size();}

    using request_type = request_settings;

    BOOST_REQUESTS_DECL std::shared_ptr<connection> get_connection(error_code & ec);
    std::shared_ptr<connection> get_connection()
    {
      boost::system::error_code ec;
      auto res = get_connection(ec);
      if (ec)
        throw_exception(system::system_error(ec, "get_connection"));
      return res;
    }

    template<BOOST_ASIO_COMPLETION_TOKEN_FOR(void (system::error_code, std::shared_ptr<connection>)) CompletionToken>
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken, void (system::error_code, std::shared_ptr<connection_type>))
      async_get_connection(CompletionToken && completion_token);

    template<typename RequestBody>
    auto ropen(beast::http::verb method,
               urls::url_view path,
               RequestBody && body,
               request_settings req,
               system::error_code & ec) -> stream
    {
      auto conn = get_connection(ec);
      if (!ec && conn == nullptr)
        BOOST_REQUESTS_ASSIGN_EC(ec, asio::error::not_found);
      if (ec)
        return stream{get_executor(), nullptr};

      BOOST_ASSERT(conn != nullptr);
      return conn->ropen(method, path, std::forward<RequestBody>(body), std::move(req), ec);
    }

    template<typename RequestBody>
    auto ropen(beast::http::verb method,
               urls::url_view path,
               RequestBody && body,
               request_settings req) -> stream
    {
      boost::system::error_code ec;
      auto res = ropen(method, path, std::forward<RequestBody>(body), std::move(req), ec);
      if (ec)
        throw_exception(system::system_error(ec, "open"));
      return res;
    }

    auto ropen(beast::http::verb method,
               urls::pct_string_view path,
               http::fields & headers,
               source & src,
               request_options opt,
               cookie_jar * jar,
               system::error_code & ec) -> stream
    {
      auto conn = get_connection(ec);
      if (!ec && conn == nullptr)
        BOOST_REQUESTS_ASSIGN_EC(ec, asio::error::not_found);
      if (ec)
        return stream{get_executor(), nullptr};

      BOOST_ASSERT(conn != nullptr);
      return conn->ropen(method, path, headers, src, opt, jar, ec);
    }

    auto ropen(beast::http::verb method,
               urls::pct_string_view path,
               http::fields & headers,
               source & src,
               request_options opt,
               cookie_jar * jar) -> stream
    {
      boost::system::error_code ec;
      auto res = ropen(method, path, headers, src, opt, jar, ec);
      if (ec)
        throw_exception(system::system_error(ec, "open"));
      return res;
    }


    template<typename RequestBody,
             typename CompletionToken>
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
                                       void (boost::system::error_code, stream))
    async_ropen(beast::http::verb method,
                urls::url_view path,
                RequestBody && body,
                request_settings req,
                CompletionToken && completion_token);

    template<typename CompletionToken>
    BOOST_ASIO_INITFN_AUTO_RESULT_TYPE(CompletionToken,
                                       void (boost::system::error_code, stream))
    async_ropen(beast::http::verb method,
                urls::pct_string_view path,
                http::fields & headers,
                source & src,
                request_options opt,
                cookie_jar * jar,
                CompletionToken && completion_token);

    bool uses_ssl() const {return use_ssl_;}

  private:
    asio::ssl::context & context_;
    bool use_ssl_{false};
    asem::mt::mutex mutex_;
    std::string host_;
    std::vector<endpoint_type> endpoints_;
    std::size_t limit_;
    std::size_t connecting_{0u};

    boost::unordered_multimap<endpoint_type,
                              std::shared_ptr<connection>,
                              detail::endpoint_hash> conns_;

    struct async_lookup_op;
    struct async_get_connection_op;

    template<typename>
    struct async_ropen_op;
    struct async_ropen_op_src;
};

template<typename Token>
struct connection_pool::defaulted : connection_pool
{
  using default_token = Token;
  using connection_pool::connection_pool;

  auto async_lookup(urls::url_view av)
  {
    return connection_pool::async_lookup(av, default_token());
  }

  auto
  async_get_connection()
  {
    return connection_pool::async_get_connection(default_token());
  }
  using connection_pool::async_lookup;
  using connection_pool::async_get_connection;
  using connection_pool::async_ropen;


  template<typename RequestBody>
  auto async_ropen(beast::http::verb method,
                   urls::url_view path,
                   RequestBody && body,
                   request_settings req)
  {
    return connection_pool::async_ropen(method, path, std::forward<RequestBody>(body), std::move(req), default_token());
  }

  template<typename RequestBody>
  auto async_ropen(http::request<RequestBody> & req,
                   request_options opt,
                   cookie_jar * jar = nullptr)
  {
    return connection_pool::async_ropen(req, std::move(opt), jar, default_token());
  }

};


}
}

#include <boost/requests/impl/connection_pool.hpp>

#endif //BOOST_REQUESTS_POOL_HPP
