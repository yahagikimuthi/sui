#pragma once

#include <cstddef>
#include <type_traits>
#include <vector>

namespace hgn::detail {
template <typename Native, typename View>
    requires std::is_constructible_v<View, Native&>
class base_vector {
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

        [[nodiscard]] auto operator*() const noexcept -> View { return View{*it_}; }

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

    [[nodiscard]] auto size() const noexcept -> std::size_t;

    [[nodiscard]] auto operator[](const std::size_t i) noexcept -> View { return View{vec_[i]}; }

    [[nodiscard]] auto native_data() noexcept -> Native* { return vec_.data(); }
    [[nodiscard]] auto native_data() const noexcept -> const Native* { return vec_.data(); }

  protected:
    base_vector(const base_vector&) noexcept                    = default;
    auto operator=(const base_vector&) noexcept -> base_vector& = default;
    base_vector(base_vector&&) noexcept                         = default;
    auto operator=(base_vector&&) noexcept -> base_vector&      = default;

  private:
    std::vector<Native> vec_;
};
}  // namespace hgn::detail

namespace hgn {
template <typename T>
class vector;
}