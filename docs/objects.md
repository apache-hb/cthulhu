# Objects

```js
class File {
    new(self, path: str) {
        handle = sys::fopen(self.path, "rb");
    }

    delete(self) {
        sys::fclose(self.handle);
    }

    var handle: *void;
}

def start(): void {
    /* stack allocated resource, cleaned up when `with` statement ends */
    with file = File::new("stack.txt") {
        
    }

    if (ptr = sys::malloc(sizeof(File))) != null {
        var file = File::new(path) from ptr;

        delete file with sys::free;
    }
}
```

```ct
class Bump {
    new(self, size: usize) {
        self.data = sys::malloc(size);
    }

    delete(self) {
        sys::free(self.data);
    }

    def malloc(self, size: usize): *void {
        var out = self.data;
        self.data += size;
        return out;
    }

    def free(self, $: *void): void {
        /* no-op */
    }

    var data: *void;
}

def main(): void {
    var bump = Bump::new(0x1000);

    var file = File::new("file.txt") with bump.malloc(sizeof(File));

    delete file with bump.free;
}
```