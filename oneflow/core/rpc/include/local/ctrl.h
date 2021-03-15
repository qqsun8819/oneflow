/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#ifndef ONEFLOW_CORE_RPC_INCLUDE_LOCAL_CTRL_
#define ONEFLOW_CORE_RPC_INCLUDE_LOCAL_CTRL_

#include <string>
#include <unordered_map>
#include "oneflow/core/control/ctrl_bootstrap.pb.h"
#include "oneflow/core/rpc/include/local/rpc.h"
#include "oneflow/core/rpc/include/base/ctrl.h"

namespace oneflow {

class CtrlClient final : public RpcClient {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CtrlClient);
  CtrlClient() = default;
  ~CtrlClient();

 private:
  friend class Global<CtrlClient>;
  CtrlClient(const ProcessCtx& process_ctx);

  const ProcessCtx& process_ctx() const { return process_ctx_; }

  ProcessCtx process_ctx_;
};

#define FILE_LINE_STR __FILE__ ":" OF_PP_STRINGIZE(__LINE__)

#define OF_ENV_BARRIER() Global<CtrlClient>::Get()->Barrier(FILE_LINE_STR)
#define OF_SESSION_BARRIER()                        \
  Global<CtrlClient>::Get()->Barrier(FILE_LINE_STR, \
                                     Global<ResourceDesc, ForSession>::Get()->TotalMachineNum())

static void OfCallOnce(const std::string& name, std::function<void()> f) {
  TryLockResult lock_ret = Global<CtrlClient>::Get()->TryLock(name);
  if (lock_ret == TryLockResult::kLocked) {
    f();
    Global<CtrlClient>::Get()->NotifyDone(name);
  } else if (lock_ret == TryLockResult::kDone) {
  } else if (lock_ret == TryLockResult::kDoing) {
    Global<CtrlClient>::Get()->WaitUntilDone(name);
  } else {
    UNIMPLEMENTED();
  }
}

template<typename Self, typename F, typename Arg, typename... Args>
static void OfCallOnce(const std::string& name, Self self, F f, Arg&& arg, Args&&... args) {
  std::function<void()> fn =
      std::bind(f, self, std::forward<Arg>(arg), std::forward<Args>(args)...);
  OfCallOnce(name, std::move(fn));
}

template<typename Self, typename F>
static void OfCallOnce(const std::string& name, Self self, F f) {
  std::function<void()> fn = std::bind(f, self, name);
  OfCallOnce(name, std::move(fn));
}

template<typename F, typename Arg, typename... Args>
static void OfCallOnce(const std::string& name, F f, Arg&& arg, Args&&... args) {
  std::function<void()> fn = std::bind(f, std::forward<Arg>(arg), std::forward<Args>(args)...);
  OfCallOnce(name, std::move(fn));
}

template<CtrlMethod ctrl_method>
class CtrlCall final : public CtrlCallIf {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CtrlCall);
  CtrlCall() : status_(Status::kBeforeHandleRequest) {}
  ~CtrlCall() = default;

  static constexpr const size_t value = (size_t)ctrl_method;

  const CtrlRequest<ctrl_method>& request() const { return request_; }
  CtrlRequest<ctrl_method>* mut_request() { return &request_; }
  CtrlResponse<ctrl_method>* mut_response() { return &response_; }
  void set_request_handler(std::function<void()> val) { request_handler_ = val; }

  void Process() override {
    switch (status_) {
      case Status::kBeforeHandleRequest: {
        request_handler_();
        return;
      }
      case Status::kBeforeDelete: {
        delete this;
        return;
      }
    }
  }

  void SendResponse() override { status_ = Status::kBeforeDelete; }

 private:
  enum class Status { kBeforeHandleRequest, kBeforeDelete };

  Status status_;
  CtrlRequest<ctrl_method> request_;
  CtrlResponse<ctrl_method> response_;
  std::function<void()> request_handler_;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_RPC_INCLUDE_LOCAL_CTRL_