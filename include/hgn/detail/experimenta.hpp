#pragma once

#include <vulkan/vulkan.h>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace hgn::detail {
template <typename T>
    requires std::is_class_v<T> or std::is_enum_v<T>
class wrapper {
  public:
    [[nodiscard]] auto native() const noexcept -> const T& { return native_; }

    [[nodiscard]] auto native() noexcept -> T& { return native_; }

  protected:
    explicit wrapper(T&& native) noexcept : native_{std::move(native)} {}
    explicit wrapper(const T& native) noexcept : native_{native} {}

  private:
    T native_;
};

template <typename Native>
auto extract_native(const wrapper<Native>&) noexcept -> Native;
}  // namespace hgn::detail

namespace hgn {
template <typename T>
class view;
}

namespace hgn::detail {
template <typename Wrapper>
class base_view;

template <typename Wrapper>
    requires(
        std::is_class_v<std::remove_const_t<Wrapper>> and
        (std::is_class_v<std::remove_cvref_t<decltype(std::declval<Wrapper>().native())>> or
         std::is_enum_v<std::remove_cvref_t<decltype(std::declval<Wrapper>().native())>>)
    )
class base_view<Wrapper> {
    using native_type = std::remove_cvref_t<decltype(std::declval<Wrapper>().native())>;
    using reference =
        std::conditional_t<std::is_const_v<Wrapper>, const native_type&, native_type&>;

  public:
    base_view(const Wrapper& wrapper) noexcept : native_ref_{wrapper.native()} {}

    template <typename U>
        requires(std::conditional_t<
                 std::is_const_v<Wrapper>,
                 std::disjunction<
                     std::is_same<Wrapper, U>,
                     std::is_same<std::remove_const_t<Wrapper>, U>>,
                 std::is_same<Wrapper, U>>::value)
    base_view(const view<U>& other) noexcept : native_ref_{other.native()} {}

    [[nodiscard]] auto native() const noexcept -> reference { return native_ref_; }

  private:
    reference native_ref_;
};

template <typename EnumClass>
    requires std::is_scoped_enum_v<EnumClass>
class base_view<EnumClass> {
  public:
  private:
};
}  // namespace hgn::detail

namespace hgn {
template <typename Wrapper>
class view final : public detail::base_view<Wrapper> {
  public:
    using detail::base_view<Wrapper>::base_view;
};

template <typename Wrapper>
    requires(std::is_class_v<Wrapper>)  // 非const
class proxy_vector final {
    using Native = decltype(extract_native(std::declval<Wrapper>()));

  public:
    explicit proxy_vector() noexcept = default;
    explicit proxy_vector(const std::size_t n) noexcept
        requires std::is_default_constructible_v<Native>
        : vec_(n) {}

    class iterator {
      public:
        explicit iterator(std::vector<Native>::iterator it) noexcept : it_{it} {}

        auto operator++() noexcept -> iterator& {
            ++it_;
            return *this;
        }

        [[nodiscard]] auto operator==(const iterator& other) const noexcept -> bool = default;

        [[nodiscard]] auto operator*() const noexcept -> view<Wrapper> {
            return view<Wrapper>{*it_};
        }

        [[nodiscard]] auto operator*() noexcept -> view<Wrapper> { return view<Wrapper>{*it_}; }

      private:
        std::vector<Native>::iterator it_;
    };

    [[nodiscard]] auto size() const noexcept -> std::size_t { return vec_.size(); }

    [[nodiscard]] auto begin() const noexcept -> iterator { return iterator{vec_.begin()}; }
    [[nodiscard]] auto begin() noexcept -> iterator { return iterator{vec_.begin()}; }

    [[nodiscard]] auto end() const noexcept -> iterator { return iterator{vec_.end()}; }
    [[nodiscard]] auto end() noexcept -> iterator { return iterator{vec_.end()}; }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> view<Wrapper> {
        return view<Wrapper>{vec_[i]};
    }

    [[nodiscard]] auto operator[](const std::size_t i) const noexcept -> view<const Wrapper> {
        return view<const Wrapper>{vec_[i]};
    }

    [[nodiscard]] auto push_back(const view<const Wrapper> element) noexcept {
        vec_.emplace_back(element.native());
    }

    [[nodiscard]] auto native_data() noexcept -> Native* { return vec_.data(); }

    [[nodiscard]] auto native_data() const noexcept -> const Native* { return vec_.data(); }

  private:
    std::vector<Native> vec_;
};

template <typename T>
    requires std::is_class_v<T>
class proxy_vector<std::optional<T&>> final {
    using value_type = T;
    using pointer    = T*;

  public:
    explicit proxy_vector(const std::size_t n) noexcept : vec_(n) {}

    class iterator final {
      public:
        explicit iterator(std::vector<pointer>::iterator it) noexcept : it_{it} {}

        auto operator++() noexcept -> iterator& {
            ++it_;
            return *this;
        }

        [[nodiscard]] auto operator==(const iterator&) const noexcept -> bool = default;

        [[nodiscard]] auto operator*() const noexcept -> std::optional<const T&> {
            const auto* ptr = *it_;
            if (ptr == nullptr) return std::nullopt;
            const auto& out = *ptr;
            return out;
        }

        [[nodiscard]] auto operator*() noexcept -> std::optional<T&> {
            auto* ptr = *it_;
            if (ptr == nullptr) return std::nullopt;
            auto& out = *ptr;
            return out;
        }

      private:
        std::vector<pointer>::iterator it_;
    };

    [[nodiscard]] auto size() const noexcept -> std::size_t { return vec_.size(); }

    [[nodiscard]] auto begin() const noexcept -> iterator { return iterator{vec_.begin()}; }
    [[nodiscard]] auto begin() noexcept -> iterator { return iterator{vec_.begin()}; }

    [[nodiscard]] auto end() const noexcept -> iterator { return iterator{vec_.end()}; }
    [[nodiscard]] auto end() noexcept -> iterator { return iterator{vec_.end()}; }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> std::optional<T&> {
        auto* ptr = vec_[i];
        if (ptr == nullptr) return std::nullopt;
        auto& out = *ptr;
        return out;
    }

    [[nodiscard]] auto operator[](const std::size_t i) const noexcept -> std::optional<const T&> {
        const auto* ptr = vec_[i];
        if (ptr == nullptr) return std::nullopt;
        const auto& out = *ptr;
        return out;
    }

    [[nodiscard]] auto native_data() noexcept -> pointer* { return vec_; }

  private:
    std::vector<pointer> vec_;
};
}  // namespace hgn