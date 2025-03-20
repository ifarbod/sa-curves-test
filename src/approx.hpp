// borrowed from doctest, edited to compile outside their codebase

#include <limits>
#include <numeric>

struct Approx {
  Approx(double value);

  Approx operator()(double value) const;

#ifdef DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS
  template <typename T>
  explicit Approx(const T &value,
                  typename detail::types::enable_if<
                      std::is_constructible<double, T>::value>::type * =
                      static_cast<T *>(nullptr)) {
    *this = static_cast<double>(value);
  }
#endif // DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS

  Approx &epsilon(double newEpsilon);

#ifdef DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS
  template <typename T>
  typename std::enable_if<std::is_constructible<double, T>::value,
                          Approx &>::type
  epsilon(const T &newEpsilon) {
    m_epsilon = static_cast<double>(newEpsilon);
    return *this;
  }
#endif //  DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS

  Approx &scale(double newScale);

#ifdef DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS
  template <typename T>
  typename std::enable_if<std::is_constructible<double, T>::value,
                          Approx &>::type
  scale(const T &newScale) {
    m_scale = static_cast<double>(newScale);
    return *this;
  }
#endif // DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS

  // clang-format off
     friend bool operator==(double lhs, const Approx & rhs);
     friend bool operator==(const Approx & lhs, double rhs);
     friend bool operator!=(double lhs, const Approx & rhs);
     friend bool operator!=(const Approx & lhs, double rhs);
     friend bool operator<=(double lhs, const Approx & rhs);
     friend bool operator<=(const Approx & lhs, double rhs);
     friend bool operator>=(double lhs, const Approx & rhs);
     friend bool operator>=(const Approx & lhs, double rhs);
     friend bool operator< (double lhs, const Approx & rhs);
     friend bool operator< (const Approx & lhs, double rhs);
     friend bool operator> (double lhs, const Approx & rhs);
     friend bool operator> (const Approx & lhs, double rhs);

#ifdef DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS
#define DOCTEST_APPROX_PREFIX \
    template <typename T> friend typename std::enable_if<std::is_constructible<double, T>::value, bool>::type

    DOCTEST_APPROX_PREFIX operator==(const T& lhs, const Approx& rhs) { return operator==(static_cast<double>(lhs), rhs); }
    DOCTEST_APPROX_PREFIX operator==(const Approx& lhs, const T& rhs) { return operator==(rhs, lhs); }
    DOCTEST_APPROX_PREFIX operator!=(const T& lhs, const Approx& rhs) { return !operator==(lhs, rhs); }
    DOCTEST_APPROX_PREFIX operator!=(const Approx& lhs, const T& rhs) { return !operator==(rhs, lhs); }
    DOCTEST_APPROX_PREFIX operator<=(const T& lhs, const Approx& rhs) { return static_cast<double>(lhs) < rhs.m_value || lhs == rhs; }
    DOCTEST_APPROX_PREFIX operator<=(const Approx& lhs, const T& rhs) { return lhs.m_value < static_cast<double>(rhs) || lhs == rhs; }
    DOCTEST_APPROX_PREFIX operator>=(const T& lhs, const Approx& rhs) { return static_cast<double>(lhs) > rhs.m_value || lhs == rhs; }
    DOCTEST_APPROX_PREFIX operator>=(const Approx& lhs, const T& rhs) { return lhs.m_value > static_cast<double>(rhs) || lhs == rhs; }
    DOCTEST_APPROX_PREFIX operator< (const T& lhs, const Approx& rhs) { return static_cast<double>(lhs) < rhs.m_value && lhs != rhs; }
    DOCTEST_APPROX_PREFIX operator< (const Approx& lhs, const T& rhs) { return lhs.m_value < static_cast<double>(rhs) && lhs != rhs; }
    DOCTEST_APPROX_PREFIX operator> (const T& lhs, const Approx& rhs) { return static_cast<double>(lhs) > rhs.m_value && lhs != rhs; }
    DOCTEST_APPROX_PREFIX operator> (const Approx& lhs, const T& rhs) { return lhs.m_value > static_cast<double>(rhs) && lhs != rhs; }
#undef DOCTEST_APPROX_PREFIX
#endif // DOCTEST_CONFIG_INCLUDE_TYPE_TRAITS

  // clang-format on

  double m_epsilon;
  double m_scale;
  double m_value;
};

Approx::Approx(double value)
    : m_epsilon(static_cast<double>(FLT_EPSILON) *
                100),
      m_scale(1.0), m_value(value) {}

Approx Approx::operator()(double value) const {
  Approx approx(value);
  approx.epsilon(m_epsilon);
  approx.scale(m_scale);
  return approx;
}

Approx &Approx::epsilon(double newEpsilon) {
  m_epsilon = newEpsilon;
  return *this;
}
Approx &Approx::scale(double newScale) {
  m_scale = newScale;
  return *this;
}

bool operator==(double lhs, const Approx &rhs) {
  // Thanks to Richard Harris for his help refining this formula
  return fabs(lhs - rhs.m_value) <
         rhs.m_epsilon *
             (rhs.m_scale +
              std::max<double>(fabs(lhs), fabs(rhs.m_value)));
}
bool operator==(const Approx &lhs, double rhs) { return operator==(rhs, lhs); }
bool operator!=(double lhs, const Approx &rhs) { return !operator==(lhs, rhs); }
bool operator!=(const Approx &lhs, double rhs) { return !operator==(rhs, lhs); }
bool operator<=(double lhs, const Approx &rhs) {
  return lhs < rhs.m_value || lhs == rhs;
}
bool operator<=(const Approx &lhs, double rhs) {
  return lhs.m_value < rhs || lhs == rhs;
}
bool operator>=(double lhs, const Approx &rhs) {
  return lhs > rhs.m_value || lhs == rhs;
}
bool operator>=(const Approx &lhs, double rhs) {
  return lhs.m_value > rhs || lhs == rhs;
}
bool operator<(double lhs, const Approx &rhs) {
  return lhs < rhs.m_value && lhs != rhs;
}
bool operator<(const Approx &lhs, double rhs) {
  return lhs.m_value < rhs && lhs != rhs;
}
bool operator>(double lhs, const Approx &rhs) {
  return lhs > rhs.m_value && lhs != rhs;
}
bool operator>(const Approx &lhs, double rhs) {
  return lhs.m_value > rhs && lhs != rhs;
}
