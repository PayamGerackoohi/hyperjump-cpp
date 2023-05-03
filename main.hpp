#ifndef MAIN_HPP
#define MAIN_HPP

#include <memory>
#include <set>
#include <string>
#include <vector>

class Solver {
public:
  enum class Mode { FULL, FIRST };
  inline static const Mode DEFAULT_SOLVER_MODE = Mode::FIRST;
  static constexpr const char *OUTPUT_DIR = "data/output.txt";

  struct Settings {
    int size;
    int finalValue;
    bool needsOneTarget;
    std::vector<int> options;
    bool areOptionsInvalid;

    Settings(std::vector<int> &&options, int size, Mode mode, int finalValue) noexcept;
  };

  struct State {
    bool goalAchieved;
    int keyIndex;

    State() noexcept;
  };

  Solver(std::vector<int> &&options, int size = 6, Mode mode = DEFAULT_SOLVER_MODE,
         int finalValue = 9) noexcept;
  explicit Solver(Settings &&settings) noexcept;
  void solve() noexcept;

private:
  Settings settings;
  State state;

  inline void showInvalidOptionsError() noexcept;
  std::unique_ptr<std::vector<int>> makeOptionsFor(int key) noexcept;
};

class Tree {
  using Solutions = std::unique_ptr<std::set<std::string>>;

public:
  bool endPoint;
  Tree() noexcept;
  Tree(int value, Solver::Settings *settings, Solver::State *state, Tree *parent = nullptr) noexcept;
  std::string toString(std::string padding = "") noexcept;
  bool put(int n, int level) noexcept;
  inline int value() const noexcept;
  operator int() const noexcept;
  Solutions solutions() noexcept;

protected:
  std::vector<int> values;
  std::vector<int> cvalues;
  std::vector<int> remainedIndices;
  std::vector<Tree> adds;
  std::vector<Tree> subs;
  std::vector<Tree> muls;
  std::vector<Tree> divs;
  int digits;

private:
  Solver::Settings *settings;
  Solver::State *state;

  void put(std::vector<Tree> &trees, bool &success, int value, int n, int level, int digits) noexcept;
  std::string processSolution(Solutions &solutions, std::stringstream &, std::string padding = "") noexcept;
  void appendSolution(std::vector<Tree> &trees, Solutions &solutions, std::stringstream &,
                      std::string &padding) noexcept;
  void append(std::stringstream &, std::vector<Tree> &, char op, std::string &padding) noexcept;
};

int main();
void show(std::vector<int> &answer) noexcept;
void show(std::vector<Tree> &) noexcept;
void show(std::vector<Tree *> &) noexcept;
bool has(int n, std::vector<int> &indices, Solver::Settings *) noexcept;
void uniqueList(std::vector<int> &list) noexcept;
void uniqueList(std::vector<Tree> &) noexcept;
void uniqueList(std::vector<Tree *> &) noexcept;
std::unique_ptr<std::vector<int>> uniqueClone(std::vector<int> &list);
std::ostream &operator<<(std::ostream &os, std::vector<int> &v);

#endif
