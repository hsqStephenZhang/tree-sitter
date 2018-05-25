#ifndef COMPILER_BUILD_TABLES_LEX_TABLE_BUILDER_H_
#define COMPILER_BUILD_TABLES_LEX_TABLE_BUILDER_H_

#include <memory>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include "compiler/parse_table.h"
#include "compiler/lex_table.h"

namespace std {

using tree_sitter::rules::Symbol;

template <>
struct hash<pair<Symbol::Index, Symbol::Index>> {
  size_t operator()(const pair<Symbol::Index, Symbol::Index> &) const;
};

} // namespace std

namespace tree_sitter {

struct ParseTable;
struct SyntaxGrammar;
struct LexicalGrammar;

namespace build_tables {

class LookaheadSet;

enum ConflictStatus {
  DoesNotMatch = 0,
  MatchesShorterStringWithinSeparators = 1 << 0,
  MatchesSameString = 1 << 1,
  MatchesLongerString = 1 << 2,
  MatchesLongerStringWithValidNextChar = 1 << 3,
  CannotDistinguish = (
    MatchesShorterStringWithinSeparators |
    MatchesSameString |
    MatchesLongerStringWithValidNextChar
  ),
};

struct CoincidentTokenIndex {
  std::unordered_map<
    std::pair<rules::Symbol::Index, rules::Symbol::Index>,
    std::unordered_set<ParseStateId>
  > entries;

  bool contains(rules::Symbol, rules::Symbol) const;
  const std::unordered_set<ParseStateId> &states_with(rules::Symbol, rules::Symbol) const;
};

class LexTableBuilder {
 public:
  static std::unique_ptr<LexTableBuilder> create(const SyntaxGrammar &,
                                                 const LexicalGrammar &,
                                                 const std::unordered_map<rules::Symbol, LookaheadSet> &,
                                                 const CoincidentTokenIndex &,
                                                 ParseTable *);

  struct BuildResult {
    LexTable main_table;
    LexTable keyword_table;
    rules::Symbol keyword_capture_token;
  };

  BuildResult build();

  ConflictStatus get_conflict_status(rules::Symbol, rules::Symbol) const;

 protected:
  LexTableBuilder() = default;
};

}  // namespace build_tables
}  // namespace tree_sitter

#endif  // COMPILER_BUILD_TABLES_LEX_TABLE_BUILDER_H_
