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
 * 
 *
 *  This file contains both the class NoOverlap and the class Square
 *  Execution command: ./square n
 *  with n the size of the biggest square
 *  
 *  Options:
 *  "- model noprop" (default) does not use the external nooverlap propagator
 *  "- model prop" uses the external propagator from the overlap class
 *  "- branching x/big/left/top/interval/split" for the different branching
 *  options (see details below)
 *  The best option we have found is "split", which gives for instance 133000
 *  nodes for n=22 
 */

#include <gecode/driver.hh>

using namespace Gecode;
using namespace Gecode::Int;

namespace {
  int n = 0; // size of the biggest square
}

// The no-overlap propagator
class NoOverlap : public Propagator {
protected:
  // The x-coordinates
  ViewArray<IntView> x;
  // The width (array)
  int* w;
  // The y-coordinates
  ViewArray<IntView> y;
  // The heights (array)
  int* h;

public:
  // Create propagator and initialize
  NoOverlap(Home home,
            ViewArray<IntView>& x0, int w0[],
            ViewArray<IntView>& y0, int h0[])
      : Propagator(home), x(x0), w(w0), y(y0), h(h0) {
    x.subscribe(home,*this,PC_INT_BND);
    y.subscribe(home,*this,PC_INT_BND);
  }
  // Post no-overlap propagator
  static ExecStatus post(Home home,
                         ViewArray<IntView>& x, int w[],
                         ViewArray<IntView>& y, int h[]) {
    // Only if there is something to propagate
    if (x.size() > 1)
      (void) new (home) NoOverlap(home,x,w,y,h);
    return ES_OK;
  }

  // Copy constructor during cloning
  NoOverlap(Space& home, bool share, NoOverlap& p)
      : Propagator(home,share,p) {
    x.update(home,share,p.x);
    y.update(home,share,p.y);
    // Also copy width and height arrays
    w = home.alloc<int>(x.size());
    h = home.alloc<int>(y.size());
    for (int i=x.size(); i--; ) {
      w[i]=p.w[i]; h[i]=p.h[i];
    }
  }
  // Create copy during cloning
  virtual Propagator* copy(Space& home, bool share) {
    return new (home) NoOverlap(home,share,*this);
  }

  // Return cost (defined as cheap quadratic)
  virtual PropCost cost(const Space&, const ModEventDelta&) const {
    return PropCost::quadratic(PropCost::LO, 2*x.size());
  }

  // Perform propagation
  virtual ExecStatus propagate(Space& home, const ModEventDelta&) {
    int n=x.size();
    for (int i=0; i<n-1; i++) {
      for (int j = i + 1; j < n; j++) {
	//if j can't be left, right, or down, then must be up
	if ((x[j].min() + w[j] > x[i].max()) && // j cannot be left
	    (x[i].min() + w[i] > x[j].max()) && // j cannot be right
	    (y[j].min() + h[j] > y[i].max()))   // j cannot be down
	  GECODE_ME_CHECK(y[j].gq(home, y[i].min()+h[i])); // j is up

	//if j can't be left, right, or up, then must be down
	if ((x[j].min() + w[j] > x[i].max()) && // j cannot be left
	    (x[i].min() + w[i] > x[j].max()) && // j cannot be right
	    (y[i].min() + h[i] > y[j].max()))   // j cannot be up
	  GECODE_ME_CHECK(y[i].gq(home, y[j].min()+h[j])); // j is down

	//if j can't be left, up, or down, then must be right
	if ((x[j].min() + w[j] > x[i].max()) && // j cannot be left
	    (y[i].min() + h[i] > y[j].max()) && // j cannot be up
	    (y[j].min() + h[j] > y[i].max()))   // j cannot be down
	  GECODE_ME_CHECK(x[j].gq(home, x[i].min()+w[i])); // j is right

	//if j can't be right, up, or down, then must be left
	if ((x[i].min() + w[i] > x[j].max()) && // j cannot be right
	    (y[i].min() + h[i] > y[j].max()) && // j cannot be up
	    (y[j].min() + h[j] > y[i].max()))   // j cannot be down
	  GECODE_ME_CHECK(x[i].gq(home, x[j].min()+w[j])); // j is left
      }
    }

    bool subsumed = false;
    int i,j=0;
    while (subsumed && i<x.size()-1){
      subsumed =
	(x[j].min() >= x[i].max()+w[i]) || // j is on the right of i
	(x[i].min() >= x[j].max()+w[j]) || // j is on the left of i
	(y[j].min() >= y[i].max()+h[i]) || // j is above i
	(y[i].min() >= y[j].max()+h[j]);   // j is below i
      if (++j >= x.size()){
	i++;
	j=i+1;
      }
    }

    if (subsumed)
      return home.ES_SUBSUMED(*this);
    else
      return ES_FIX;
  }

