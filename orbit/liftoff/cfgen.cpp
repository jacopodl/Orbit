// This source file is part of the Orbit project.
//
// Licensed under the Apache License v2.0

#include <orbit/liftoff/cfgen.h>

using namespace liftoff;
using namespace liftoff::parser;

ASTNode *CFGGen::visitASTNode(ASTNode *node) {
    // TODO: Implement ASTNode visitation
    return node;
}

ASTNode *CFGGen::visitAssignment(Assignment *node) {
    // TODO: Implement Assignment visitation
    return node;
}

ASTNode *CFGGen::visitBinary(Binary *node) {
    return node;
}

ASTNode *CFGGen::visitBlock(Block *node) {
    // TODO: Implement Block visitation
    return node;
}

ASTNode *CFGGen::visitBranch(Branch *node) {
    // TODO: Implement Branch visitation
    return node;
}

ASTNode *CFGGen::visitCall(Call *node) {
    // TODO: Implement Call visitation
    return node;
}

ASTNode *CFGGen::visitCatchBlock(CatchBlock *node) {
    // TODO: Implement CatchBlock visitation
    return node;
}

ASTNode *CFGGen::visitConstruct(Construct *node) {
    // TODO: Implement Construct visitation
    return node;
}

ASTNode *CFGGen::visitDecorator(Decorator *node) {
    // TODO: Implement Decorator visitation
    return node;
}

ASTNode *CFGGen::visitFunction(Function *node) {
    // TODO: Implement Function visitation
    return node;
}

ASTNode *CFGGen::visitIdentifier(Identifier *node) {
    // TODO: Implement Identifier visitation
    return node;
}

ASTNode *CFGGen::visitImport(Import *node) {
    // TODO: Implement Import visitation
    return node;
}

ASTNode *CFGGen::visitImportName(ImportName *node) {
    // TODO: Implement ImportName visitation
    return node;
}

ASTNode *CFGGen::visitJump(Jump *node) {
    // TODO: Implement Jump visitation
    return node;
}

ASTNode *CFGGen::visitLabel(Label *node) {
    // TODO: Implement Label visitation
    return node;
}

ASTNode *CFGGen::visitListExpression(ListExpression *node) {
    // TODO: Implement ListExpression visitation
    return node;
}

ASTNode *CFGGen::visitLiteral(Literal *node) {
    // TODO: Implement Literal visitation
    return node;
}

ASTNode *CFGGen::visitLoop(Loop *node) {
    // TODO: Implement Loop visitation
    return node;
}

ASTNode *CFGGen::visitModule(Module *node) {
    // TODO: Implement Module visitation
    return node;
}

ASTNode *CFGGen::visitNativeFunc(NativeFunc *node) {
    // TODO: Implement NativeFunc visitation
    return node;
}

ASTNode *CFGGen::visitNativeParameter(NativeParameter *node) {
    // TODO: Implement NativeParameter visitation
    return node;
}

ASTNode *CFGGen::visitNativeVariable(NativeVariable *node) {
    // TODO: Implement NativeVariable visitation
    return node;
}

ASTNode *CFGGen::visitParameter(Parameter *node) {
    // TODO: Implement Parameter visitation
    return node;
}

ASTNode *CFGGen::visitSubscript(Subscript *node) {
    // TODO: Implement Subscript visitation
    return node;
}

ASTNode *CFGGen::visitSwitchCase(SwitchCase *node) {
    // TODO: Implement SwitchCase visitation
    return node;
}

ASTNode *CFGGen::visitSwitchBlock(SwitchBlock *node) {
    // TODO: Implement SwitchBlock visitation
    return node;
}

ASTNode *CFGGen::visitTryBlock(TryBlock *node) {
    // TODO: Implement TryBlock visitation
    return node;
}

ASTNode *CFGGen::visitUnary(Unary *node) {
    // TODO: Implement Unary visitation
    return node;
}

void *CFGGen::BuildCFG(parser::ASTHandle<parser::Module *> &module) {
    return nullptr;
}
