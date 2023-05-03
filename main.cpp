#include "main.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <utility>

using std::cerr;
using std::cout;
using std::endl;
using std::make_unique;
using std::move;
using std::ostream;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;
using std::unique_ptr;
using std::vector;

int main() {
  // Solver solver({8, 7, 7, 4, 1, 3, 6, 7});
  Solver solver({8, 3, 4, 1, 5, 7, 8, 1}, 6, Solver::Mode::FULL);
  // Solver solver({8, 3, 4, 1, 5, 7, 8, 1}, 8, Solver::Mode::FULL);
  // Solver solver({8, 7, 7, 4, 1, 3, 6, 7}, 6, Solver::Mode::FULL);
  solver.solve();
}

void uniqueList(vector<int> &list) noexcept {
  std::sort(list.begin(), list.end());
  auto it = std::unique(list.begin(), list.end());
  list.resize(std::distance(list.begin(), it));
}

void uniqueList(vector<Tree> &list) noexcept {
  std::sort(list.begin(), list.end());
  auto it = std::unique(list.begin(), list.end());
  list.resize(std::distance(list.begin(), it));
}

void uniqueList(vector<Tree *> &list) noexcept {
  std::sort(list.begin(), list.end(), [](Tree *l, Tree *r) { return *l < *r; });
  auto it = std::unique(list.begin(), list.end(), [](Tree *l, Tree *r) { return *l == *r; });
  list.resize(std::distance(list.begin(), it));
}

Solver::Solver(vector<int> &&options, int size, Mode mode, int finalValue) noexcept
    : settings(move(options), size, move(mode), finalValue) {}

// TODO does move work for vectors?
Solver::Solver(Solver::Settings &&settings) noexcept : settings(move(settings)) {
  if (settings.areOptionsInvalid)
    showInvalidOptionsError();
}

unique_ptr<vector<int>> Solver::makeOptionsFor(int key) noexcept {
  auto list = make_unique<vector<int>>();
  list->reserve(settings.options.size());
  bool found = false;
  for (int option : settings.options) {
    if (!found && option == key)
      found = true;
    else
      list->push_back(option);
  }
  uniqueList(*list);
  return list;
}

void Solver::solve() noexcept {
  if (settings.areOptionsInvalid) {
    showInvalidOptionsError();
    return;
  }
  auto keys = uniqueClone(settings.options);
  using il = std::initializer_list<int>;
  std::ofstream output(OUTPUT_DIR);
  for (auto key : *keys) {
    auto &o = settings.options;
    state.keyIndex = std::find(o.begin(), o.end(), key) - o.begin();
    auto options = makeOptionsFor(key);
    for (int option : *options) {
      Tree tree(key, &settings, &state);
      tree.put(option, settings.size - 1);
      auto solutions = tree.solutions();
      for (auto &solution : *solutions)
        output << solution << endl;
      if (state.goalAchieved)
        break;
    }
    if (state.goalAchieved)
      break;
  }
  output.close();
}

Tree::Tree() noexcept : settings(nullptr), state(nullptr) {}

Tree::Tree(int value, Solver::Settings *settings, Solver::State *state, Tree *parent) noexcept
    : digits(0), endPoint(true), settings(settings), state(state) {
  values.push_back(value);
  if (parent != nullptr) {
    auto cvs = parent->cvalues;
    auto ri = parent->remainedIndices;
    for (auto &v : cvs)
      values.push_back(v + value);
    remainedIndices.insert(remainedIndices.begin(), ri.begin(), ri.end());
  } else {
    auto optionsSize = settings->options.size();
    remainedIndices.reserve(optionsSize);
    for (int i = 0; i < optionsSize; i++)
      if (i != state->keyIndex)
        remainedIndices.push_back(i);
  }
  for (auto &v : values)
    cvalues.push_back(10 * v);
};

int Tree::value() const noexcept {
  if (values.size() == 0)
    return 0;
  else
    return values[0];
}

Tree::Solutions Tree::solutions() noexcept {
  stringstream ss;
  auto solutions = make_unique<set<string>>();
  processSolution(solutions, ss);
  return solutions;
}

string Tree::processSolution(Solutions &solutions, stringstream &ss, string padding) noexcept {
  // TODO stringstream is horribly used!
  ss.str("");
  if (padding != "")
    padding += " -> ";
  padding += to_string(values[0]);
  appendSolution(adds, solutions, ss, padding);
  appendSolution(subs, solutions, ss, padding);
  appendSolution(muls, solutions, ss, padding);
  appendSolution(divs, solutions, ss, padding);
  if (endPoint) {
    ss << padding;
    string solution = ss.str();
    if (solution.size() > 1) {
      solutions->emplace(solution);
      if (settings->needsOneTarget)
        state->goalAchieved = true;
    }
    return "";
  } else
    return ss.str();
}

