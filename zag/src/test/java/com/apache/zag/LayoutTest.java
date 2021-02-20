package com.apache.zag;

import java.util.List;

import com.apache.zag.ir.Alias;
import com.apache.zag.ir.Function;
import com.apache.zag.ir.Root;
import com.apache.zag.ir.Include;
import com.apache.zag.ir.Type.Scalar;
import com.apache.zag.ir.Type.Pointer;
import com.apache.zag.ir.Type.Struct;
import com.apache.zag.ir.Type.Union;
import com.apache.zag.ir.Type.Variable;
import com.apache.zag.ir.Function.Argument;
import com.apache.zag.ir.Stmt.Compound;
import com.apache.zag.ir.Stmt.Return;
import com.apache.zag.ir.Expr.Binary;
import com.apache.zag.ir.Expr.Int;
import com.apache.zag.ir.Expr.Name;
import com.apache.zag.ir.Expr.Op;

import org.junit.jupiter.api.Test;

public class LayoutTest {
    @Test
    void genTest() {
        Root root = new Root();

        root.includes.add(new Include("part", "another", "yes"));

        root.aliases.add(new Alias("name", new Pointer(new Scalar("uint8_t"))));
        root.aggregates.add(new Struct("type",
            new Variable("field", new Scalar("int")),
            new Variable("field2", new Pointer(new Scalar("void")))
        ));

        root.aggregates.add(new Union("thing",
            new Variable("field", new Scalar("bool")),
            new Variable("yes", new Pointer(new Pointer(new Scalar("void"))))
        ));

        root.functions.add(new Function("mul2", new Scalar("uint8_t"),
            List.of(new Argument("it", new Scalar("uint8_t"))),
            new Compound(new Return(new Binary(Op.MUL, new Int("2"), new Name("it"))))
        ));

        System.out.println(root.emit());
    }
}
