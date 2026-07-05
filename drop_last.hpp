#include <ranges>

namespace std::ranges {
template<view V>
  requires forward_range<V> || sized_range<V>
class drop_last_view : public view_interface<drop_last_view<V>> {
 private:
  V base_ = V();                     // exposition only
  range_difference_t<V> count_ = 0;  // exposition only

  class iterator;
  class sentinel;

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
    } else
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
    } else
      return sentinel(ranges::end(base_));
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
}  // namespace std::ranges