  // Dispose propagator and return its size
  virtual size_t dispose(Space& home) {
    x.cancel(home,*this,PC_INT_BND);
    y.cancel(home,*this,PC_INT_BND);
    (void) Propagator::dispose(home);
    return sizeof(*this);
  }
}; // end of class NoOverlap

/*
 * Post the constraint that the rectangles defined by the coordinates
 * x and y and width w and height h do not overlap.
 *
 * This is the function that you will call from your model. The best
 * is to paste the entire file into your model.
 */
void nooverlap2(Home home,
               const IntVarArgs& x, const IntArgs& w,
               const IntVarArgs& y, const IntArgs& h) {
  // Check whether the arguments make sense
  if ((x.size() != y.size()) || (x.size() != w.size()) ||
      (y.size() != h.size()))
    throw ArgumentSizeMismatch("nooverlap");
  // Never post a propagator in a failed space
  if (home.failed()) return;
  // Set up array of views for the coordinates
  ViewArray<IntView> vx(home,x);
  ViewArray<IntView> vy(home,y);
  // Set up arrays (allocated in home) for width and height and initialize
  int* wc = static_cast<Space&>(home).alloc<int>(x.size());
  int* hc = static_cast<Space&>(home).alloc<int>(y.size());
  for (int i=x.size(); i--; ) {
    wc[i]=w[i]; hc[i]=h[i];
  }
  // If posting failed, fail space
  if (NoOverlap::post(home,vx,wc,vy,hc) != ES_OK)
    home.fail();
}





class Square : public Script {
protected:
  IntVar s;      // side of the enclosing box
  IntVarArray x; // x coordinates for the squares
  IntVarArray y; // y coordinates for the squares

private:
  // size function returns the size of the square of a given index
  static int size(int i){
    return n-i;
  }
  // function for the interval and split branching options
  static bool filter(const Space& home, IntVar x, int i){
    return x.size() >= 0.3*size(i);
  }
  // function for initial domain reduction (forbidden gaps)
  static int forbiddenGap (int i){
    if (size(i)==45)
      return 10;
    if (size(i)>=34)
      return 9;
    if (size(i)>=30)
      return 8;
    if (size(i)>=22)
      return 7;
    if (size(i)>=18)
      return 6;
    if (size(i)>=12)
      return 5;
    if (size(i)>=9)
      return 4;
    if (size(i)>=5)
      return 3;
    if (size(i)>=2)
      return 2;
    else
      return -1;
  }

public:
  // Branching variants
  enum {
    BRANCH_X_FIRST,    ///< Assign x then y
    BRANCH_BIG_FIRST,  ///< Try bigger squares first
    BRANCH_LEFT_FIRST, ///< Try to place squares from left to right
    BRANCH_TOP_FIRST,  ///< Try to place squares from top to bottom
    BRANCH_INTERVAL,   ///< Split x, assign x, split y, then assign y
    BRANCH_SPLIT       ///< Split x, split y, assign x, then assign y
  };
  // Model variants
  enum {
    MODEL_NO_PROP,     ///< do not use external propagator
    MODEL_PROP         ///< use external propagator (function nooverlap)
  };
  
