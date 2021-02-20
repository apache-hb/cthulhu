package com.apache.zag.err;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Deque;
import java.util.List;
import java.util.Stack;
import java.util.function.Consumer;

import com.apache.zag.ir.Node;
import com.apache.zag.ir.Root;

// parse tree validator
public class State {
    private class Env {
        Deque<List<Node>> nodes = new ArrayDeque<>();

        void scope(Consumer<Env> eat) {
            nodes.add(new ArrayList<>());
            eat.accept(this);
            nodes.pop();
        }

        // check if a name exists
        boolean lookup(String name) {
            for (List<Node> level : nodes) {
                if (level.stream().anyMatch(it -> it.equals(level))) {
                    return true;
                }
            }

            return false;
        }
    }

    public void validate(Root root) {
        Env env = new Env();

        env.scope(it -> {
            
        });
    }
}
