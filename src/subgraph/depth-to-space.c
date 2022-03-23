// Copyright 2020 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <stddef.h>
#include <stdint.h>

#include <xnnpack.h>
#include <xnnpack/log.h>
#include <xnnpack/params.h>
#include <xnnpack/subgraph.h>


static enum xnn_status create_depth_to_space_operator(
  const struct xnn_node* node,
  const struct xnn_value* values,
  size_t num_values,
  struct xnn_operator_data* opdata,
  const struct xnn_caches* caches)
{
  assert(node->num_inputs == 1);
  const uint32_t input_id = node->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_values);

  assert(node->num_outputs == 1);
  const uint32_t output_id = node->outputs[0];
  assert(output_id != XNN_INVALID_VALUE_ID);
  assert(output_id < num_values);

  const size_t input_channel_dim = values[input_id].shape.dim[3];
  const size_t output_channel_dim = values[output_id].shape.dim[3];

  enum xnn_status status;
  if (values[input_id].layout == xnn_layout_type_nchw) {
    assert(values[output_id].layout == xnn_layout_type_nhwc);
    assert(node->compute_type == xnn_compute_type_fp32);
    status = xnn_create_depth_to_space_nchw2nhwc_x32(
        output_channel_dim /* output channels */,
        input_channel_dim /* input stride */,
        output_channel_dim /* output stride */,
        node->params.depth_to_space.block_size,
        node->flags,
        &opdata->operator_objects[0]);
  } else {
    assert(values[input_id].layout == xnn_layout_type_nhwc);
    assert(values[output_id].layout == xnn_layout_type_nhwc);
    switch (node->compute_type) {
#ifndef XNN_NO_F16_OPERATORS
      case xnn_compute_type_fp16:
        status = xnn_create_depth_to_space_nhwc_x16(
            output_channel_dim /* output channels */,
            input_channel_dim /* input stride */,
            output_channel_dim /* output stride */,
            node->params.depth_to_space.block_size,
            node->flags,
            &opdata->operator_objects[0]);
        break;
#endif  // XNN_NO_F16_OPERATORS
      case xnn_compute_type_fp32:
        status = xnn_create_depth_to_space_nhwc_x32(
            output_channel_dim /* output channels */,
            input_channel_dim /* input stride */,
            output_channel_dim /* output stride */,
            node->params.depth_to_space.block_size,
            node->flags,
            &opdata->operator_objects[0]);
        break;
#if !defined(XNN_NO_S8_OPERATORS) && !defined(XNN_NO_U8_OPERATORS)
      case xnn_compute_type_qs8:
      case xnn_compute_type_qu8:
        status = xnn_create_depth_to_space_nhwc_x8(
            output_channel_dim /* output channels */,
            input_channel_dim /* input stride */,
            output_channel_dim /* output stride */,
            node->params.depth_to_space.block_size,
            node->flags,
            &opdata->operator_objects[0]);
        break;
#endif  // !defined(XNN_NO_S8_OPERATORS) && !defined(XNN_NO_U8_OPERATORS)
      default:
        XNN_UNREACHABLE;
    }
  }
  if (status == xnn_status_success) {
    opdata->batch_size = values[input_id].shape.dim[0];
    opdata->input_height = values[input_id].shape.dim[1];
    opdata->input_width = values[input_id].shape.dim[2];
    opdata->output_height = values[output_id].shape.dim[1];
    opdata->output_width = values[output_id].shape.dim[2];
    opdata->inputs[0] = input_id;
    opdata->outputs[0] = output_id;
  }
  return status;
}

