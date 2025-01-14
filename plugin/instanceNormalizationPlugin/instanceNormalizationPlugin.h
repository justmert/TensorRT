/*
 * Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TRT_INSTANCE_NORMALIZATION_PLUGIN_H
#define TRT_INSTANCE_NORMALIZATION_PLUGIN_H
#include "instanceNormFwd.h"
#include "plugin.h"
#include "serialize.hpp"
#include <cuda_fp16.h>
#include <cudnn.h>
#include <iostream>
#include <string>
#include <vector>

typedef unsigned short half_type;

namespace nvinfer1
{
namespace plugin
{
using namespace instance_norm_impl;
class InstanceNormalizationPlugin final : public nvinfer1::IPluginV2DynamicExt
{

public:
    InstanceNormalizationPlugin(
        float epsilon, nvinfer1::Weights const& scale, nvinfer1::Weights const& bias, int relu = 0, float alpha = 0.f);
    InstanceNormalizationPlugin(float epsilon, const std::vector<float>& scale, const std::vector<float>& bias,
        int relu = 0, float alpha = 0.f);
    InstanceNormalizationPlugin(void const* serialData, size_t serialLength);

    InstanceNormalizationPlugin() = delete;

    ~InstanceNormalizationPlugin() override;

    int getNbOutputs() const override;

    // DynamicExt plugins returns DimsExprs class instead of Dims
    using nvinfer1::IPluginV2::getOutputDimensions;
    DimsExprs getOutputDimensions(
        int outputIndex, const nvinfer1::DimsExprs* inputs, int nbInputs, nvinfer1::IExprBuilder& exprBuilder) override;

    int initialize() override;

    void terminate() override;

    using nvinfer1::IPluginV2::getWorkspaceSize;
    size_t getWorkspaceSize(const nvinfer1::PluginTensorDesc* inputs, int nbInputs,
        const nvinfer1::PluginTensorDesc* outputs, int nbOutputs) const override;

    using nvinfer1::IPluginV2::enqueue;
    int enqueue(const nvinfer1::PluginTensorDesc* inputDesc, const nvinfer1::PluginTensorDesc* outputDesc,
        const void* const* inputs, void* const* outputs, void* workspace, cudaStream_t stream) override;

    size_t getSerializationSize() const override;

    void serialize(void* buffer) const override;

    // DynamicExt plugin supportsFormat update.
    bool supportsFormatCombination(
        int pos, const nvinfer1::PluginTensorDesc* inOut, int nbInputs, int nbOutputs) override;

    const char* getPluginType() const override;

    const char* getPluginVersion() const override;

    void destroy() override;

    nvinfer1::IPluginV2DynamicExt* clone() const override;

    void setPluginNamespace(const char* pluginNamespace) override;

    const char* getPluginNamespace() const override;

    DataType getOutputDataType(int index, const nvinfer1::DataType* inputTypes, int nbInputs) const override;

    void attachToContext(cudnnContext* cudnn, cublasContext* cublas, nvinfer1::IGpuAllocator* allocator) override;

    void detachFromContext() override;

    using nvinfer1::IPluginV2Ext::configurePlugin;
    void configurePlugin(const nvinfer1::DynamicPluginTensorDesc* in, int nbInputs,
        const nvinfer1::DynamicPluginTensorDesc* out, int nbOutputs) override;

private:
    float mEpsilon;
    float mAlpha;
    int mRelu;
    int mNchan;
    std::vector<float> mHostScale;
    std::vector<float> mHostBias;
    float* mDeviceScale;
    float* mDeviceBias;
    size_t mDeviceBytes;
    cudnnHandle_t mCudnnHandle;
    cudnnTensorDescriptor_t mXDescriptor;
    cudnnTensorDescriptor_t mYDescriptor;
    cudnnTensorDescriptor_t mBDescriptor;
    std::string mPluginNamespace;
    std::string mNamespace;
    bool mInitialized{false};

    // NDHWC implementation
    InstanceNormFwdParams mParams;
    InstanceNormFwdContext mContext;

    float mInputScale;
    float mOutputScale;
};

class InstanceNormalizationPluginCreator : public BaseCreator
{
public:
    InstanceNormalizationPluginCreator();

    ~InstanceNormalizationPluginCreator() override = default;

    const char* getPluginName() const override;

    const char* getPluginVersion() const override;

    const PluginFieldCollection* getFieldNames() override;

    IPluginV2DynamicExt* createPlugin(const char* name, const nvinfer1::PluginFieldCollection* fc) override;

    IPluginV2DynamicExt* deserializePlugin(const char* name, const void* serialData, size_t serialLength) override;

private:
    static PluginFieldCollection mFC;
    static std::vector<PluginField> mPluginAttributes;
    std::string mNamespace;
};
} // namespace plugin
} // namespace nvinfer1

#endif // TRT_INSTANCE_NORMALIZATION_PLUGIN_H