void Tree::appendSolution(vector<Tree> &trees, Solutions &solutions, stringstream &ss,
                          string &padding) noexcept {
  // TODO stringstream is horribly used!
  // TODO inefficient: the whole tree must be flattened.
  if (state->goalAchieved)
    return;
  vector<Tree *> refTrees;
  refTrees.reserve(trees.size());
  for (auto &tree : trees)
    refTrees.emplace_back(&tree);
  uniqueList(refTrees);
  for (auto &tree : refTrees) {
    ss << tree->processSolution(solutions, ss, padding);
    if (state->goalAchieved)
      break;
  }
}

void Tree::append(stringstream &ss, vector<Tree> &trees, char op, string &padding) noexcept {
  // TODO stringstream is horribly used!
  for (int i = 0; i < trees.size(); i++) {
    auto tree = trees[i];
    ss << padding;
    for (int j = 0; j < tree.digits; j++)
      ss << ".";
    ss << op << " " << trees[i].toString(padding);
  }
}

string Tree::toString(string padding) noexcept {
  // TODO stringstream is horribly used!
  stringstream ss;
  ss << values[0] << endl;
  padding += " ";
  append(ss, adds, '+', padding);
  append(ss, subs, '-', padding);
  append(ss, muls, '*', padding);
  append(ss, divs, '/', padding);
  return ss.str();
}

bool has(int n, vector<int> &indices, Solver::Settings *settings) noexcept {
  for (int i = 0; i < indices.size(); i++)
    if (settings->options[indices[i]] == n) {
      indices.erase(indices.begin() + i);
      return true;
    }
  return false;
}

void Tree::put(vector<Tree> &trees, bool &success, int value, int n, int level, int digits) noexcept {
  Tree t(n, settings, state, this);
  if (t.put(value % 10, level)) {
    t.digits = digits;
    trees.push_back(move(t));
    success = true;
  }
}

bool Tree::put(int n, int level) noexcept {
  if (level == 0 && n == settings->finalValue)
    return true;
  if (!has(n, remainedIndices, settings))
    return false;
  if (level == 0)
    return false;
  level--;
  bool success = false;
  for (int i = 0; i < values.size(); i++) {
    auto value = values[i];
    put(adds, success, value + n, n, level, i);
    if (value > n)
      put(subs, success, value - n, n, level, i);
    put(muls, success, value * n, n, level, i);
    if (value % n == 0)
      put(divs, success, value / n, n, level, i);
  }
  endPoint = !success;
  return success;
}

// TODO show functions need cleanup
void show(vector<int> &answer) noexcept {
  stringstream ss;
  ss << "[";
  bool first = true;
  for (auto i : answer) {
    if (first)
      first = false;
    else
      ss << ", ";
    ss << i;
  }
  ss << "]";
  cout << ss.str() << endl;
}

void show(vector<Tree> &trees) noexcept {
  stringstream ss;
  ss << "[";
  bool first = true;
  for (auto &tree : trees) {
    if (first)
      first = false;
    else
      ss << ", ";
    ss << tree.value();
  }
  ss << "]";
  cout << ss.str() << endl;
}

void show(vector<Tree *> &trees) noexcept {
  stringstream ss;
  ss << "[";
  bool first = true;
  for (auto &tree : trees) {
    if (first)
      first = false;
    else
      ss << ", ";
    ss << tree->value();
  }
  ss << "]";
  cout << ss.str() << endl;
}

Solver::Settings::Settings(std::vector<int> &&options, int size, Mode mode, int finalValue) noexcept
    : size(size), finalValue(finalValue), areOptionsInvalid(false),
      needsOneTarget(mode == DEFAULT_SOLVER_MODE), options(move(options)) {
  if (this->options.size() < 2)
    areOptionsInvalid = true;
}

Solver::State::State() noexcept : goalAchieved(false), keyIndex(0) {}

void Solver::showInvalidOptionsError() noexcept { std::cerr << "Invalid Options!" << endl; }

Tree::operator int() const noexcept { return value(); }

unique_ptr<vector<int>> uniqueClone(vector<int> &list) {
  auto clone = make_unique<vector<int>>();
  clone->reserve(list.size());
  for (int i : list)
    clone->push_back(i);
  uniqueList(*clone);
  return clone;
}

ostream &operator<<(ostream &os, vector<int> &v) {
  os << "[";
  bool first = true;
  for (int i : v) {
    if (first)
      first = false;
    else
      os << ", ";
    os << i;
  }
  os << "]";
  return os;
}
