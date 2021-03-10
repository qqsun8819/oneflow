#ifndef ONEFLOW_CORE_RPC_INCLUDE_BASE_CTRL_
#define ONEFLOW_CORE_RPC_INCLUDE_BASE_CTRL_

#include "oneflow/core/common/preprocessor.h"
#include "oneflow/core/common/util.h"
#include "oneflow/core/control/control.pb.h"

namespace oneflow {

#define CTRL_METHOD_SEQ               \
  OF_PP_MAKE_TUPLE_SEQ(LoadServer)    \
  OF_PP_MAKE_TUPLE_SEQ(Barrier)       \
  OF_PP_MAKE_TUPLE_SEQ(TryLock)       \
  OF_PP_MAKE_TUPLE_SEQ(NotifyDone)    \
  OF_PP_MAKE_TUPLE_SEQ(WaitUntilDone) \
  OF_PP_MAKE_TUPLE_SEQ(PushKV)        \
  OF_PP_MAKE_TUPLE_SEQ(ClearKV)       \
  OF_PP_MAKE_TUPLE_SEQ(PullKV)        \
  OF_PP_MAKE_TUPLE_SEQ(PushActEvent)  \
  OF_PP_MAKE_TUPLE_SEQ(Clear)         \
  OF_PP_MAKE_TUPLE_SEQ(IncreaseCount) \
  OF_PP_MAKE_TUPLE_SEQ(EraseCount)

#define CatRequest(method) method##Request,
#define CatReqponse(method) method##Response,
#define CatEnum(method) k##method,
#define CatName(method) "/oneflow.CtrlService/" OF_PP_STRINGIZE(method),

#define MAKE_META_DATA()                                                                       \
  enum class CtrlMethod { OF_PP_FOR_EACH_TUPLE(CatEnum, CTRL_METHOD_SEQ) };                    \
  static const char* g_method_name[] = {OF_PP_FOR_EACH_TUPLE(CatName, CTRL_METHOD_SEQ)};       \
  using CtrlRequestTuple = std::tuple<OF_PP_FOR_EACH_TUPLE(CatRequest, CTRL_METHOD_SEQ) void>; \
  using CtrlResponseTuple = std::tuple<OF_PP_FOR_EACH_TUPLE(CatReqponse, CTRL_METHOD_SEQ) void>;

MAKE_META_DATA()

constexpr const size_t kCtrlMethodNum = OF_PP_SEQ_SIZE(CTRL_METHOD_SEQ);

template<CtrlMethod ctrl_method>
using CtrlRequest =
    typename std::tuple_element<static_cast<size_t>(ctrl_method), CtrlRequestTuple>::type;

template<CtrlMethod ctrl_method>
using CtrlResponse =
    typename std::tuple_element<static_cast<size_t>(ctrl_method), CtrlResponseTuple>::type;

inline const char* GetMethodName(CtrlMethod method) {
  return g_method_name[static_cast<int32_t>(method)];
}

class CtrlCallIf {
 public:
  OF_DISALLOW_COPY_AND_MOVE(CtrlCallIf);
  virtual ~CtrlCallIf() = default;

  virtual void Process() = 0;
  virtual void SendResponse() = 0;

 protected:
  CtrlCallIf() = default;

 private:
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_RPC_INCLUDE_BASE_CTRL_