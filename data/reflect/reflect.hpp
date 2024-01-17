#pragma once

#define CTU_CXX_REFLECT 1

namespace ctu {
    template<typename T, size_t N>
    class Storage {
    public:
        T data[N];
    };

    template<typename T, size_t N>
    class View {
        const T *data;

    public:
        constexpr T *begin() const noexcept { return data; }
        constexpr T *end() const noexcept { return data + N; }
    };

    class StructField {
        const char *name;

    public:
        consteval StructField(const char *name) noexcept
            : name(name)
        { }

        constexpr const char *get_name() const noexcept { return name; }
    };

    template<typename T, size_t N>
    class Struct {
    public:
        using storage_t = Storage<StructField, N>;
        using view_t = View<const StructField, N>;

    private:
        const char *name;
        storage_t fields;

    public:
        consteval Struct(const char *name, storage_t fields) noexcept
            : name(name)
            , fields(fields)
        { }

        constexpr const char *get_name() const noexcept { return name; }
        constexpr view_t get_fields() const noexcept { return fields; }
    };

    // customization point
    template<typename T>
    consteval auto reflect() noexcept;
}
