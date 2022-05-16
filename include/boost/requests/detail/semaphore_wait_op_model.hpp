//
// Copyright (c) 2021 Richard Hodges (hodges.r@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/madmongo1/asio_experiments
//

#ifndef BOOST_REQUESTS_DETAIL_SEMAPHORE_WAIT_OP_MODEL_HPP
#define BOOST_REQUESTS_DETAIL_SEMAPHORE_WAIT_OP_MODEL_HPP

#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_cancellation_slot.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <boost/requests/detail/semaphore_wait_op.hpp>
#include <boost/system/error_code.hpp>

namespace boost::requests
{
namespace detail
{
template < class Executor, class Handler >
struct semaphore_wait_op_model final : semaphore_wait_op
{
    using executor_type = Executor;
    using cancellation_slot_type =
        asio::associated_cancellation_slot_t< Handler >;
    using allocator_type = asio::associated_allocator_t< Handler >;

    allocator_type
    get_allocator()
    {
        return asio::get_associated_allocator(handler_);
    }

    cancellation_slot_type
    get_cancellation_slot()
    {
        return asio::get_associated_cancellation_slot(handler_);
    }

    executor_type
    get_executor()
    {
        return work_guard_.get_executor();
    }

    static semaphore_wait_op_model *
    construct(async_semaphore_base *host, Executor e, Handler handler);

    static void
    destroy(semaphore_wait_op_model *self);

    semaphore_wait_op_model(async_semaphore_base *host,
                            Executor              e,
                            Handler               handler);

    virtual void
    complete(system::error_code ec) override;

  private:
    struct cancellation_handler
    {
        semaphore_wait_op_model* self;

        void operator()(asio::cancellation_type type);
    };

  private:
    asio::executor_work_guard< Executor > work_guard_;
    Handler                               handler_;
};

}   // namespace detail
}   // namespace boost::requests

#endif

#include <boost/requests/detail/impl/semaphore_wait_op_model.hpp>