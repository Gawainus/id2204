/*
 *  Authors:
 *    Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *    Christian Schulte, 2008-2015
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software, to deal in the software without restriction,
 *  including without limitation the rights to use, copy, modify, merge,
 *  publish, distribute, sublicense, and/or sell copies of the software,
 *  and to permit persons to whom the software is furnished to do so, subject
 *  to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include <gecode/search.hh>

using namespace Gecode;
namespace {
  unsigned int n = 0;
}

class Square : public Script {
protected:
  IntVar s; // side of the enclosing box
  IntVarArray x;
  IntVarArray y;

private:
  static int size(int i){
    return n-i;
  }
  static bool filter(const Space& home, IntVar x, int i){
    return x.size() >= 5;
  }
public:
  // Branching variants
  enum {
    BRANCH_X_FIRST,     ///< Assign x then y
    BRANCH_BIG_FIRST,   ///< Try bigger squares first
    BRANCH_LEFT_FIRST,  ///< Try to place squares from left to right
    BRANCH_TOP_FIRST,   ///< Try to place squares from top to bottom
    BRANCH_INTERVAL,
    BRANCH_SPLIT
  };
  
  Square(const SizeOptions& opt): Script(opt) {
    n = opt.size();
    s = IntVar(*this, sqrt(n*(n+1)*(2*n+1)/6), n*sqrt(n+1));
    x = IntVarArray(*this, n-1, 0, n*sqrt(n+1)-1);
    y = IntVarArray(*this, n-1, 0, n*sqrt(n+1)-1);

    
    // max coordinates
    for (int i=0; i<n-1; i++){
      rel(*this, x[i]<=s-size(i));
      rel(*this, y[i]<=s-size(i));
    }

    rel(*this, x[0]<=(s-n)/2);
    rel(*this, y[0]<=x[0]);

    for (int i=0; i<n-1; i++) {
      if (size(i)==45){
	      rel(*this, x[i]!=10);
	      rel(*this, y[i]!=10);
      }
      else if (size(i)>=34) {
	      rel(*this, x[i]!=9);
	      rel(*this, y[i]!=9);
      }
      else if (size(i)>=30){
        rel(*this, x[i]!=8);
        rel(*this, y[i]!=8);
      }
      else if (size(i)>=22){
        rel(*this, x[i]!=7);
        rel(*this, y[i]!=7);
      }
      else if (size(i)>=18){
        rel(*this, x[i]!=6);
        rel(*this, y[i]!=6);
      }
      else if (size(i)>=12){
        rel(*this, x[i]!=5);
        rel(*this, y[i]!=5);
      }
      else if (size(i)>=9){
	      rel(*this, x[i]!=4);
        rel(*this, y[i]!=4);
      }
      else if (size(i)>=5){
        rel(*this, x[i]!=3);
        rel(*this, y[i]!=3);
      }
      else if (size(i)>=2){
        rel(*this, x[i]!=2);
        rel(*this, y[i]!=2);
      }
      if (size(i)==3){
        rel(*this, x[i]!=3);
	      rel(*this, y[i]!=3);
      }
      if (size(i)==2){
	      rel(*this, x[i]!=1);
	      rel(*this, y[i]!=1);
      }
    }

    // no overlap - disjoint
    for (int i=0; i<n-1; i++){
      for (int j=i+1; j<n-1; j++){
        rel(*this, x[j]>=x[i]+size(i) || x[i]>=x[j]+size(j) ||
            y[i]>=y[j]+size(j) || y[j]>=y[i]+size(i));
      }
    }

    // no overlap - cumulative column
    for (int k=0; k<n*sqrt(n+1); k++){
      BoolVarArray bc(*this, n-1, 0, 1);
      for (int i=0; i<n-1; i++){
	      //bc[i] = expr(*this, (x[i] <= k) && (x[i]+size(i)>k));
	      dom(*this, x[i], k-size(i)+1, k, bc[i]);
      }
      rel(*this, sum(IntArgs::create(n-1,n,-1), bc) <= s);
    }

    // no overlap - cumulative row
    for (int k=0; k<n*sqrt(n+1); k++) {
      BoolVarArray br(*this, n-1, 0, 1);
      for (int i=0; i<n-1; i++) {
        //br[i] = expr(*this, (y[i] <= k) && (y[i]+size(i)>k));
        dom(*this, y[i], k-size(i)+1, k, br[i]);
      }
      rel(*this, sum(IntArgs::create(n-1,n,-1), br) <= s);
    }

    // Branching
    branch(*this, s, INT_VAL_MIN());
    if (opt.branching() == BRANCH_X_FIRST) {
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
      branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    }
    else if (opt.branching() == BRANCH_BIG_FIRST) {
      for (int i = 0; i<n-1; i++){
	      branch(*this, x[i], INT_VAL_MIN());
	      branch(*this, y[i], INT_VAL_MIN());
      }
    }
    else if (opt.branching() == BRANCH_LEFT_FIRST) {
      branch(*this, x, INT_VAR_MIN_MIN(), INT_VAL_MIN());
      branch(*this, y, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    }
    else if (opt.branching() == BRANCH_TOP_FIRST) {
      branch(*this, y, INT_VAR_MAX_MAX(), INT_VAL_MAX());
      branch(*this, x, INT_VAR_SIZE_MIN(), INT_VAL_MAX());
    } else if (opt.branching() == BRANCH_INTERVAL) {
      branch(*this, x, INT_VAR_NONE(), INT_VAL_SPLIT_MIN(), &filter);
      branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
      branch(*this, y, INT_VAR_NONE(), INT_VAL_SPLIT_MIN(), &filter);
      branch(*this, y, INT_VAR_NONE(), INT_VAL_MIN());
    } else if (opt.branching() == BRANCH_SPLIT) {
      branch(*this, x, INT_VAR_NONE(), INT_VAL_SPLIT_MIN(), &filter);
      branch(*this, y, INT_VAR_NONE(), INT_VAL_SPLIT_MIN(), &filter);
      branch(*this, x, INT_VAR_NONE(), INT_VAL_MIN());
      branch(*this, y, INT_VAR_NONE(), INT_VAL_MIN());
    }
    
  }
  
  Square(bool share, Square& square) : Script(share, square) {
    s.update(*this, share, square.s);
    x.update(*this, share, square.x);
    y.update(*this, share, square.y);
  }

  virtual Space* copy(bool share) {
    return new Square(share,*this);
  }

  virtual void print(std::ostream& os) const {
    os << "s = " << s << std::endl << std::endl;
    for (int i = 0; i<x.size(); i++){
      os << x.size()+1-i << ", x=" << x[i] << ", y=" << y[i] << std::endl; 
    }
  }
};

int main(int argc, char* argv[]) {
  // commandline options
  SizeOptions opt("Square Packing");
  opt.solutions(1);
  opt.size(5);

  opt.branching(Square::BRANCH_X_FIRST);
  opt.branching(Square::BRANCH_X_FIRST, "x", "x then y");
  opt.branching(Square::BRANCH_BIG_FIRST, "big", "big first");
  opt.branching(Square::BRANCH_LEFT_FIRST, "left", "left first");
  opt.branching(Square::BRANCH_TOP_FIRST, "top", "top first");
  opt.branching(Square::BRANCH_INTERVAL, "interval", "interval");
  opt.branching(Square::BRANCH_SPLIT, "split", "split");
  
  opt.parse(argc,argv);
  if (opt.size() < 2) {
    std::cerr << "Error: size must be greater than 1" << std::endl;
    return 1;
  }
  
  // run script
  Script::run<Square,BAB,SizeOptions>(opt);
  return 0;
}
