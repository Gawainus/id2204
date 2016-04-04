/*
 * id2204 A1
 * Author: Yumen & Marion
 *
 * Description:
 * If there is a row or a column that does not have a queen, then,
 * by pigeon hole principle, there is a row or column that has
 * more than one queen which make the board in an attacking state.
 *
 * So sum of number on each tile by row, by column or by diagonal
 * should all be 1
 */

#include <gecode/driver.hh>

using namespace Gecode;

class Queens: public Script {
protected:
  const int dim; // the dimension of the chess board
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
  Queens(const SizeOptions& opt, unsigned int n): Script(opt), dim(n) {}

  /// Copy ctr
  Queens(bool share, Queens& s) : Script(share,s), dim(s.dim) {}

}; // end of class Queens

class QueensInt : virtual public Queens {
protected:
  /// Values for the whole board with dimxdim elements either 0 or 1
  IntVarArray x;
public:
  /// Propagation variants
  enum {
    PROP_NONE, ///< No additional constraints
    PROP_SAME, ///< Use "same" constraint with integer model
  };

  /// Constructor
  QueensInt(const SizeOptions& opt, unsigned int n)
      : Queens(opt, n), x(*this, dim*dim, 0, 1) {
    Matrix<IntVarArray> m(x, dim, dim);

    IntArgs c(dim);
    for (int i=0; i<dim; i++) {
      c[i] = 1;
    }

    // Constraints for rows and columns
    for (int i=0; i<dim; i++) {
      linear(*this, c, m.col(i), IRT_EQ, 1);
      linear(*this, c, m.row(i), IRT_EQ, 1);
    }

    // Constraints for diagonals

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
  QueensInt(bool share, QueensInt& s) : Queens(share, s) {
    x.update(*this, share, s.x);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new QueensInt(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    os << "  ";
    for (int i = 0; i<dim*dim; i++) {
      if (x[i].assigned()) {
        if (x[i].val()<10)
          os << x[i] << " ";
        else
          os << (char)(x[i].val()+'A'-10) << " ";
      }
      else
        os << ". ";
      if((i+1)%(dim*dim) == 0)
        os << std::endl << "  ";
    }
    os << std::endl;
  }

  void print(void) const {
    for (int i=0; i<dim; i++) {
      for (int j=0; j<dim; j++) {
        std::cout << x[i*dim+j] << " ";
      }
      std::cout << std::endl;
    }
  }

}; // end of class QueensInt


int
main(int argc, char* argv[]) {
  SizeOptions opt("Queens");
  opt.size(0);
  opt.icl(ICL_DOM);
  opt.solutions(2);

  opt.propagation(QueensInt::PROP_NONE);
  opt.propagation(QueensInt::PROP_NONE, "none", "no additional constraints");
  opt.propagation(QueensInt::PROP_SAME, "same",
                  "additional \"same\" constraint for integer model");

  opt.branching(Queens::BRANCH_SIZE_AFC);
  opt.branching(Queens::BRANCH_NONE, "none", "none");
  opt.branching(Queens::BRANCH_SIZE, "size", "min size");
  opt.branching(Queens::BRANCH_SIZE_DEGREE, "sizedeg", "min size over degree");
  opt.branching(Queens::BRANCH_SIZE_AFC, "sizeafc", "min size over afc");
  opt.branching(Queens::BRANCH_AFC, "afc", "maximum afc");
  opt.parse(argc,argv);

  QueensInt queensInt(opt, 6);
  DFS<QueensInt> dfs(&queensInt);
  // search and print all solutions
  int solnCount = 1;
  while (QueensInt* qi = dfs.next()) {
    std::cout << "Solution " << std::to_string(solnCount) << ":" << std::endl;
    qi->print();
    delete qi;
    std::cout << std::endl;
    solnCount++;
  }
  return 0;
} // end of main
