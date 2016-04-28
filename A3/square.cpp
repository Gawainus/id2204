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


using namespace Gecode;
using namespace Gecode::Int;

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
    double sqrtNPlusOne = sqrt(n+1);
    s = IntVar(*this, sqrt(n*(n+1)*(2*n+1)/6), n*sqrtNPlusOne);
    x = IntVarArray(*this, n-1, 0, n*sqrtNPlusOne-1);
    y = IntVarArray(*this, n-1, 0, n*sqrtNPlusOne-1);

    
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

    /*

    // no overlap - disjoint
    for (int i=0; i<n-1; i++){
      for (int j=i+1; j<n-1; j++){
        rel(*this, x[j]>=x[i]+size(i) || x[i]>=x[j]+size(j) ||
            y[i]>=y[j]+size(j) || y[j]>=y[i]+size(i));
      }
    }
    */

    // no overlap using our own propagator -- no-overlap.cpp


    // construct side array
    int sArr[n-1];
    for (int i=0; i<n-1; i++) {
      sArr[i] = size(i);
    }

    nooverlap(*this, IntVarArgs(x), IntArgs(n-1, sArr), IntVarArgs(y), IntArgs(n-1, sArr));

    // no overlap - cumulative column
    for (int k=0; k<n*sqrtNPlusOne; k++){
      BoolVarArray bc(*this, n-1, 0, 1);
      for (int i=0; i<n-1; i++){
	      //bc[i] = expr(*this, (x[i] <= k) && (x[i]+size(i)>k));
	      dom(*this, x[i], k-size(i)+1, k, bc[i]);
      }
      rel(*this, sum(IntArgs::create(n-1,n,-1), bc) <= s);
    }

    // no overlap - cumulative row
    for (int k=0; k<n*sqrtNPlusOne; k++) {
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
  
  opt.parse(argc,argv);
  if (opt.size() < 2) {
    std::cerr << "Error: size must be greater than 1" << std::endl;
    return 1;
  }
  
  // run script
  Script::run<Square,BAB,SizeOptions>(opt);
  return 0;
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
    // TODO: complete this function

    int n = x.size();
    bool subSumed = true;
    for (auto xItr: x) {
      subSumed &= xItr.assigned();
    }
    for (auto yIter: y) {
      subSumed &= yIter.assigned();
    }
    if (subSumed) {
      return home.ES_SUBSUMED(* this);
    }

    for (int i=0; i<n-1; i++) {
      for (int j = i + 1; j < n - 1; j++) {
        bool overlapped =
            x[i].gq(home, x[j].min()+w[j]) == ME_INT_FAILED &&
            x[j].gq(home, x[i].min()+w[i]) == ME_INT_FAILED &&
            y[i].gq(home, y[j].min()+h[j]) == ME_INT_FAILED &&
            y[j].gq(home, y[i].min()+h[i]) == ME_INT_FAILED;
        if (overlapped)
          return ES_FAILED;
      }
    }
    return ES_OK;

    return ES_NOFIX;
    return ES_FIX;
    return ES_NOFIX_FORCE;
    return __ES_PARTIAL;
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
void nooverlap(Home home,
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
