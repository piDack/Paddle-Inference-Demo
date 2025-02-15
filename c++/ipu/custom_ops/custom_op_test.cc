/* Copyright (c) 2022 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <numeric>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include "paddle/include/paddle_inference_api.h"

using paddle_infer::Config;
using paddle_infer::Predictor;
using paddle_infer::CreatePredictor;

void run(Predictor *predictor, const std::vector<float> &input,
         const std::vector<int> &input_shape, std::vector<float> *out_data) {
  auto input_names = predictor->GetInputNames();
  auto input_t = predictor->GetInputHandle(input_names[0]);
  input_t->Reshape(input_shape);
  input_t->CopyFromCpu(input.data());

  CHECK(predictor->Run());

  auto output_names = predictor->GetOutputNames();
  auto output_t = predictor->GetOutputHandle(output_names[0]);
  std::vector<int> output_shape = output_t->shape();
  int out_num = std::accumulate(output_shape.begin(), output_shape.end(), 1,
                                std::multiplies<int>());

  out_data->resize(out_num);
  output_t->CopyToCpu(out_data->data());
}

int main() {
  paddle::AnalysisConfig config;
  config.SetModel("./custom_relu_infer_model/custom_relu.pdmodel",
                  "./custom_relu_infer_model/custom_relu.pdiparams");
  config.EnableIpu();
  std::vector<std::vector<std::string>> custom_ops_info {
                {"custom_relu", "Relu", "custom.ops", "1"}};
  config.SetIpuCustomInfo(custom_ops_info);
  auto predictor{paddle_infer::CreatePredictor(config)};
  std::vector<int> input_shape = {1, 1, 28, 28};
  std::vector<float> input_data(1 * 1 * 28 * 28, 1);
  std::vector<float> out_data;
  run(predictor.get(), input_data, input_shape, &out_data);
  for (auto e : out_data) {
    LOG(INFO) << e << '\n';
  }
  return 0;
}