static enum xnn_status setup_depth_to_space_operator(
  const struct xnn_operator_data* opdata,
  const struct xnn_blob* blobs,
  size_t num_blobs,
  pthreadpool_t threadpool)
{
  const uint32_t input_id = opdata->inputs[0];
  assert(input_id != XNN_INVALID_VALUE_ID);
  assert(input_id < num_blobs);

  const uint32_t output_id = opdata->outputs[0];
  assert(output_id != XNN_INVALID_VALUE_ID);
  assert(output_id < num_blobs);

  const struct xnn_blob* input_blob = blobs + input_id;
  const void* input_data = input_blob->data;
  assert(input_data != NULL);

  const struct xnn_blob* output_blob = blobs + output_id;
  void* output_data = output_blob->data;
  assert(output_data != NULL);

  switch (opdata->operator_objects[0]->type) {
    case xnn_operator_type_depth_to_space_nchw2nhwc_x32:
      return xnn_setup_depth_to_space_nchw2nhwc_x32(
          opdata->operator_objects[0],
          opdata->batch_size,
          opdata->input_height,
          opdata->input_width,
          input_data,
          output_data,
          threadpool);
#ifndef XNN_NO_F16_OPERATORS
    case xnn_operator_type_depth_to_space_nhwc_x16:
      return xnn_setup_depth_to_space_nhwc_x16(
          opdata->operator_objects[0],
          opdata->batch_size,
          opdata->input_height,
          opdata->input_width,
          input_data,
          output_data,
          threadpool);
#endif  // XNN_NO_F16_OPERATORS
    case xnn_operator_type_depth_to_space_nhwc_x32:
      return xnn_setup_depth_to_space_nhwc_x32(
          opdata->operator_objects[0],
          opdata->batch_size,
          opdata->input_height,
          opdata->input_width,
          input_data,
          output_data,
          threadpool);
#if !defined(XNN_NO_S8_OPERATORS) && !defined(XNN_NO_U8_OPERATORS)
    case xnn_operator_type_depth_to_space_nhwc_x8:
      return xnn_setup_depth_to_space_nhwc_x8(
          opdata->operator_objects[0],
          opdata->batch_size,
          opdata->input_height,
          opdata->input_width,
          input_data,
          output_data,
          threadpool);
#endif  // !defined(XNN_NO_S8_OPERATORS) && !defined(XNN_NO_U8_OPERATORS)
    default:
      XNN_UNREACHABLE;
  }
}

