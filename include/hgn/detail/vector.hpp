#pragma once

#include <cstddef>
#include <optional>
#include <type_traits>
#include <variant>
#include <vector>

namespace hgn::detail {
template <typename Native, typename Handler = std::monostate>
class base_vector;

// 完全型の実体配列
template <typename Native, typename Handler>
    requires std::is_class_v<Native> and std::is_class_v<Handler> and
             std::is_default_constructible_v<Handler> and
             std::is_constructible_v<Handler, Native&> and
             (not std::same_as<Handler, std::monostate>) and (std::is_move_constructible_v<Handler>)
class base_vector<Native, Handler> {
  public:
    explicit base_vector() noexcept = default;
    explicit base_vector(const std::size_t n) noexcept
        requires std::is_default_constructible_v<Native>
    {
        vec_.resize(n);
    }

    ~base_vector() noexcept = default;

    class iterator final {
      public:
        explicit iterator(std::vector<Native>::iterator it) noexcept : it_{it} {}

        [[nodiscard]] auto operator*() noexcept -> Handler&
            requires(not std::is_pointer_v<Native>)
        {
            handler_  = Handler{*it_};
            auto& out = *handler_;
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
        std::vector<Native>::iterator it_;
        std::optional<Handler>        handler_;
    };

    [[nodiscard]] auto begin() noexcept -> auto { return iterator{vec_.begin()}; }
    [[nodiscard]] auto begin() const noexcept -> auto { return iterator{vec_.begin()}; }

    [[nodiscard]] auto end() noexcept -> auto { return iterator{vec_.end()}; }
    [[nodiscard]] auto end() const noexcept -> auto { return iterator{vec_.end()}; }

    [[nodiscard]] auto size() const noexcept -> std::size_t { return vec_.size(); }

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> Handler&
        requires(not std::is_pointer_v<Native>)
    {
        handler_ = Handler{vec_[i]};
        return *handler_;
    }

    [[nodiscard]] auto native_data() noexcept -> Native* { return vec_.data(); }
    [[nodiscard]] auto native_data() const noexcept -> const Native* { return vec_.data(); }

  protected:
    base_vector(const base_vector&) noexcept                    = default;
    auto operator=(const base_vector&) noexcept -> base_vector& = default;
    base_vector(base_vector&&) noexcept                         = default;
    auto operator=(base_vector&&) noexcept -> base_vector&      = default;

  private:
    std::vector<Native>    vec_;
    std::optional<Handler> handler_;
};

// 不完全型のポインタ配列
template <typename Native>
    requires std::is_class_v<Native>
class base_vector<Native*> {
  public:
    explicit base_vector() noexcept = default;
    explicit base_vector(const std::size_t n) noexcept { vec_.resize(n); }

    ~base_vector() noexcept = default;

    class iterator final {
      public:
        explicit iterator(std::vector<Native*>::iterator it) noexcept : it_{it} {}

        [[nodiscard]] auto operator*() noexcept -> std::optional<Native&>& {
            auto* ptr = *it_;
            if (ptr == nullptr) {
                native_.reset();
            } else {
                native_ = std::optional<Native&>(*ptr);
            }
            return native_;
        }

        auto operator++() noexcept -> auto& {
            ++it_;
            return *this;
        }

        [[nodiscard]] auto operator==(const iterator& it) const noexcept -> bool {
            return it_ == it.it_;
        }

      private:
        std::vector<Native*>::iterator it_;
        std::optional<Native&>         native_{std::nullopt};
    };

    [[nodiscard]] auto begin() noexcept -> auto { return iterator{vec_.begin()}; }
    [[nodiscard]] auto begin() const noexcept -> auto { return iterator{vec_.begin()}; }

    [[nodiscard]] auto end() noexcept -> auto { return iterator{vec_.end()}; }
    [[nodiscard]] auto end() const noexcept -> auto { return iterator{vec_.end()}; }

    [[nodiscard]] auto size() const noexcept -> std::size_t;

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> std::optional<Native&>& {
        auto* ptr = vec_[i];
        if (vec_[i] == nullptr) {
            native_.reset();
        } else {
            native_ = std::optional<Native&>{*ptr};
        }
        return native_;
    }

    [[nodiscard]] auto native_data() noexcept -> Native** { return vec_.data(); }
    [[nodiscard]] auto native_data() const noexcept -> const Native** { return vec_.data(); }

  protected:
    base_vector(const base_vector&) noexcept                    = default;
    auto operator=(const base_vector&) noexcept -> base_vector& = default;
    base_vector(base_vector&&) noexcept                         = default;
    auto operator=(base_vector&&) noexcept -> base_vector&      = default;

  private:
    std::vector<Native*>   vec_;
    std::optional<Native&> native_{std::nullopt};
};
}  // namespace hgn::detail

namespace hgn {
template <typename T>
class vector;
}