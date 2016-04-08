/*
 * id2204 A1
 * Author: Yumen & Marion
 *
 * Based on Gecode/examples/sudoku.cpp credits to original authers
 *
 * Description: Standard 9x9 sudoku problems.
 *
 * Type ./queens -help for help
 *
 * Size Option: to run
 * put A1.cpp in the same folder with this file
 *
 * ./queens [options] [index of example to run]
 *
 */

#include <gecode/driver.hh>
#include "A1.cpp"

using namespace Gecode;

namespace {
  extern const unsigned int numOfExamples = sizeof(examples)/81/sizeof(int);
}

class Sudoku: public Script {
protected:
  const int n = 3; // each block is 3 by 3 since the game is 9 by 9
public:

  // Branching variants
  enum {
    BRANCH_NONE,        ///< Use lexicographic ordering
    BRANCH_SIZE,        ///< Use minimum size
    BRANCH_SIZE_DEGREE, ///< Use minimum size over degree
    BRANCH_SIZE_AFC,    ///< Use minimum size over afc
    BRANCH_AFC          ///< Use maximum afc
  };

  /// Ctr
  Sudoku(const SizeOptions& opt): Script(opt) {}

  /// Copy ctr
  Sudoku(bool share, Sudoku& s) : Script(share,s) {}

}; // end of class Sudoku

class SudokuInt : virtual public Sudoku {
protected:
  /// Values for the fields
  IntVarArray x;
public:
  // Constructor
  SudokuInt(const SizeOptions& opt)
      : Sudoku(opt), x(*this, n*n*n*n, 1, n*n) {
    const int nn = n*n;
    Matrix<IntVarArray> m(x, nn, nn);

    // Constraints for rows and columns
    for (int i=0; i<nn; i++) {
      distinct(*this, m.row(i), opt.icl());
      distinct(*this, m.col(i), opt.icl());
    }

    // Constraints for squares
    for (int i=0; i<nn; i+=n) {
      for (int j=0; j<nn; j+=n) {
        distinct(*this, m.slice(i, i+n, j, j+n), opt.icl());
      }
    }

    // Fill-in predefined fields
    for (int i=0; i<nn; i++)
      for (int j=0; j<nn; j++)
        if (int v = examples[opt.size()][j][i])
          rel(*this, m(i,j), IRT_EQ, v);



    if (opt.branching() == BRANCH_NONE) {
      branch(*this, x, INT_VAR_NONE(), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_SIZE) {
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_SIZE_DEGREE) {
      branch(*this, x, INT_VAR_DEGREE_SIZE_MAX(), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_SIZE_AFC) {
      branch(*this, x, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
    } else if (opt.branching() == BRANCH_AFC) {
      branch(*this, x, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_SPLIT_MIN());
    }
  }

  /// Constructor for cloning \a s
  SudokuInt(bool share, SudokuInt& s) : Sudoku(share, s) {
    x.update(*this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new SudokuInt(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "  ";
    for (int i = 0; i<n*n*n*n; i++) {
      if (x[i].assigned()) {
        if (x[i].val()<10)
          os << x[i] << " ";
        else
          os << (char)(x[i].val()+'A'-10) << " ";
      }
      else
        os << ". ";
      if((i+1)%(n*n) == 0)
        os << std::endl << "  ";
    }
    os << std::endl;
  }

private:
  /// Post the constraint that \a a and \a b take the same values
  void same(Space& home, int nn, IntVarArgs a, IntVarArgs b) {
    SetVar u(home, IntSet::empty, 1, nn);
    rel(home, SOT_DUNION, a, u);
    rel(home, SOT_DUNION, b, u);
  }

  /// Extract column \a bc from block starting at (\a i,\a j)
  IntVarArgs
  block_col(Matrix<IntVarArray> m, int bc, int i, int j) {
    return m.slice(bc*n+i, bc*n+i+1, j*n, (j+1)*n);
  }

  /// Extract row \a br from block starting at (\a i,\a j)
  IntVarArgs
  block_row(Matrix<IntVarArray> m, int br, int i, int j) {
    return m.slice(j*n, (j+1)*n, br*n+i, br*n+i+1);
  }
};

int
main(int argc, char* argv[]) {
  std::string title = "Sudoku Example ";
  SizeOptions opt(title.c_str());
  opt.size(0);
  //    opt.icl(ICL_DEF);  // 55 nodes
  //   opt.icl(ICL_VAL);  // 55 nodes
  opt.icl(ICL_DOM);  // 1 node. This is the fastest way ICL option
  opt.solutions(1);

  opt.branching(Sudoku::BRANCH_SIZE_AFC);
  opt.branching(Sudoku::BRANCH_NONE, "none", "none");
  opt.branching(Sudoku::BRANCH_SIZE, "size", "min size");
  opt.branching(Sudoku::BRANCH_SIZE_DEGREE, "sizedeg", "min size over degree");
  opt.branching(Sudoku::BRANCH_SIZE_AFC, "sizeafc", "min size over afc");
  opt.branching(Sudoku::BRANCH_AFC, "afc", "maximum afc");
  opt.parse(argc,argv);

  if (opt.size() >= numOfExamples) {
    std::cerr << "Error: size must be between 0 and "
    << numOfExamples-1 << std::endl;
    return 1;
  }

  Script::run<SudokuInt,DFS,SizeOptions>(opt);

  return 0;
} // end of main
