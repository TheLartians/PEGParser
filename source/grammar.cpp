#include <easy_iterator.h>
#include <peg_parser/grammar.h>
#include <peg_parser/interpreter.h>

using namespace peg_parser::grammar;

namespace {

  /**  alternative to `std::get` that works on iOS < 11 */
  template <class T, class V> const T &pget(const V &v) {
    if (auto r = std::get_if<T>(&v)) {
      return *r;
    } else {
      throw std::runtime_error("corrupted grammar node");
    }
  }

}  // namespace

std::ostream &peg_parser::grammar::operator<<(std::ostream &stream, const Node &node) {
  using Symbol = peg_parser::grammar::Node::Symbol;

  switch (node.symbol) {
    case Node::Symbol::WORD: {
      stream << "'" << pget<std::string>(node.data) << "'";
      break;
    }

    case Node::Symbol::ANY: {
      stream << ".";
      break;
    }
    case Symbol::RANGE: {
      auto &v = pget<std::array<peg_parser::grammar::Letter, 2>>(node.data);
      stream << "[" << v[0] << "-" << v[1] << "]";
      break;
    }

    case Symbol::SEQUENCE: {
      const auto &data = pget<std::vector<Node::Shared>>(node.data);
      stream << "(";
      for (auto [i, n] : easy_iterator::enumerate(data)) {
        stream << *n << (i + 1 == data.size() ? "" : " ");
      }
      stream << ")";
      break;
    }

    case Symbol::CHOICE: {
      stream << "(";
      const auto &data = pget<std::vector<Node::Shared>>(node.data);
      for (auto [i, n] : easy_iterator::enumerate(data)) {
        stream << *n << (i + 1 == data.size() ? "" : " | ");
      }
      stream << ")";
      break;
    }

    case Symbol::ZERO_OR_MORE: {
      const auto &data = pget<Node::Shared>(node.data);
      stream << *data << "*";
      break;
    }

    case Symbol::ONE_OR_MORE: {
      const auto &data = pget<Node::Shared>(node.data);
      stream << *data << "+";
      break;
    }

    case Node::Symbol::OPTIONAL: {
      stream << *pget<Node::Shared>(node.data) << "?";
      break;
    }

    case Node::Symbol::ALSO: {
      stream << "&" << *pget<Node::Shared>(node.data);
      break;
    }

    case Node::Symbol::NOT: {
      stream << "!" << *pget<Node::Shared>(node.data);
      break;
    }

    case Node::Symbol::EMPTY: {
      stream << "''";
      break;
    }

    case Node::Symbol::ERROR: {
      stream << "[]";
      break;
    }

    case Node::Symbol::RULE: {
      auto rule = pget<std::shared_ptr<Rule>>(node.data);
      stream << rule->name;
      break;
    }

    case Node::Symbol::WEAK_RULE: {
      if (auto rule = pget<std::weak_ptr<Rule>>(node.data).lock()) {
        stream << rule->name;
      } else {
        stream << "<DeletedRule>";
      }
      break;
    }

    case Node::Symbol::END_OF_FILE: {
      stream << "<EOF>";
      break;
    }

    case Node::Symbol::FILTER: {
      stream << "<Filter>";
      break;
    }
  }

  return stream;
}
