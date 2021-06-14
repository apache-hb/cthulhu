#include "llvm.h"
#include "debug.h"

#include "cthulhu/util/report.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#   include "llvm/IR/Module.h"
#   include "llvm/IR/IRBuilder.h"
#   include "llvm/Support/Host.h"
#   include "llvm/IR/LegacyPassManager.h"
#   include "llvm/Support/TargetRegistry.h"
#   include "llvm/Support/TargetSelect.h"
#   include "llvm/Target/TargetMachine.h"
#   include "llvm/Target/TargetOptions.h"
#pragma GCC diagnostic pop

using namespace llvm;

using LLVMBuilder = IRBuilder<>;

struct UnitContext;

struct FlowContext {
    FlowContext(UnitContext *ctx, flow_t *flow);

    UnitContext *parent;
    flow_t *flow;

    Function *function;

    std::map<size_t, Value*> vregs = { };

    std::map<size_t, BasicBlock*> blocks = { };

    BasicBlock *get_block(size_t idx) {
        return blocks[idx];
    }

    op_t *opcode(size_t idx) {
        return flow->ops + idx;
    }

    size_t len() const { return flow->len; }

    Value *get_value(operand_t op);
    Value *get_vreg(size_t idx) {
        return vregs[idx];
    }
};

struct UnitContext {
    UnitContext(unit_t *data) : unit(data) {
        builder = new LLVMBuilder(llvm);
        mod = new Module(data->name, llvm);
    }

    unit_t *unit;
    LLVMContext llvm;
    LLVMBuilder *builder;
    Module *mod;

    FlowContext get(size_t idx) {
        return FlowContext(this, unit->flows + idx);
    }

    size_t len() const { return unit->len; }
};

FlowContext::FlowContext(UnitContext *ctx, flow_t *flow) : parent(ctx), flow(flow) { 
    auto *type = FunctionType::get(Type::getInt64Ty(parent->llvm), false);
    function = Function::Create(type, Function::ExternalLinkage, flow->name, ctx->mod);

    auto *block = BasicBlock::Create(parent->llvm, "entry", function);
    parent->builder->SetInsertPoint(block);
}

static Value *get_imm(UnitContext *ctx, int64_t imm) {
    return ConstantInt::get(ctx->llvm, APInt(64, imm, true));
}

Value *FlowContext::get_value(operand_t op) {
    Value *out;
    
    switch (op.kind) {
    case IMM: 
        out = get_imm(parent, op.imm);
        break;
    case VREG: 
        out = get_vreg(op.vreg); 
        break;
    }

    return out;
}

template<typename F>
static void llvm_compile_binary(FlowContext *ctx, op_t *op, size_t idx, F&& func) {
    Value *lhs = ctx->get_value(op->lhs),
          *rhs = ctx->get_value(op->rhs);

    ctx->vregs[idx] = func(ctx->parent->builder, lhs, rhs);
}

template<typename F>
static void llvm_compile_unary(FlowContext *ctx, op_t *op, size_t idx, F&& func) {
    Value *val = ctx->get_value(op->expr);
    ctx->vregs[idx] = func(ctx->parent->builder, val);
}

static std::string new_label(size_t idx) {
    return "L_" + std::to_string(idx);
}

static void llvm_compile_block(FlowContext *ctx, size_t idx) {
    auto *parent = ctx->parent;

    ctx->blocks[idx] = BasicBlock::Create(parent->llvm, new_label(idx), ctx->function);
    parent->builder->SetInsertPoint(ctx->get_block(idx));
}

static void llvm_compile_phi(FlowContext *ctx, op_t *op, size_t idx) {
    auto *phi = ctx->parent->builder->CreatePHI(Type::getInt64Ty(ctx->parent->llvm), 2, new_label(idx));
    for (size_t i = 0; i < op->len; i++) {
        branch_t branch = op->branches[i];
        phi->addIncoming(
            ctx->get_value(branch.val), 
            ctx->get_block(branch.block)
        );
    }
    ctx->vregs[idx] = phi;
}

