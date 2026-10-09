#pragma once

#include <cstddef>
#include <optional>
#include <type_traits>
#include <vector>

namespace hgn::detail {
// 完全型の実体配列
//! HandlerはNativeの参照を持つことが期待されています
//! アクセスによる変更を配列へ波及させるためです
template <typename Native, typename Handler>
    requires(std::is_class_v<Native> or std::is_enum_v<Native>) and std::is_class_v<Handler> and
            std::is_constructible_v<Handler, Native&> and (std::is_move_constructible_v<Handler>)
class view_vector {
  public:
    explicit view_vector() noexcept = default;
    explicit view_vector(const std::size_t n) noexcept
        requires std::is_default_constructible_v<Native>
        : vec_(n) {}

    explicit view_vector(const std::size_t n, const Handler& handler) noexcept
        requires std::copy_constructible<Native> and requires {
            { handler.native() } noexcept -> std::convertible_to<Native>;
        }
        : vec_(n, handler.native()) {}

    explicit view_vector(std::initializer_list<Handler> handlers) noexcept
        requires std::copy_constructible<Native> and requires(Handler handler) {
            { handler.native() } noexcept -> std::convertible_to<Native>;
        }
    {
        for (auto&& handler : handlers) {
            vec_.emplace_back(handler.native());
        }
    }

    ~view_vector() noexcept = default;

    class iterator final {
      public:
        explicit iterator(std::vector<Native>::iterator it) noexcept : it_{it} {}

        [[nodiscard]] auto operator*() noexcept -> Handler
            requires(not std::is_pointer_v<Native>)
        {
            return Handler{*it_};
        }

        auto operator++() noexcept -> auto& {
            ++it_;
            return *this;
        }

        [[nodiscard]] auto operator==(const iterator& it) const noexcept -> bool {
            return it_ == it.it_;
        }

      private:
        std::vector<Native>::iterator it_;
    };

    [[nodiscard]] auto begin() noexcept -> auto { return iterator{vec_.begin()}; }
    [[nodiscard]] auto begin() const noexcept -> auto { return iterator{vec_.begin()}; }

    [[nodiscard]] auto end() noexcept -> auto { return iterator{vec_.end()}; }
    [[nodiscard]] auto end() const noexcept -> auto { return iterator{vec_.end()}; }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return vec_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> Handler {
        return Handler{vec_[i]};
    }

    [[nodiscard]] auto native_data() noexcept -> Native* { return vec_.data(); }
    [[nodiscard]] auto native_data() const noexcept -> const Native* { return vec_.data(); }

  protected:
    view_vector(const view_vector&) noexcept                    = default;
    auto operator=(const view_vector&) noexcept -> view_vector& = default;
    view_vector(view_vector&&) noexcept                         = default;
    auto operator=(view_vector&&) noexcept -> view_vector&      = default;

  private:
    std::vector<Native> vec_;
};

// 不完全型のポインタ配列
template <typename Native>
    requires std::is_pointer_v<Native>
class pointer_vector {
    using pointer    = Native;
    using value_type = std::remove_pointer_t<Native>;

  public:
    explicit pointer_vector() noexcept = default;
    explicit pointer_vector(const std::size_t n) noexcept : vec_(n) {}

    ~pointer_vector() noexcept = default;

    class iterator final {
      public:
        explicit iterator(std::vector<pointer>::iterator it) noexcept : it_{it} {}

        [[nodiscard]] auto operator*() noexcept -> std::optional<value_type&> {
            auto* ptr = *it_;
            if (ptr == nullptr) return std::nullopt;
            auto& out = *ptr;
            return out;
        }

        auto operator++() noexcept -> auto& {
            ++it_;
            return *this;
        }

        [[nodiscard]] auto operator==(const iterator& it) const noexcept -> bool {
            return it_ == it.it_;
        }

      private:
        std::vector<pointer>::iterator it_;
    };

    [[nodiscard]] auto begin() noexcept -> auto { return iterator{vec_.begin()}; }
    [[nodiscard]] auto begin() const noexcept -> auto { return iterator{vec_.begin()}; }

    [[nodiscard]] auto end() noexcept -> auto { return iterator{vec_.end()}; }
    [[nodiscard]] auto end() const noexcept -> auto { return iterator{vec_.end()}; }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return vec_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> std::optional<value_type&> {
        auto* ptr = vec_[i];
        if (vec_[i] == nullptr) return std::nullopt;
        auto& out = *ptr;
        return out;
    }

    [[nodiscard]] auto native_data() noexcept -> pointer* { return vec_.data(); }
    [[nodiscard]] auto native_data() const noexcept -> const pointer* { return vec_.data(); }

  protected:
    pointer_vector(const pointer_vector&) noexcept                    = default;
    auto operator=(const pointer_vector&) noexcept -> pointer_vector& = default;
    pointer_vector(pointer_vector&&) noexcept                         = default;
    auto operator=(pointer_vector&&) noexcept -> pointer_vector&      = default;

  private:
    std::vector<pointer> vec_;
};
}  // namespace hgn::detail

namespace hgn {
template <typename T>
class proxy_vector;
}