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

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>

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

class Life: public Script {
public:
  // dimension of the board
  int dim;
  int dimWithBorder;

  // Number of live cells
  IntVar c;

  // Position of live cells
  IntVarArray q;

  // The actual problem
  Life(const SizeOptions& opt) : Script(opt) {
    dim = opt.size();
    dimWithBorder = dim+2;
    // c = IntVar(*this, 1, (n/3)*(n/3)*4 + (n%3)*n*2 - (n%3)*(n%3));
    c = IntVar(*this, 1, dim*dim);
    q = IntVarArray(*this, dimWithBorder*dimWithBorder, 0, 1);

    Matrix<IntVarArray> m(q, dimWithBorder, dimWithBorder);

    // border
    for (int i = 0; i<dimWithBorder; i++){
      rel(*this, m(0,i) == 0);
      rel(*this, m(dim+1,i) == 0);
      rel(*this, m(i,0) == 0);
      rel(*this, m(i,dim+1) == 0);
    }

    // number of live cells
    rel(*this, sum(q) == c);

    //Stay alive
    for (int i=1; i<=dim; i++){
      for (int j=1; j<=dim; j++){
        LinIntExpr around =
          m(i-1,j-1) + m(i,j-1) + m(i+1,j-1) +
          m(i-1,j) + m(i+1,j) +
          m(i-1,j+1) + m(i,j+1) + m(i+1,j+1);

        rel(*this, (m(i,j)==1) >> ((around == 2) || (around ==3)));
        rel(*this, (m(i,j)==0) >> (around != 3));
      }
    }

    branch(*this, c, INT_VAL_MAX());
    branch(*this, q, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_MAX());
  }

  // Constructor for cloning \a s
  Life(bool share, Life& life) : Script(share, life) {
    dim = life.dim;
    dimWithBorder = life.dimWithBorder;

    q.update(*this, share, life.q);
    c.update(*this, share, life.c);
  }

  // Perform copying during cloning
  virtual Space* copy(bool share) {
    return new Life(share, *this);
  }

  // Print solution
  virtual void print(std::ostream& os) const {
    bool checks = true;
    for (int i = 1; i <= dim; i++) {
      for (int j = 1; j <= dim; j++) {
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
            ((q[i*(dimWithBorder)+j].val() == 1) && sumOfNeibors>3 || sumOfNeibors<2)
                            ||
            ((q[i*(dimWithBorder)+j].val() == 0) && sumOfNeibors == 3);
        if (nonStillCond) {
          checks = false;
          os << "Cell (" << i << ", " << j << ") " << "is not still." << std::endl;
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
        if (cell == 1) {
          os << "O";

        }
        else {
          os << " ";

        }
      }
      os << std::endl;
    }
  }
}; // end of Life class


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
    const int n = q.dim;
    Matrix<IntVarArray> m(q.q, n+2, n+2);
    
    if (!scene)
      initialize();
    QList <QGraphicsItem*> itemList = scene->items();
    foreach (QGraphicsItem* i, scene->items()) {
      scene->removeItem(i);
      delete i;
    }

    for (int i=1; i<=n; i++) {
      for (int j=1; j<=n; j++) {
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
    mw->connect(closeWindow, SIGNAL(triggered()), mw, SLOT(close()));
    mw->addAction(closeWindow);
  }
  
  /// Name of the inspector
  virtual std::string name(void) {
    return "Board";
  }

  /// Finalize inspector
  virtual void finalize(void) {
    delete mw;
    mw = NULL;
  }
}; // end of class LifeInspector

#endif // GECODE_HAS_GIST

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
  Script::run<Life, DFS, SizeOptions>(opt);
  return 0;
} // end of Main

// STATISTICS: example-any
