/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Christian Schulte <schulte@gecode.org>
 *     Yumen & Marion
 *
 *  Copyright:
 *     Christian Schulte, 2001
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
 *
 *  In this model, the constraints are the following:
 *  - there must be exactly one tile in each row and in each column
 *  - every diagonal contains at most one tile
 *
 *  Several branching options are available, the best one we have found
 *  is the default one based on AFC. During the branching, we always start
 *  with the maximum value (which is 1) because it propagates more constraints
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
class Queens : public Script {
public:
  /// Position of queens on boards (1 or 0)
  IntVarArray q;

  // Branching variants
  enum {
    BRANCH_NONE,        ///< Use lexicographic ordering
    BRANCH_SIZE,        ///< Use minimum size
    BRANCH_SIZE_AFC,    ///< Use minimum size over afc
    BRANCH_AFC          ///< Use maximum afc
  };
  
  /// The actual problem
  Queens(const SizeOptions& opt)
    : Script(opt), q(*this,opt.size()*opt.size(),0,1) {
    int n = opt.size();
    Matrix<IntVarArray> m(q, n, n);
    
    // only one 1 on each row
    for (int i = 0; i<n; i++) {
      linear(*this, m.row(i), IRT_EQ, 1);
    }

    // only one 1 on each column
    for (int j = 0; j<n; j++) {
      linear(*this, m.col(j), IRT_EQ, 1);
    }

    // constraint on each diagonal : no more than one tile
    for (int i=0; i<=n-2; i++){
      count(*this, q.slice(i, n+1, n-i), 1, IRT_LQ, 1);
    }

    for (int i=1; i<=n-2; i++){
      count(*this, q.slice(i*n, n+1, n-i), 1, IRT_LQ, 1);
    }

    for (int i=1; i<=n-2; i++){
      count(*this, q.slice(i*n, -(n-1), i+1), 1, IRT_LQ, 1);
    }

    for (int i=2; i<=n-1; i++){
      count(*this, q.slice(i*n-1, n-1, n-i+1), 1, IRT_LQ, 1);
    }

    count(*this, q.slice(n-1,n-1,n), 1, IRT_LQ, 1);

    if (opt.branching() == BRANCH_NONE) {
      branch(*this, q, INT_VAR_NONE(), INT_VAL_SPLIT_MAX());
    } else if (opt.branching() == BRANCH_SIZE) {
      branch(*this, q, INT_VAR_SIZE_MIN(), INT_VAL_MAX());
    } else if (opt.branching() == BRANCH_SIZE_AFC) {
      branch(*this, q, INT_VAR_AFC_SIZE_MAX(opt.decay()), INT_VAL_MAX());
    } else if (opt.branching() == BRANCH_AFC) {
      branch(*this, q, INT_VAR_AFC_MAX(opt.decay()), INT_VAL_MAX());
    }
  }

  /// Constructor for cloning \a s
  Queens(bool share, Queens& s) : Script(share,s) {
    q.update(*this, share, s.q);
  }

  /// Perform copying during cloning
  virtual Space*
  copy(bool share) {
    return new Queens(share,*this);
  }

  /// Print solution
  virtual void
  print(std::ostream& os) const {
    for (int i = 0; i < q.size(); i++) {
      int n = sqrt(q.size());
      os << q[i] << " ";
      if ((i+1) % n == 0)
        os << std::endl;
    }
    os << std::endl;
  }
};

#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
/// Inspector showing queens on a chess board
class QueensInspector : public Gist::Inspector {
protected:
  /// The graphics scene displaying the board
  QGraphicsScene* scene;
  /// The window containing the graphics scene
  QMainWindow* mw;
  /// The size of a field on the board
  static const int unit = 20;
public:
  /// Constructor
  QueensInspector(void) : scene(NULL), mw(NULL) {}
  /// Inspect space \a s
  virtual void inspect(const Space& s) {
    const Queens& q = static_cast<const Queens&>(s);
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
 *  \relates Queens
 */
int
main(int argc, char* argv[]) {
  SizeOptions opt("Queens");
  opt.iterations(500);
  opt.size(8);
  opt.solutions(1);

  opt.branching(Queens::BRANCH_AFC);
  opt.branching(Queens::BRANCH_NONE, "none", "none");
  opt.branching(Queens::BRANCH_SIZE, "size", "min size");
  opt.branching(Queens::BRANCH_SIZE_AFC, "sizeafc", "min size over afc");
  opt.branching(Queens::BRANCH_AFC, "afc", "maximum afc");
  
#if defined(GECODE_HAS_QT) && defined(GECODE_HAS_GIST)
  QueensInspector ki;
  opt.inspect.click(&ki);
#endif

  opt.parse(argc,argv);
  Script::run<Queens,DFS,SizeOptions>(opt);
  return 0;
}

// STATISTICS: example-any