enum xnn_status xnn_define_depth_to_space(
  xnn_subgraph_t subgraph,
  uint32_t input_id,
  uint32_t output_id,
  uint32_t block_size,
  uint32_t flags)
{
  if ((xnn_params.init_flags & XNN_INIT_FLAG_XNNPACK) == 0) {
    xnn_log_error("failed to define %s operator: XNNPACK is not initialized",
      xnn_node_type_to_string(xnn_node_type_depth_to_space));
    return xnn_status_uninitialized;
  }

  if (input_id >= subgraph->num_values) {
    xnn_log_error(
      "failed to define %s operator with input ID #%" PRIu32 ": invalid Value ID",
      xnn_node_type_to_string(xnn_node_type_depth_to_space), input_id);
    return xnn_status_invalid_parameter;
  }

  const struct xnn_value* input_value = &subgraph->values[input_id];
  if (input_value->type != xnn_value_type_dense_tensor) {
    xnn_log_error(
      "failed to define %s operator with input ID #%" PRIu32 ": unsupported Value type %d (expected dense tensor)",
      xnn_node_type_to_string(xnn_node_type_depth_to_space), input_id, input_value->type);
    return xnn_status_invalid_parameter;
  }

  switch (input_value->datatype) {
    case xnn_datatype_fp32:
#ifndef XNN_NO_QS8_OPERATORS
    case xnn_datatype_qint8:
#endif  // !defined(XNN_NO_QS8_OPERATORS)
#ifndef XNN_NO_QU8_OPERATORS
    case xnn_datatype_quint8:
#endif  // !defined(XNN_NO_QU8_OPERATORS)
      break;
    default:
      xnn_log_error(
        "failed to define %s operator with input ID #%" PRIu32 ": unsupported Value datatype %s (%d)",
        xnn_node_type_to_string(xnn_node_type_depth_to_space), input_id,
        xnn_datatype_to_string(input_value->datatype), input_value->datatype);
      return xnn_status_invalid_parameter;
  }

  if (output_id >= subgraph->num_values) {
    xnn_log_error(
      "failed to define %s operator with output ID #%" PRIu32 ": invalid Value ID",
      xnn_node_type_to_string(xnn_node_type_depth_to_space), output_id);
    return xnn_status_invalid_parameter;
  }

  const struct xnn_value* output_value = &subgraph->values[output_id];
  if (output_value->type != xnn_value_type_dense_tensor) {
    xnn_log_error(
      "failed to define %s operator with output ID #%" PRIu32 ": unsupported Value type %d (expected dense tensor)",
      xnn_node_type_to_string(xnn_node_type_depth_to_space), output_id, output_value->type);
    return xnn_status_invalid_parameter;
  }

  enum xnn_compute_type compute_type = xnn_compute_type_invalid;
  switch (output_value->datatype) {
    case xnn_datatype_fp32:
      compute_type = xnn_compute_type_fp32;
      break;
#ifndef XNN_NO_QS8_OPERATORS
    case xnn_datatype_qint8:
      compute_type = xnn_compute_type_qs8;
      break;
#endif  // !defined(XNN_NO_QS8_OPERATORS)
#ifndef XNN_NO_QU8_OPERATORS
    case xnn_datatype_quint8:
      compute_type = xnn_compute_type_qu8;
      break;
#endif  // !defined(XNN_NO_QU8_OPERATORS)
    default:
      xnn_log_error(
        "failed to define %s operator with output ID #%" PRIu32 ": unsupported Value datatype %s (%d)",
        xnn_node_type_to_string(xnn_node_type_depth_to_space), output_id,
        xnn_datatype_to_string(output_value->datatype), output_value->datatype);
      return xnn_status_invalid_parameter;
  }
  assert(compute_type != xnn_compute_type_invalid);

  if (input_value->datatype != output_value->datatype) {
    xnn_log_error(
      "failed to define %s operator with input ID #%" PRIu32 " and output ID #%" PRIu32
      ": mismatching datatypes across input (%s) and output (%s)",
      xnn_node_type_to_string(xnn_node_type_depth_to_space), input_id, output_id,
      xnn_datatype_to_string(input_value->datatype),
      xnn_datatype_to_string(output_value->datatype));
    return xnn_status_invalid_parameter;
  }

#if !defined(XNN_NO_QU8_OPERATORS) || !defined(XNN_NO_QS8_OPERATORS)
  if (compute_type == xnn_datatype_qint8 || compute_type == xnn_datatype_quint8) {
    if (input_value->quantization.zero_point != output_value->quantization.zero_point) {
      xnn_log_error(
        "failed to define %s operator with input ID #%" PRIu32 " and output ID #%" PRIu32
        ": mismatching zero point quantization parameter across input (%"PRId32") and output (%"PRId32")",
        xnn_node_type_to_string(xnn_node_type_depth_to_space), input_id, output_id,
        input_value->quantization.zero_point, output_value->quantization.zero_point);
      return xnn_status_invalid_parameter;
    }
    if (input_value->quantization.scale != output_value->quantization.scale) {
      xnn_log_error(
        "failed to define %s operator with input ID #%" PRIu32 " and output ID #%" PRIu32
        ": mismatching zero point quantization parameter across input (%.7g) and output (%.7g)",
        xnn_node_type_to_string(xnn_node_type_depth_to_space), input_id, output_id,
        input_value->quantization.scale, output_value->quantization.scale);
      return xnn_status_invalid_parameter;
    }
  }
#endif  // !defined(XNN_NO_QU8_OPERATORS) || !defined(XNN_NO_QS8_OPERATORS)

  if (block_size < 2) {
    xnn_log_error(
      "failed to define %s operator with block size #%" PRIu32 ": invalid block_size",
      xnn_node_type_to_string(xnn_node_type_depth_to_space), block_size);
    return xnn_status_invalid_parameter;
  }

  struct xnn_node* node = xnn_subgraph_new_node(subgraph);
  if (node == NULL) {
    return xnn_status_out_of_memory;
  }

  node->type = xnn_node_type_depth_to_space;
  node->compute_type = compute_type;
  node->num_inputs = 1;
  node->inputs[0] = input_id;
  node->num_outputs = 1;
  node->outputs[0] = output_id;
  node->params.depth_to_space.block_size = block_size;
  node->flags = flags;

  node->create = create_depth_to_space_operator;
  node->setup = setup_depth_to_space_operator;

  return xnn_status_success;
}
