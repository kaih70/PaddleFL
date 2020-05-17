/* Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include "paddle/fluid/framework/op_registry.h"
#include "mpc_elementwise_add_op.h"

namespace paddle {
namespace operators {

using Tensor = framework::Tensor;

class MpcElementwiseAddOp : public framework::OperatorWithKernel {
public:
    using framework::OperatorWithKernel::OperatorWithKernel;

    void InferShape(framework::InferShapeContext* ctx) const override {
        PADDLE_ENFORCE_EQ(
            ctx->HasInput("X"), true,
            platform::errors::NotFound("Input(X) of MpcElementwiseAddOp should not be null."));
        PADDLE_ENFORCE_EQ(
            ctx->HasInput("Y"), true,
            platform::errors::NotFound("Input(Y) of MpcElementwiseAddOp should not be null."));
        PADDLE_ENFORCE_EQ(
            ctx->HasOutput("Out"), true,
            platform::errors::NotFound("Output(Out) of MpcElementwiseAddOp should not be null."));
        PADDLE_ENFORCE_GE(
            ctx->GetInputDim("X").size(), ctx->GetInputDim("Y").size(),
            platform::errors::InvalidArgument(
                "The dimensions of X should be greater than the dimensions of Y. "
                "But received the dimensions of X is [%s], the dimensions of Y is [%s]",
            ctx->GetInputDim("X"), ctx->GetInputDim("Y")));
        ctx->ShareDim("X", /*->*/ "Out");
        ctx->ShareLoD("X", /*->*/ "Out");
    }

};

class MpcElementwiseAddOpMaker : public framework::OpProtoAndCheckerMaker {
public:
    void Make() override {
        AddInput("X", "(Tensor), The first input tensor of mpc elementwise add op.");
        AddInput("Y", "(Tensor), The second input tensor of mpc elementwise add op.");
        AddOutput("Out", "(Tensor), The output tensor of mpc elementwise add op.");
        AddAttr<int>("axis",
                 "(int, default -1). If X.dimension != Y.dimension,"
                 "Y.dimension must be a subsequence of x.dimension. And axis "
                 "is the start dimension index "
                 "for broadcasting Y onto X. ")
        .SetDefault(-1)
        .EqualGreaterThan(-1);
        AddComment(R"DOC(
MPC elementwise add Operator.
)DOC");
    }
};

class MpcElementwiseAddGradOp : public framework::OperatorWithKernel {
public:
    using framework::OperatorWithKernel::OperatorWithKernel;
    using Tensor = framework::Tensor;
  
    void InferShape(framework::InferShapeContext *ctx) const override {
        auto out_grad_name = framework::GradVarName("Out");
        PADDLE_ENFORCE_EQ(ctx->HasInput("X"), true, "Input(X) should not be null.");
        PADDLE_ENFORCE_EQ(ctx->HasInput("Y"), true, "Input(Y) should not be null.");
        PADDLE_ENFORCE_EQ(ctx->HasInput(out_grad_name), true,
                      "Input(Out@GRAD) should not be null.");
        auto x_grad_name = framework::GradVarName("X");
        auto y_grad_name = framework::GradVarName("Y");
        if (ctx->HasOutput(x_grad_name)) {
            ctx->ShareDim("X", /*->*/ x_grad_name);
            ctx->ShareLoD("X", /*->*/ x_grad_name);
        }
        if (ctx->HasOutput(y_grad_name)) {
            ctx->ShareDim("Y", /*->*/ y_grad_name);
            ctx->ShareLoD("Y", /*->*/ y_grad_name);
        }
    }

};

template <typename T>
class MpcElementwiseAddOpGradMaker : public framework::SingleGradOpMaker<T> {
public:
    using framework::SingleGradOpMaker<T>::SingleGradOpMaker;

protected:
    void Apply(GradOpPtr<T> grad) const override {
        grad->SetType("mpc_elementwise_add_grad");
        grad->SetInput("X", this->Input("X"));
        grad->SetInput("Y", this->Input("Y"));
        grad->SetInput(framework::GradVarName("Out"), this->OutputGrad("Out"));
        grad->SetOutput(framework::GradVarName("X"), this->InputGrad("X"));
        grad->SetOutput(framework::GradVarName("Y"), this->InputGrad("Y"));
        grad->SetAttrMap(this->Attrs());
    }
};

}  // namespace operators
}  // namespace paddle

namespace ops = paddle::operators;
REGISTER_OPERATOR(mpc_elementwise_add, ops::MpcElementwiseAddOp, 
                 ops::MpcElementwiseAddOpMaker, 
                 ops::MpcElementwiseAddOpGradMaker<paddle::framework::OpDesc>); 

REGISTER_OPERATOR(mpc_elementwise_add_grad, ops::MpcElementwiseAddGradOp); 

REGISTER_OP_CPU_KERNEL(
    mpc_elementwise_add, 
    ops::MpcElementwiseAddKernel<paddle::platform::CPUDeviceContext, int64_t>);

REGISTER_OP_CPU_KERNEL(
    mpc_elementwise_add_grad, 
    ops::MpcElementwiseAddGradKernel<paddle::platform::CPUDeviceContext, int64_t>);
