#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace hgn::experimental {
template <typename T>
    requires std::is_class_v<T> or std::is_enum_v<T>
class wrapper {
  public:
    [[nodiscard]] auto native() const noexcept -> const T& { return native_; }

    [[nodiscard]] auto native() noexcept -> T& { return native_; }

  protected:
    explicit wrapper(T&& native) : native_{std::move(native)} {}
    explicit wrapper(const T& native) : native_{native} {}

  private:
    T native_;
};

template <typename T>
    requires std::is_class_v<std::remove_const_t<T>> and
             std::is_class_v<std::remove_cvref_t<decltype(std::declval<T>().native())>>
class view final {
    using native_type = std::remove_cvref_t<decltype(std::declval<T>().native())>;
    using reference   = std::conditional_t<std::is_const_v<T>, const native_type&, native_type&>;

  public:
    view(const T& wrapper) noexcept : native_ref_{wrapper.native()} {}

    template <typename U>
        requires(std::conditional_t<
                 std::is_const_v<T>,
                 std::disjunction<std::is_same<T, U>, std::is_same<std::remove_const_t<T>, U>>,
                 std::is_same<T, U>>::value)
    view(const view<U> other) noexcept : native_ref_{other.native()} {}

    [[nodiscard]] auto native() const noexcept -> reference { return native_ref_; }

  private:
    reference native_ref_;
};

template <typename Native>
auto extract_native(const wrapper<Native>&) noexcept -> Native;

template <typename Wrapper>
    requires(std::is_class_v<Wrapper>)  // 非const
class proxy_vector {
  public:
    using Native = decltype(extract_native(std::declval<Wrapper>()));
    [[nodiscard]] auto size() const noexcept -> std::size_t { return vec_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> view<Wrapper> {
        return view<Wrapper>{vec_[i]};
    }

    [[nodiscard]] auto operator[](const std::size_t i) const noexcept -> view<const Wrapper> {
        return view<const Wrapper>{vec_[i]};
    }

    [[nodiscard]] auto push_back(const view<const Wrapper> element) noexcept {
        vec_.emplace_back(element.native());
    }

  private:
    std::vector<Native> vec_;
};

template <typename T>
    requires std::is_class_v<T>
class proxy_vector<std::optional<T&>> {
    using value_type = T;
    using pointer    = T*;

  public:
    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> std::optional<T&> {
        auto* ptr = vec_[i];
        if (ptr == nullptr) return std::nullopt;
        auto& out = *ptr;
        return out;
    }

  private:
    std::vector<pointer> vec_;
};
}  // namespace hgn::experimental