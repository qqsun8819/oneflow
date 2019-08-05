#include <google/protobuf/text_format.h>
#include "oneflow/core/common/buffer_manager.h"
#include "oneflow/core/common/protobuf.h"
#include "oneflow/core/register/ofblob.h"
#include "oneflow/core/job/job_set.pb.h"
#include "oneflow/core/job/oneflow.h"
#include "oneflow/core/job/foreign_job_instance.h"
#include "oneflow/core/job/environment_objects_scope.h"
#include "oneflow/core/job/machine_context.h"
#include "oneflow/core/job/oneflow.h"
#include "oneflow/core/control/cluster_control.pb.h"
#include "oneflow/core/control/ctrl_client.h"
#include "oneflow/core/job/runtime_buffer_managers_scope.h"
#include "oneflow/core/job/runtime_buffers_scope.h"
#include "oneflow/core/control/cluster_control.h"
#include "oneflow/core/job/job_set_compile_ctx.h"

bool IsOpTypeCaseCpuSupportOnly(int64_t op_type_case) {
  using namespace oneflow;
  using OnlyCpuSupport = OnlyCpuSupportPredicator;
  CHECK(IsClassRegistered<OnlyCpuSupport>(op_type_case)) << ": op_type_case = " << op_type_case;
  return *std::unique_ptr<OnlyCpuSupport>(NewObj<OnlyCpuSupport>(op_type_case));
}

void InitBySerializedConfigProto(const std::string& config_proto_str) {
  using namespace oneflow;
  ConfigProto config_proto;
  CHECK(google::protobuf::TextFormat::ParseFromString(config_proto_str, &config_proto));
  CHECK_ISNULL(Global<EnvironmentObjectsScope>::Get());
  // Global<T>::New is not allowed to be called here
  // because glog is not constructed yet and LOG(INFO) has bad bahavior
  Global<EnvironmentObjectsScope>::SetAllocated(new EnvironmentObjectsScope(config_proto));
  LOG(INFO) << "NewGlobal " << typeid(EnvironmentObjectsScope).name();
  if (Global<MachineCtx>::Get()->IsThisMachineMaster()) {
    Global<CtrlClient>::Get()->PushKV("master_config_proto", config_proto);
  } else {
    ConfigProto master_config_proto;
    Global<CtrlClient>::Get()->PullKV("master_config_proto", &master_config_proto);
    CHECK(PbMd().Equals(config_proto, master_config_proto));

    while (true) {
      ClusterControl::WorkerSendAckAndExitIfReceiveHalt();
      JobSet job_set;
      Global<CtrlClient>::Get()->PullKV("session_job_set", &job_set);
      { Oneflow oneflow(job_set); }
    }
  }
}

void InitGlobalOneflowBySerializedJobSet(const std::string& job_set_str) {
  using namespace oneflow;
  CHECK(Global<MachineCtx>::Get()->IsThisMachineMaster());
  ClusterControl::MasterSendSessionStart();
  JobSet job_set;
  CHECK(google::protobuf::TextFormat::ParseFromString(job_set_str, &job_set));
  CHECK_ISNULL(Global<Oneflow>::Get());
  Global<CtrlClient>::Get()->PushKV("session_job_set", job_set);
  Global<RuntimeBufferManagersScope>::New();
  Global<JobSetCompileCtx>::New();
  Global<Oneflow>::New(job_set);
  Global<RuntimeBuffersScope>::New();
}

std::string GetSerializedInterUserJobInfo() {
  using namespace oneflow;
  CHECK(Global<MachineCtx>::Get()->IsThisMachineMaster());
  CHECK_NOTNULL(Global<Oneflow>::Get());
  CHECK_NOTNULL(Global<InterUserJobInfo>::Get());
  std::string ret;
  google::protobuf::TextFormat::PrintToString(*Global<InterUserJobInfo>::Get(), &ret);
  return ret;
}

void LaunchJob(const std::shared_ptr<oneflow::ForeignJobInstance>& cb) {
  using namespace oneflow;
  CHECK(Global<MachineCtx>::Get()->IsThisMachineMaster());
  CHECK_NOTNULL(Global<Oneflow>::Get());
  const auto& job_name = cb->job_name();
  auto* buffer_mgr = Global<BufferMgr<std::shared_ptr<ForeignJobInstance>>>::Get();
  int64_t job_id = Global<JobName2JobId>::Get()->at(job_name);
  const JobDesc& job_desc = GlobalJobDesc(job_id);
  if (job_desc.is_pull_job()) { buffer_mgr->Get(GetForeignOutputBufferName(job_name))->Send(cb); }
  if (job_desc.is_push_job()) { buffer_mgr->Get(GetForeignInputBufferName(job_name))->Send(cb); }
  buffer_mgr->Get(GetCallbackNotifierBufferName(job_name))->Send(cb);
  Global<BufferMgr<int64_t>>::Get()->Get(kBufferNameGlobalWaitJobId)->Send(job_id);
}

void DestroyGlobalOneflow() {
  using namespace oneflow;
  CHECK(Global<MachineCtx>::Get()->IsThisMachineMaster());
  CHECK_NOTNULL(Global<RuntimeBuffersScope>::Get());
  Global<RuntimeBuffersScope>::Delete();
  CHECK_NOTNULL(Global<Oneflow>::Get());
  Global<Oneflow>::Delete();
  Global<JobSetCompileCtx>::Delete();
  Global<RuntimeBufferManagersScope>::Delete();
}

void DestroyGlobalEnvironment() {
  using namespace oneflow;
  if (Global<EnvironmentObjectsScope>::Get() == nullptr) { return; }
  bool is_master = Global<MachineCtx>::Get()->IsThisMachineMaster();
  if (is_master) { ClusterControl::MasterSendHaltAndWaitAck(); }
  Global<EnvironmentObjectsScope>::Delete();
}

int Ofblob_GetDataType(uint64_t of_blob_ptr) {
  using namespace oneflow;
  auto* of_blob = reinterpret_cast<OfBlob*>(of_blob_ptr);
  return of_blob->data_type();
}

size_t OfBlob_NumAxes(uint64_t of_blob_ptr) {
  using namespace oneflow;
  auto* of_blob = reinterpret_cast<OfBlob*>(of_blob_ptr);
  return of_blob->NumAxes();
}

void OfBlob_CopyShapeToNumpy(uint64_t of_blob_ptr, int64_t* array, int size) {
  using namespace oneflow;
  auto* of_blob = reinterpret_cast<OfBlob*>(of_blob_ptr);
  return of_blob->CopyShapeTo(array, size);
};

namespace oneflow {

namespace {

struct GlobalChecker final {
  GlobalChecker() = default;
  ~GlobalChecker() {
    if (Global<Oneflow>::Get() != nullptr) { LOG(FATAL) << "global oneflow is not destroyed yet"; }
    if (Global<EnvironmentObjectsScope>::Get() != nullptr) {
      LOG(FATAL) << "global environment is not destroyed yet";
    }
  }
};

GlobalChecker checker;

}  // namespace

}  // namespace oneflow
