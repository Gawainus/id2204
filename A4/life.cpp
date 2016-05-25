/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  Last modified:
 *     $Date: 2015-03-17 16:09:39 +0100 (Tue, 17 Mar 2015) $ by $Author: schulte $
 *     $Revision: 14447 $
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
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

/* Solution for n=8
 * Density: 36/64 = 0.5625
 * 11011011
 * 11011011
 * 00000000
 * 11011011
 * 11011011
 * 00000000
 * 11011011
 * 11011011

 *
 * Solution for n=9
 * Density: 43/81 ~ 0.531
 * 011011011
 * 001011010
 * 101000001
 * 110111111
 * 000100000
 * 110101111
 * 010101001
 * 100101010
 * 110011011
 * 
 *
 * Note on the implementation:
 * We also tried to implement a symmetry breaking condition by saying that
 * the density in the upper half of the board should be higher or equal to
 * the lower half, and the same for left and right. However, the computation
 * were only slightly faster for some value of n, and slower for other values,
 * so we finally removed these constraints.
 */


#include <gecode/driver.hh>

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#endif

using namespace Gecode;

/**
 * \brief %Example: n-%Queens puzzle
 *
 * Place n queens on an n times n chessboard such that they do not
 * attack each other.
 *
 * \ingroup Example
 *
 */
class Life : public Script {
public:

  // dimensions of the board and borders
  int dim;
  int dimWithBorder; // extra two layers of border
  int headIdx = 2;   // the first index on the board after border cells
  int tailIdx;       // the last index on the board before border cells

  // Number of live cells
  IntVar c;            // total number of live cells
  IntVarArray csquare; // number of live cells inside each square of size 3*3

  // Position of live cells
  IntVarArray q;


  /// The actual problem
  Life(const SizeOptions& opt) : Script(opt) {

    // init dimension related variables
    dim = opt.size();
    dimWithBorder = dim+4;
    tailIdx = headIdx+dim-1;

    // number of squares of size 3*3
    int squares = ceil(dim/3.)*ceil(dim/3.);
    c = IntVar(*this, 1, dim*dim);
    // in each square of size 3*3, the number of live cells is limited by 6
    csquare = IntVarArray(*this, squares, 0, 6);
    q = IntVarArray(*this, dimWithBorder*dimWithBorder,0,1);

    Matrix<IntVarArray> m(q, dimWithBorder, dimWithBorder);

    // outer border (only dead cells)
    for (int i=0; i<dimWithBorder; i++) {
      rel(*this, m(0, i) == 0);
      rel(*this, m(i, 0) == 0);
      rel(*this, m(dimWithBorder-1, i) == 0);
      rel(*this, m(i, dimWithBorder-1) == 0);
    }

    // inner border (only dead cells)
    for (int i=headIdx-1; i<=tailIdx+1; i++) {
      rel(*this, m(headIdx-1, i) == 0);
      rel(*this, m(i, headIdx-1) == 0);
      rel(*this, m(i, tailIdx+1) == 0);
      rel(*this, m(tailIdx+1, i) == 0);
    }

    // number of live cells
    int s = 0;
    for (int i=headIdx; i<=tailIdx; i+=3){
      for (int j=headIdx; j<=tailIdx; j+=3){
        rel(*this, csquare[s] == sum(m.slice(i, i+3, j, j+3)));;
        s++;
      }
    }
    rel(*this, sum(csquare) == c);
    
    // apply still life constraints to the board and the inner border
    for (int i=headIdx-1; i<=tailIdx+1; i++){
      for (int j=headIdx-1; j<=tailIdx+1; j++){
        // sum of all the values of the neighboors of cell (i,j)
        LinIntExpr around =
          m(i-1,j-1) + m(i,j-1) + m(i+1,j-1) +
          m(i-1,j) + m(i+1,j) +
          m(i-1,j+1) + m(i,j+1) + m(i+1,j+1);

        // from the paper, both CP and IP are used
        // the constraints below are the best subset of all constraints in the trick paper
        rel(*this, (m(i,j)==1) >> ((around == 2) || (around ==3)));
        rel(*this, (m(i,j)==0) >> (around != 3));
      }
    }

    // First branching on c in order to maximize the number of live cells
    branch(*this, c, INT_VAL_MAX());

    // Based on experiments,
    // AFC branching on the whole board works best for dimensions that is
    // not divisible by 3, while SQUARE branching works best for those divisible by 3
    // SQUARE branching consists in doing AFC branching on each 3*3 squares, taken
    // in a lexicographic order
    if (dim%3 != 0) {
      branch(*this, q, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_MAX());
    }
    else {
      for (int i=headIdx; i<=tailIdx; i+=3){
        for (int j=headIdx; j<=tailIdx; j+=3){
          branch(*this, m.slice(i,i+3,j,j+3),
                 INT_VAR_AFC_MAX(opt.decay()), INT_VAL_MAX());
        }
      }
    }
  } // end of Life Ctr

