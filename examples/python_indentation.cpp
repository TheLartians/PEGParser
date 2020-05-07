/**
 * This is a proof-of-concept example that parses python-like indentation
 * blocks.
 */

#include <peg_parser/generator.h>

#include <algorithm>
#include <iostream>
#include <unordered_set>
#include <vector>

int main() {
  using namespace std;

  struct Block {
    size_t begin, length;
  };

  using Blocks = vector<Block>;

  peg_parser::ParserGenerator<void, Blocks &> blockParser;

  blockParser["Indentation"] << "' '*";

  /** storage for indentation depths */
  std::vector<unsigned> indentations;

  /** initializer is necessarry to reset the state after syntax errors */
  blockParser["InitBlocks"] << "''" << [&](auto &) -> bool {
    indentations.resize(0);
    return true;
  };

  /**
   * matches the current block intendation
   * note that this rule is not cacheable as results are context-dependent
   */
  blockParser["SameIndentation"] << "Indentation" <<
      [&](auto &s) -> bool { return s->length() == indentations.back(); };
  blockParser["SameIndentation"]->cacheable = false;

  /** matches a deeper block intendation */
  blockParser["DeeperIndentation"]
      << "Indentation" << [&](auto &s) -> bool { return s->length() > indentations.back(); };
  blockParser["DeeperIndentation"]->cacheable = false;

  // enters a new block and stores the indentation
  blockParser["EnterBlock"] << "Indentation" << [&](auto &s) -> bool {
    if (indentations.size() == 0 || s->length() > indentations.back()) {
      indentations.push_back(s->length());
      return true;
    } else {
      return false;
    }
  };
  blockParser["EnterBlock"]->cacheable = false;

  /** matches a line in the current block */
  blockParser["Line"] << "SameIndentation (!'\n' .)+ '\n'";
  blockParser.getRule("Line")->cacheable = false;

  /** matches an empty line */
  blockParser["EmptyLine"] << "Indentation '\n'";

  /** exits a block and pops the current indentation */
  blockParser["ExitBlock"] << "''" << [&](auto &) -> bool {
    indentations.pop_back();
    return true;
  };
  blockParser.getRule("ExitBlock")->cacheable = false;

  /** store all successfully parsed blocks */
  blockParser["Block"] << "&EnterBlock Line (EmptyLine | Block | Line)* &ExitBlock" >>
      [](auto e, Blocks &blocks) {
        for (auto a : e) a.evaluate(blocks);
        blocks.push_back(Block{e.position(), e.length()});
      };

  blockParser.setStart(blockParser["Start"] << "InitBlocks Block");

  while (true) {
    string str, input;
    cout << "Enter a python-like indented string. Push enter twice to parse." << endl;
    cout << "> ";
    getline(cin, str);
    if (str == "q" || str == "quit") {
      break;
    }
    do {
      input += str + '\n';
      cout << "- ";
      getline(cin, str);
    } while (str != "");
    try {
      Blocks blocks;
      blockParser.run(input, blocks);
      cout << "matched " << blocks.size() << " blocks." << endl;
      for (auto b : blocks) {
        cout << "- from line " << std::count(input.begin(), input.begin() + b.begin, '\n') + 1;
        cout << " to " << std::count(input.begin(), input.begin() + b.begin + b.length, '\n')
             << endl;
      }
    } catch (peg_parser::SyntaxError &error) {
      auto syntax = error.syntax;
      cout << "  ";
      cout << "  "
           << "Syntax error at character " << syntax->end << " while parsing " << syntax->rule->name
           << endl;
    }
  }

  return 0;
}