  Square(const SizeOptions& opt): Script(opt) {
    n = opt.size();
    double ceilSqrt = ceil(sqrt(n));
    // Minimum value for s: sum of areas of all the squares
    // Maximum value for s: n*ceil(sqrt(n)), which gives a total area
    // of at least n*n*n for the box, which can be divided into n square
    // sub-boxes of size n, so each sub-box can contain one square of
    // k (1 <= k <= n)
    s = IntVar(*this, ceil(sqrt(n*(n+1)*(2*n+1)/6)), n*ceilSqrt);
    // Square of size 1 are ignored (only n-1 variables)
    x = IntVarArray(*this, n-1, 0, n*ceilSqrt-1);
    y = IntVarArray(*this, n-1, 0, n*ceilSqrt-1);

    
    // max coordinates depending on the size of the squares
    for (int i=0; i<n-1; i++){
      rel(*this, x[i]<=s-size(i));
      rel(*this, y[i]<=s-size(i));
    }

    // symmetry removal
    rel(*this, x[0]<=(s-n)/2);
    rel(*this, y[0]<=x[0]);

    // initial domain reduction (forbidden gaps)
    for (int i=0; i<n-1; i++) {
      if (size(i)<=45 || size(i)>=2){
	rel(*this, x[i]!=forbiddenGap(i));
	rel(*this, y[i]!=forbiddenGap(i));
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

    // symmetry removal
    rel(*this, x[0]<=(s-n)/2);
    rel(*this, y[0]<=x[0]);

    
    // Constraint no overlap - disjoint
    switch (opt.model()) {
      case MODEL_NO_PROP:
	for (int i=0; i<n-1; i++){
	  for (int j=i+1; j<n-1; j++){
	    rel(*this, x[j]>=x[i]+size(i) || x[i]>=x[j]+size(j) ||
		y[i]>=y[j]+size(j) || y[j]>=y[i]+size(i));
	  }
	}
	break;
	
    case MODEL_PROP: // no overlap using nooverlap propagator
      // construct side array
      int sArr[n-1];
      for (int i=0; i<n-1; i++) {
	sArr[i] = size(i);
      }

      nooverlap2(*this,
		IntVarArgs(x), IntArgs(n-1, sArr),
		IntVarArgs(y), IntArgs(n-1, sArr));
      break;
    }

    // Constraint no overlap - cumulative
    for (int k=0; k<n*ceilSqrt; k++){
      // boolean array for the reified constraint
      // bc[i]=true if the square of index i have a non-empty
      // intersection with column k
      // br[i]=true if the square of index i have a non-empty
      // intersection with row k
      BoolVarArray bc(*this, n-1, 0, 1);
      BoolVarArray br(*this, n-1, 0, 1);
      for (int i=0; i<n-1; i++){
	dom(*this, x[i], k-size(i)+1, k, bc[i]);
	dom(*this, y[i], k-size(i)+1, k, br[i]);
      }
      // The sum of the sizes of all squares having non-empty
      // intersection with column/row k must be lower or equal to s
      // The size of one square is added in the sum if and only if
      // bc/br=true
      rel(*this, sum(IntArgs::create(n-1,n,-1), bc) <= s);
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
}; // end of class Square

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

  opt.model(Square::MODEL_NO_PROP, 
            "noprop", "no extern no-overlap propagator");
  opt.model(Square::MODEL_PROP, 
            "prop", "use extern no-overlap propagator");
  opt.model(Square::MODEL_NO_PROP);
  
  opt.parse(argc,argv);
  if (opt.size() < 2) {
    std::cerr << "Error: size must be greater than 1" << std::endl;
    return 1;
  }
  
  // run script
  Script::run<Square,BAB,SizeOptions>(opt);
  return 0;
}

