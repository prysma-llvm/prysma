#ifndef VISITEUR_DESTRUCTION_H
#define VISITEUR_DESTRUCTION_H

#include "compiler/visitor/visitor_base_generale.h"
#include "compiler/ast/ast_genere.h"

class VisitorDestruction : public VisitorBaseGenerale
{
public:


public:
    void visiter(NodeCallFunction* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeCallFunction();
    }
    void visiter(NodeArgFunction* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeArgFunction();
    }
    void visiter(NodeDeclarationFunction* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeDeclarationFunction();
    }
    void visiter(NodeReturn* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeReturn();
    }
    void visiter(NodeAssignmentVariable* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeAssignmentVariable();
    }
    void visiter(NodeDeclarationVariable* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeDeclarationVariable();
    }
    void visiter(NodeRefVariable* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeRefVariable();
    }
    void visiter(NodeUnRefVariable* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeUnRefVariable();
    }
    void visiter(NodeIdentifiant* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeIdentifiant();
    }
    void visiter(NodeAssignmentArray* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeAssignmentArray();
    }
    void visiter(NodeArrayInitialization* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeArrayInitialization();
    }
    void visiter(NodeReadingArray* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeReadingArray();
    }
    void visiter(NodeClass* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeClass();
    }
    void visiter(NodeCallObject* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeCallObject();
    }
    void visiter(NodeAccesAttribute* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeAccesAttribute();
    }
    void visiter(NodeDeclarationObject* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeDeclarationObject();
    }
    void visiter(NodeIf* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeIf();
    }
    void visiter(NodeWhile* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeWhile();
    }
    void visiter(NodeNew* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeNew();
    }
    void visiter(NodeDelete* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeDelete();
    }
    void visiter(NodeInclude* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeInclude();
    }
    void visiter(NodeOperation* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeOperation();
    }
    void visiter(NodeLiteral* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeLiteral();
    }
    void visiter(NodeNegation* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeNegation();
    }
    void visiter(NodeString* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeString();
    }
    void visiter(NodeInstruction* node) override {
        VisitorBaseGenerale::visiter(node);
        node->~NodeInstruction();
    }
};

#endif