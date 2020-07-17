from __future__ import absolute_import

import operator
from functools import reduce

import oneflow as flow
import oneflow.core.operator.op_conf_pb2 as op_conf_util
import oneflow.core.register.logical_blob_id_pb2 as logical_blob_id_util
import oneflow.python.framework.interpret_util as interpret_util
import oneflow.python.framework.distribute as distribute_util
import oneflow.python.framework.id_util as id_util
import oneflow.python.framework.input_blob_def as input_blob_util
import oneflow.python.framework.remote_blob as remote_blob_util
from oneflow.python.oneflow_export import oneflow_export
from typing import Optional


@oneflow_export("experimental.square_sum")
def square_sum(
    x: input_blob_util.ArgBlobDef, name: Optional[str] = None
) -> remote_blob_util.BlobDef:
    op_conf = op_conf_util.OperatorConf()
    if name is None:
        op_conf.name = id_util.UniqueStr("SquareSum_")
    else:
        op_conf.name = name

    op_conf.square_sum_conf.x = x.unique_name
    op_conf.square_sum_conf.y = "y"

    interpret_util.Forward(op_conf)
    lbi = logical_blob_id_util.LogicalBlobId()
    lbi.op_name = op_conf.name
    lbi.blob_name = "y"
    return remote_blob_util.RemoteBlob(lbi)
