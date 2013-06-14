#ifndef VAST_REGEX_H
#define VAST_REGEX_H

#include "vast/config.h"

#ifdef VAST_CLANG
#include <regex>
#else
#include <boost/regex.hpp>
#endif

#include "vast/string.h"

namespace vast {

/// A regular expression.
class regex : util::totally_ordered<regex>
{
public:
  /// Constructs a regex from a glob expression. A glob expression consists
  /// of the following elements:
  ///
  ///     - `*`   Equivalent to `.*` in a regex
  ///
  ///     - `?`    Equivalent to `.` in a regex
  ///
  ///     - `[ab]` Equivalent to the character class `[ab]` in a regex.
  ///
  /// @param str The glob expression.
  ///
  /// @return A regex for *str*.
  static regex glob(std::string const& str);

  /// Constructs an empty regex.
  regex() = default;

  /// Constructs a regex from a VAST string.
  /// @param str The regular expression string.
  regex(string str);

  /// Copy-constructs a regex.
  /// @param other The regex to copy.
  regex(regex const& other) = default;

  /// Move-constructs a regex.
  /// @param other The regex to move.
#ifdef VAST_CLANG
  regex(regex&& other) = default;
#else
  // FIXME: Boost does not yet provide a move constructor for regexes, so we
  // copy the regex for now.
  regex(regex&& other)
    : rx_(other.rx_),
      str_(std::move(other.str_))
  {
  }
#endif

  /// Assigns another regex to this instance.
  /// @param other The right-hand side of the assignment.
  regex& operator=(regex const& other) = default;
#ifdef VAST_CLANG
  regex& operator=(regex&& other) = default;
#else
  // FIXME: See note above.
  regex& operator=(regex&& other)
  {
    rx_ = other.rx_;
    str_ = std::move(other.str_);
    return *this;
  }
#endif

  /// Matches a string against the regex.
  /// @param str The string to match.
  /// @return @c true if the regex matches @a str.
  template <typename String>
  bool match(String const& str) const
  {
#ifdef VAST_CLANG
    return std::regex_match(str.begin(), str.end(), rx_);
#else
    return boost::regex_match(str.begin(), str.end(), rx_);
#endif
  }

  /// Searches a pattern in a string.
  /// @param str The string to search.
  /// @return @c true if the regex matches inside @a str.
  template <typename String>
  bool search(String const& str) const
  {
#ifdef VAST_CLANG
    return std::regex_search(str.begin(), str.end(), rx_);
#else
    return boost::regex_search(str.begin(), str.end(), rx_);
#endif
  }

  /// Matches a string against the regex.
  /// @param str The string to match.
  /// @param f A function to invoke on each captured submatch.
  /// @return @c true if the regex matches @a str.
  bool match(std::string const& str,
             std::function<void(std::string const&)> f) const;

private:
  friend io::access;
  void serialize(io::serializer& sink);
  void deserialize(io::deserializer& source);

  friend bool operator==(regex const& x, regex const& y);
  friend bool operator<(regex const& x, regex const& y);
  friend std::string to_string(regex const& rx);
  friend std::ostream& operator<<(std::ostream& out, regex const& r);

#ifdef VAST_CLANG
  std::regex rx_;
#else
  boost::regex rx_;
#endif
  string str_;
};

} // namespace vast

#endif