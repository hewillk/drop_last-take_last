#include <ranges>

namespace std::ranges {
template<view V>
  requires forward_range<V> || sized_range<V>
class drop_last_view : public view_interface<drop_last_view<V>> {
 private:
  V base_ = V();                     // exposition only
  range_difference_t<V> count_ = 0;  // exposition only

  class iterator {
   private:
    iterator_t<V> current_ = iterator_t<V>();  // exposition only
    iterator_t<V> probe_ = iterator_t<V>();    // exposition only
    constexpr iterator(iterator_t<V> current, iterator_t<V> probe)
      : current_(current), probe_(probe) { }  // exposition only

   public:
    using iterator_concept = decltype([] {
      if constexpr (contiguous_range<V>)
        return contiguous_iterator_tag{};
      else if constexpr (random_access_range<V>)
        return random_access_iterator_tag{};
      else if constexpr (bidirectional_range<V>)
        return bidirectional_iterator_tag{};
      else
        return forward_iterator_tag{};
    }());
    using iterator_category = decltype([] {
      using C = iterator_traits<iterator_t<V>>::iterator_category;
      if constexpr (derived_from<C, random_access_iterator_tag>)
        return random_access_iterator_tag{};
      else if constexpr (derived_from<C, bidirectional_iterator_tag>)
        return bidirectional_iterator_tag{};
      else
        return forward_iterator_tag{};
    }());
    using value_type = range_value_t<V>;
    using difference_type = range_difference_t<V>;

    iterator() = default;

    constexpr iterator_t<V>
    base() const {
      return current_;
    }

    constexpr range_reference_t<V>
    operator*() const {
      return *current_;
    }

    constexpr auto
    operator->() const noexcept
      requires contiguous_range<V>
    {
      return to_address(current_);
    }

    constexpr iterator&
    operator++() {
      ++current_;
      ++probe_;
      return *this;
    }

    constexpr iterator
    operator++(int) {
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator&
    operator--()
      requires bidirectional_range<V>
    {
      --current_;
      --probe_;
      return *this;
    }

    constexpr iterator
    operator--(int)
      requires bidirectional_range<V>
    {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    constexpr iterator&
    operator+=(difference_type n)
      requires random_access_range<V>
    {
      current_ += n;
      probe_ += n;
      return *this;
    }

    constexpr iterator&
    operator-=(difference_type n)
      requires random_access_range<V>
    {
      current_ -= n;
      probe_ -= n;
      return *this;
    }

    constexpr decltype(auto)
    operator[](difference_type n) const
      requires random_access_range<V>
    {
      return current_[n];
    }

    friend constexpr bool
    operator==(const iterator& x, const iterator& y) {
      return x.current_ == y.current_;
    }

    friend constexpr bool
    operator==(const iterator& x, const sentinel_t<V>& y) {
      return x.probe_ == y;
    }

    friend constexpr bool
    operator<(const iterator& x, const iterator& y)
      requires random_access_range<V>
    {
      return x.current_ < y.current_;
    }

    friend constexpr bool
    operator>(const iterator& x, const iterator& y)
      requires random_access_range<V>
    {
      return x.current_ > y.current_;
    }

    friend constexpr bool
    operator<=(const iterator& x, const iterator& y)
      requires random_access_range<V>
    {
      return x.current_ <= y.current_;
    }

    friend constexpr bool
    operator>=(const iterator& x, const iterator& y)
      requires random_access_range<V>
    {
      return x.current_ >= y.current_;
    }

    friend constexpr auto
    operator<=>(const iterator& x, const iterator& y)
      requires random_access_range<V> && three_way_comparable<iterator_t<V>>
    {
      return x.current_ <=> y.current_;
    }

    friend constexpr iterator
    operator+(iterator i, difference_type n)
      requires random_access_range<V>
    {
      return i += n;
    }

    friend constexpr iterator
    operator+(difference_type n, iterator i)
      requires random_access_range<V>
    {
      return i += n;
    }

    friend constexpr iterator
    operator-(iterator i, difference_type n)
      requires random_access_range<V>
    {
      return i -= n;
    }

    friend constexpr decltype(auto)
    iter_move(const iterator& i) noexcept(noexcept(ranges::iter_move(i.current_))) {
      return ranges::iter_move(i.current_);
    }

    friend constexpr void
    iter_swap(const iterator& x,
              const iterator& y) noexcept(noexcept(ranges::iter_swap(x.current_, y.current_)))
      requires indirectly_swappable<iterator_t<V>, iterator_t<V>>
    {
      ranges::iter_swap(x.current_, y.current_);
    }
  };

 public:
  drop_last_view()
    requires default_initializable<V>
  = default;

  constexpr explicit drop_last_view(V base, range_difference_t<V> count)
    : base_(std::move(base)), count_(count) { }

  constexpr V
  base() const&
    requires copy_constructible<V>
  {
    return base_;
  }

  constexpr V
  base() && {
    return std::move(base_);
  }

  constexpr auto
  begin()
    requires(!(__detail::__simple_view<V> && sized_range<const V>))
  {
    if constexpr (sized_range<V>) {
      if constexpr (random_access_range<V>)
        return ranges::begin(base_);
      else {
        const auto sz = ranges::distance(base_);
        const auto n = std::min(sz, count_);
        return std::counted_iterator(ranges::begin(base_), sz - n);
      }
    } else if constexpr (bidirectional_range<V> && common_range<V>)
      return ranges::begin(base_);
    else
      return iterator(ranges::begin(base_),
                      ranges::next(ranges::begin(base_), count_, ranges::end(base_)));
  }

  constexpr auto
  begin() const
    requires sized_range<const V>
  {
    if constexpr (random_access_range<const V>)
      return ranges::begin(base_);
    else {
      const auto sz = ranges::distance(base_);
      const auto n = std::min(sz, count_);
      return std::counted_iterator(ranges::begin(base_), sz - n);
    }
  }

  constexpr auto
  end()
    requires(!(__detail::__simple_view<V> && sized_range<const V>))
  {
    if constexpr (sized_range<V>) {
      if constexpr (random_access_range<V>) {
        const auto sz = ranges::distance(base_);
        const auto n = std::min(sz, count_);
        return ranges::next(ranges::begin(base_), sz - n);
      } else
        return std::default_sentinel;
    } else if constexpr (bidirectional_range<V> && common_range<V>)
      return ranges::prev(ranges::end(base_), count_, ranges::begin(base_));
    else
      return ranges::end(base_);
  }

  constexpr auto
  end() const
    requires sized_range<const V>
  {
    if constexpr (random_access_range<const V>) {
      const auto sz = ranges::distance(base_);
      const auto n = std::min(sz, count_);
      return ranges::next(ranges::begin(base_), sz - n);
    } else
      return std::default_sentinel;
  }

  constexpr auto
  size()
    requires sized_range<V>
  {
    const auto sz = ranges::size(base_);
    const auto n = static_cast<decltype(sz)>(count_);
    return sz > n ? sz - n : 0;
  }

  constexpr auto
  size() const
    requires sized_range<const V>
  {
    const auto sz = ranges::size(base_);
    const auto n = static_cast<decltype(sz)>(count_);
    return sz > n ? sz - n : 0;
  }
};

template<class R>
drop_last_view(R&&, range_difference_t<R>) -> drop_last_view<views::all_t<R>>;

template<class T>
constexpr bool enable_borrowed_range<drop_last_view<T>> = enable_borrowed_range<T>;
}  // namespace std::ranges
