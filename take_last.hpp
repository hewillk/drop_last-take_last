#include <ranges>

namespace std::ranges {
template<view V>
  requires forward_range<V> || sized_range<V>
class take_last_view : public view_interface<take_last_view<V>> {
 private:
  V base_ = V();                     // exposition only
  range_difference_t<V> count_ = 0;  // exposition only

 public:
  take_last_view()
    requires default_initializable<V>
  = default;

  constexpr explicit take_last_view(V base, range_difference_t<V> count)
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
    requires(!(__detail::__simple_view<V> && random_access_range<const V> && sized_range<const V>))
  {
    if constexpr (sized_range<V>) {
      const auto sz = ranges::distance(base_);
      const auto n = std::min(sz, count_);
      const auto from_begin = sz - n;
      if constexpr (bidirectional_range<V> && common_range<V>) {
        if (n < from_begin)
          return ranges::prev(ranges::end(base_), n);
      }
      return ranges::next(ranges::begin(base_), from_begin);
    } else if constexpr (bidirectional_range<V> && common_range<V>) {
      return ranges::prev(ranges::end(base_), count_, ranges::begin(base_));
    } else {
      const auto end = ranges::end(base_);
      auto slow = ranges::begin(base_);
      auto fast = ranges::next(slow, count_, end);
      while (fast != end) {
        ++slow;
        ++fast;
      }
      /* Theoretically, fast here can be used as the return for end() */
      return slow;
    }
  }

  constexpr auto
  begin() const
    requires random_access_range<const V> && sized_range<const V>
  {
    const auto sz = ranges::distance(base_);
    const auto n = std::min(sz, count_);
    return ranges::next(ranges::begin(base_), sz - n);
  }

  constexpr auto
  end() {
    /*
      If end() relies on the fast cache calculated by begin(): When v.end() is called first,
      begin()  hasn't executed yet, and the cache is either empty。 In this case, what should end()
      do? It's forced to run an O(N) to find the endpoint. Therefore, this optimization is not
      performed.
    */
    return ranges::end(base_);
  }

  constexpr auto
  end() const
    requires range<const V>
  {
    return ranges::end(base_);
  }

  constexpr auto
  size()
    requires sized_range<V>
  {
    const auto sz = ranges::size(base_);
    return std::min(sz, static_cast<decltype(sz)>(count_));
  }

  constexpr auto
  size() const
    requires sized_range<const V>
  {
    const auto sz = ranges::size(base_);
    return std::min(sz, static_cast<decltype(sz)>(count_));
  }
};

template<class R>
take_last_view(R&&, range_difference_t<R>) -> take_last_view<views::all_t<R>>;

template<class T>
constexpr bool enable_borrowed_range<take_last_view<T>> = enable_borrowed_range<T>;
}  // namespace std::ranges