  /// Constructor for cloning \a s
  Life(bool share, Life& life) : Script(share, life) {

    dim = life.dim;
    dimWithBorder = life.dimWithBorder;
    headIdx = 2;
    tailIdx = life.tailIdx;

    q.update(*this, share, life.q);
    c.update(*this, share, life.c);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Life(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    bool checks = true;
    for (int i = headIdx; i <= tailIdx; i++) {
      for (int j = headIdx; j <= tailIdx; j++) {
        int sumOfNeibors =
            q[(i-1)*(dimWithBorder)+j-1].val() +
            q[(i-1)*(dimWithBorder)+j].val()+
            q[(i-1)*(dimWithBorder)+j+1].val()+
            q[i*(dimWithBorder)+j-1].val()+
            q[i*(dimWithBorder)+j+1].val()+
            q[(i+1)*(dimWithBorder)+j-1].val()+
            q[(i+1)*(dimWithBorder)+j].val()+
            q[(i+1)*(dimWithBorder)+j+1].val();

        bool nonStillCond =
            ((q[i*(dimWithBorder)+j].val() == 1) && (sumOfNeibors>3 || sumOfNeibors<2))
            ||
            ((q[i*(dimWithBorder)+j].val() == 0) && sumOfNeibors == 3);
        if (nonStillCond) {
          checks = false;
          os << "Cell (" << i-1 << ", " << j-1 << ") " << "is not still." << std::endl;
        }
      }
    }
    if (checks) {
      os << "The solution checks." << std::endl
      << "Every live cell has 2 or 3 neighbors." << std::endl
      << "Every dead cell has no 3 neighbors." << std::endl;

    }
    else {
      os << "The Board is not still!" << std:: endl;
    }
    os << "Number of live cells: " << c << std::endl << std::endl;
    for (int i = 0; i < dimWithBorder; i++) {
      for (int j = 0; j < dimWithBorder; j++) {
        int cell = q[i*(dimWithBorder)+j].val();
        if (i == 0 || i == 1 || i==dimWithBorder-1 || i== tailIdx+1) {
          os << "~";
        }
        else if (j ==0 || j ==1 || j == dimWithBorder-1 || j == tailIdx+1) {
          os << "|";
        }
        else if (cell == 1) {
          os << "O";

        }
        else {
          os << " ";

        }
      }
      os << std::endl;
    }
  }
};


#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
/// Inspector showing queens on a chess board
class LifeInspector : public Gist::Inspector {
protected:
  /// The graphics scene displaying the board
  QGraphicsScene* scene;
  /// The window containing the graphics scene
  QMainWindow* mw;
  /// The size of a field on the board
  static const int unit = 20;
public:
  /// Constructor
  LifeInspector(void) : scene(NULL), mw(NULL) {}
  /// Inspect space \a s
  virtual void inspect(const Space& s) {
    const Life& q = static_cast<const Life&>(s);
    const int n = sqrt(q.q.size());
    Matrix<IntVarArray> m(q.q, n, n);
    
    if (!scene)
      initialize();
    QList <QGraphicsItem*> itemList = scene->items();
    foreach (QGraphicsItem* i, scene->items()) {
      scene->removeItem(i);
      delete i;
    }

    for (int i=0; i<n; i++) {
      for (int j=0; j<n; j++) {
        scene->addRect(i*unit,j*unit,unit,unit);
        QBrush b(m(i,j).assigned() ? Qt::black : Qt::red);
        QPen p(m(i,j).assigned() ? Qt::black : Qt::white);
        for (IntVarValues xv(m(i,j)); xv(); ++xv) {
          if( xv.val() == 1)
            scene->addEllipse(QRectF(i*unit+unit/4,j*unit+unit/4,
                                     unit/2,unit/2), p, b);
        } 
      }
    }
    mw->show();    
  }
    
  /// Set up main window
  void initialize(void) {
    mw = new QMainWindow();
    scene = new QGraphicsScene();
    QGraphicsView* view = new QGraphicsView(scene);
    view->setRenderHints(QPainter::Antialiasing);
    mw->setCentralWidget(view);
    mw->setAttribute(Qt::WA_QuitOnClose, false);
    mw->setAttribute(Qt::WA_DeleteOnClose, false);
    QAction* closeWindow = new QAction("Close window", mw);
    closeWindow->setShortcut(QKeySequence("Ctrl+W"));
    mw->connect(closeWindow, SIGNAL(triggered()),
                mw, SLOT(close()));
    mw->addAction(closeWindow);
  }
  
  /// Name of the inspector
  virtual std::string name(void) { return "Board"; }
  /// Finalize inspector
  virtual void finalize(void) {
    delete mw;
    mw = NULL;
  }
};

#endif /* GECODE_HAS_GIST */

/** \brief Main-function
 *  \relates Life
 */
int main(int argc, char* argv[]) {
  SizeOptions opt("Life");
  opt.iterations(500);
  opt.size(5);

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
  LifeInspector ki;
  opt.inspect.click(&ki);
#endif

  opt.parse(argc,argv);
  Script::run<Life,DFS,SizeOptions>(opt);
  return 0;
}