static void llvm_compile_cond(FlowContext *ctx, op_t *op) {
    ctx->parent->builder->CreateCondBr(
        ctx->get_value(op->cond), 
        ctx->get_block(op->block),
        ctx->get_block(op->other)
    );
}

static void llvm_compile_jmp(FlowContext *ctx, op_t *op) {
    ctx->parent->builder->CreateBr(
        ctx->get_block(op->label)
    );
}

static void llvm_compile_opcode(FlowContext *ctx, size_t idx) {
    op_t *op = ctx->opcode(idx);
    Value *expr;

    switch (op->kind) {
    case OP_VALUE: 
        expr = ctx->get_value(op->expr);
        ctx->vregs[idx] = expr;
        break;
    
    case OP_ADD:
        llvm_compile_binary(ctx, op, idx, [](auto *builder, auto *lhs, auto *rhs) {
            return builder->CreateAdd(lhs, rhs);
        });
        break;
    case OP_SUB:
        llvm_compile_binary(ctx, op, idx, [](auto *builder, auto *lhs, auto *rhs) {
            return builder->CreateSub(lhs, rhs);
        });
        break;
    case OP_DIV:
        llvm_compile_binary(ctx, op, idx, [](auto *builder, auto *lhs, auto *rhs) {
            return builder->CreateSDiv(lhs, rhs);
        });
        break;
    case OP_MUL:
        llvm_compile_binary(ctx, op, idx, [](auto *builder, auto *lhs, auto *rhs) {
            return builder->CreateMul(lhs, rhs);
        });
        break;
    case OP_REM:
        llvm_compile_binary(ctx, op, idx, [](auto *builder, auto *lhs, auto *rhs) {
            return builder->CreateSRem(lhs, rhs);
        });
        break;

    case OP_NEG:
        llvm_compile_unary(ctx, op, idx, [ctx](auto *builder, auto *val) {
            return builder->CreateMul(val, get_imm(ctx->parent, -1));
        });
        break;

    case OP_BLOCK:
        llvm_compile_block(ctx, idx);
        break;

    case OP_PHI:
        llvm_compile_phi(ctx, op, idx);
        break;

    case OP_COND:
        llvm_compile_cond(ctx, op);
        break;

    case OP_JMP:
        llvm_compile_jmp(ctx, op);
        break;

    case OP_RET:
        expr = ctx->get_value(op->expr);
        ctx->parent->builder->CreateRet(expr);
        break;

    default:
        reportf("llvm_compile_opcode(op->kind = %d)\n", op->kind);
        break;
    }
}

llvm_context *llvm_compile(unit_t *unit) {
    UnitContext *ctx = new UnitContext(unit);

    for (size_t f = 0; f < ctx->len(); f++) {
        FlowContext flow = ctx->get(f);

        for (size_t i = 0; i < flow.len(); i++) {
            llvm_compile_opcode(&flow, i);
        }
    }

    return ctx;
}

static raw_fd_ostream output(FILE *file) {
    return { fileno(file), false };
}

void llvm_output(llvm_context *ctx, FILE *file) {
    auto *self = (UnitContext*)ctx;

    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();

    auto triple = sys::getDefaultTargetTriple();

    std::string error;
    auto target = TargetRegistry::lookupTarget(triple, error);

    if (!target) {
        reportf("llvm failed to find target `%s`", error.c_str());
        return;
    }

    TargetOptions options;
    auto reloc = Optional<Reloc::Model>();
    auto machine = target->createTargetMachine(triple, "generic", "", options, reloc);

    self->mod->setDataLayout(machine->createDataLayout());
    self->mod->setTargetTriple(triple);

    legacy::PassManager passes;
    auto out = output(file);

    if (machine->addPassesToEmitFile(passes, out, nullptr, CGFT_ObjectFile)) {
        reportf("llvm failed to write output");
        return;
    }

    passes.run(*self->mod);
    out.flush();
}

void llvm_debug(llvm_context *ctx) {
    auto *self = (UnitContext*)ctx;
    auto out = output(get_debug());

    self->mod->print(out, nullptr);
}

bool llvm_enabled(void) {
    return true;
}
