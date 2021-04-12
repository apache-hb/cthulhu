#include "cthulhu.h"

namespace cthulhu::ast {
    void ArrayType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void RecordType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void AliasType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void SumType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void SentinelType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void PointerType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void ClosureType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void ScalarType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void BoolType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void VoidType::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void IntLiteral::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void Binary::visit(Visitor* visitor) {
        visitor->visit(this);
    }

    void Function::visit(Visitor* visitor) {
        visitor->visit(this);
    }
}
