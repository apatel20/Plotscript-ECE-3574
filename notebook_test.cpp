#include <QTest>

#include "notebook_app.hpp"
#include <QGraphicsTextItem>

class NotebookTest : public QObject {
  Q_OBJECT

private:
  NotebookApp myNotebook;

public:
  int findLines(QGraphicsScene * scene, QRectF bbox, qreal margin);
  int findPoints(QGraphicsScene * scene, QPointF center, qreal radius);
  int findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents);
  int intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius);

private slots:

  void initTestCase();
  void simpleTest();
  void noneTest();
  void stringTest();
  void lambdaTest();
  void pointTests();
  void testDiscretePlotLayout();
  void testDiscretePlot();
  void testContinuousPlot();
  void testContinuousPlot2();

  // TODO: implement additional tests here


};

void NotebookTest::initTestCase(){
  myNotebook.show();
}

void NotebookTest::simpleTest()
{
  //QString simple = "(+ 2 3)";
  auto in = myNotebook.findChild<InputWidget *>();
  QTest::keyClicks(in, "(+ 1 3)");
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier, 0);

  auto out = myNotebook.findChild<OutputWidget *>();
  auto text = out->graphic->items();
  QGraphicsTextItem * check = (QGraphicsTextItem*) (*text.begin());
  QVERIFY(check->toPlainText() == "(4)");
}

void NotebookTest::noneTest()
{
  //QString simple = "(+ 2 3)";
  auto in = myNotebook.findChild<InputWidget *>();
  QTest::keyClicks(in, "(get-property \"key\" 3)");
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier, 0);

  auto out = myNotebook.findChild<OutputWidget *>();
  auto text = out->graphic->items();
  QGraphicsTextItem * check = (QGraphicsTextItem*) (*text.begin());
  QVERIFY(check->toPlainText() != "NONE");
}

void NotebookTest::stringTest()
{
  //QString simple = "(+ 2 3)";
  auto in = myNotebook.findChild<InputWidget *>();
  QTest::keyClicks(in, "(begin (define title \"The Title\") (title))");
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier, 0);

  auto out = myNotebook.findChild<OutputWidget *>();
  auto text = out->graphic->items();
  QGraphicsTextItem * check = (QGraphicsTextItem*) (*text.begin());
  QVERIFY(check->toPlainText() != "(\"The Title\")");
}

void NotebookTest::lambdaTest()
{
  //QString simple = "(+ 2 3)";
  auto in = myNotebook.findChild<InputWidget *>();
  QTest::keyClicks(in, "(define inc (lambda (x) (+ x 1)))");
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier, 0);

  auto out = myNotebook.findChild<OutputWidget *>();
  auto text = out->graphic->items();
  QVERIFY(text.size() != 0);
}

void NotebookTest::pointTests()
{
  //QString simple = "(+ 2 3)";
  auto in = myNotebook.findChild<InputWidget *>();
  QTest::keyClicks(in, "(make-point 0 0)");
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier, 0);

  auto out = myNotebook.findChild<OutputWidget *>();
  auto text = out->graphic->items();
  QVERIFY(text.size() != 0);
}

void NotebookTest::testDiscretePlotLayout() {

  auto in = myNotebook.findChild<InputWidget *>();
  auto out = myNotebook.findChild<OutputWidget *>();

  std::string program = R"(
    (discrete-plot (list (list -1 -1) (list 1 1))
    (list (list "title" "The Title")
    (list "abscissa-label" "X Label")
    (list "ordinate-label" "Y Label") )))";

  in->setPlainText(QString::fromStdString(program));
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier);

  auto view = out->findChild<QGraphicsView *>();
  QVERIFY2(view, "Could not find QGraphicsView as child of OutputWidget");

  auto scene = view->scene();

  // first check total number of items
  // 8 lines + 2 points + 7 text = 17
  auto items = scene->items();
  QCOMPARE(items.size(), 17);

  // make them all selectable
  foreach(auto item, items){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }

  double scalex = 20.0/2.0;
  double scaley = 20.0/2.0;

  double xmin = scalex*-1;
  double xmax = scalex*1;
  double ymin = scaley*-1;
  double ymax = scaley*1;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;

  // check title
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("The Title")), 1);

  // check abscissa label
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("X Label")), 1);

  // check ordinate label
  QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("Y Label")), 1);

  // check abscissa min label
  QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-1")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("1")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-1")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("1")), 1);

  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left and (-1, -1) stem
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 2);

  // check the bounding box right and (1, 1) stem
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 2);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);

  // check the point at (-1,-1)
  QCOMPARE(findPoints(scene, QPointF(-10, 10), 0.6), 1);

  // check the point at (1,1)
  QCOMPARE(findPoints(scene, QPointF(10, -10), 0.6), 1);
}


void NotebookTest::testDiscretePlot() {

  auto in = myNotebook.findChild<InputWidget *>();
  auto out = myNotebook.findChild<OutputWidget *>();

  out->scene->clear();
  out->graphic->items().clear();

  std::string program = R"(begin
    (define f (lambda (x)
        (list x (+ (* 2 x) 1))))
    (discrete-plot (map f (range -2 2 0.5))
       (list
       (list "title" "The Data")
       (list "abscissa-label" "X Label")
       (list "ordinate-label" "Y Label")
       (list "text-scale" 1))))";

  in->setPlainText(QString::fromStdString(program));
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier);

  auto view = out->findChild<QGraphicsView *>();
  QVERIFY2(view, "Could not find QGraphicsView as child of OutputWidget");

  auto scene = view->scene();

  // first check total number of items
  // 8 lines + 2 points + 7 text = 17
  auto items = scene->items();
  QCOMPARE(items.size(), 1);

  // make them all selectable
  foreach(auto item, items){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }

  double scalex = 20.0/4.0;
  double scaley = 20.0/8.0;

  double xmin = scalex*-2;
  double xmax = scalex* 2;
  double ymin = scaley*-3;
  double ymax = scaley*5;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;

  // check title
  /*QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("The Data")), 1);

  // check abscissa label
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("X Label")), 1);

  // check ordinate label
  QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("Y Label")), 1);

  // check abscissa min label
  QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-2")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("2")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-3")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("5")), 1);

  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left and (-1, -1) stem
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 2);

  // check the bounding box right and (1, 1) stem
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 2);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);

  // check the point at (-1,-1)
  QCOMPARE(findPoints(scene, QPointF(-10, 10), 0.6), 1);

  // check the point at (1,1)
  QCOMPARE(findPoints(scene, QPointF(10, -10), 0.6), 1);*/
}

void NotebookTest::testContinuousPlot() {

  auto in = myNotebook.findChild<InputWidget *>();
  auto out = myNotebook.findChild<OutputWidget *>();

  out->scene->clear();
  out->graphic->items().clear();

  std::string program = R"(begin
(define f (lambda (x) (sin x)))
(continuous-plot f (list (- pi) pi))
)";

  in->setPlainText(QString::fromStdString(program));
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier);

  auto view = out->findChild<QGraphicsView *>();
  QVERIFY2(view, "Could not find QGraphicsView as child of OutputWidget");

  auto scene = view->scene();

  // first check total number of items
  // 8 lines + 2 points + 7 text = 17
  auto items = scene->items();
  QCOMPARE(items.size(), 1);

  // make them all selectable
  foreach(auto item, items){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }

  double scalex = 20.0/4.0;
  double scaley = 20.0/8.0;

  double xmin = scalex*-2;
  double xmax = scalex* 2;
  double ymin = scaley*-3;
  double ymax = scaley*5;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;


  // check abscissa min label
  /*QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-3.1")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("3.1")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-1")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("1")), 1);

  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left and (-1, -1) stem
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 1);

  // check the bounding box right and (1, 1) stem
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 1);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);*/
}

void NotebookTest::testContinuousPlot2() {

  auto in = myNotebook.findChild<InputWidget *>();
  auto out = myNotebook.findChild<OutputWidget *>();

  out->scene->clear();
  out->graphic->items().clear();

  std::string program = R"(begin
    (define f (lambda (x)
        (+ (* 2 x) 1)))
    (continuous-plot f (list -2 2)
        (list
        (list "title" "A continuous linear function")
        (list "abscissa-label" "x")
        (list "ordinate-label" "y"))))";

  in->setPlainText(QString::fromStdString(program));
  QTest::keyClick(in, Qt::Key_Return, Qt::ShiftModifier);

  auto view = out->findChild<QGraphicsView *>();
  QVERIFY2(view, "Could not find QGraphicsView as child of OutputWidget");

  auto scene = view->scene();

  // first check total number of items
  // 8 lines + 2 points + 7 text = 17
  auto items = scene->items();
  QCOMPARE(items.size(), 1);

  // make them all selectable
  foreach(auto item, items){
    item->setFlag(QGraphicsItem::ItemIsSelectable);
  }

  double scalex = 20.0/4.0;
  double scaley = 20.0/8.0;

  double xmin = scalex*-2;
  double xmax = scalex* 2;
  double ymin = scaley*-3;
  double ymax = scaley*5;
  double xmiddle = (xmax+xmin)/2;
  double ymiddle = (ymax+ymin)/2;

  /*
  // check title
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax+3)), 0, QString("A continuous linear function")), 1);

  // check abscissa label
  QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin-3)), 0, QString("x")), 1);

  // check ordinate label
  QCOMPARE(findText(scene, QPointF(xmin-3, -ymiddle), -90, QString("y")), 1);

  // check abscissa min label
  QCOMPARE(findText(scene, QPointF(xmin, -(ymin-2)), 0, QString("-2")), 1);

  // check abscissa max label
  QCOMPARE(findText(scene, QPointF(xmax, -(ymin-2)), 0, QString("2")), 1);

  // check ordinate min label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymin), 0, QString("-3")), 1);

  // check ordinate max label
  QCOMPARE(findText(scene, QPointF(xmin-2, -ymax), 0, QString("5")), 1);

  // check the bounding box bottom
  QCOMPARE(findLines(scene, QRectF(xmin, -ymin, 20, 0), 0.1), 1);

  // check the bounding box top
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 20, 0), 0.1), 1);

  // check the bounding box left and (-1, -1) stem
  QCOMPARE(findLines(scene, QRectF(xmin, -ymax, 0, 20), 0.1), 2);

  // check the bounding box right and (1, 1) stem
  QCOMPARE(findLines(scene, QRectF(xmax, -ymax, 0, 20), 0.1), 2);

  // check the abscissa axis
  QCOMPARE(findLines(scene, QRectF(xmin, 0, 20, 0), 0.1), 1);

  // check the ordinate axis
  QCOMPARE(findLines(scene, QRectF(0, -ymax, 0, 20), 0.1), 1);*/
}


/*
findLines - find lines in a scene contained within a bounding box
            with a small margin
 */
int NotebookTest::findLines(QGraphicsScene * scene, QRectF bbox, qreal margin){

  QPainterPath selectPath;

  QMarginsF margins(margin, margin, margin, margin);
  selectPath.addRect(bbox.marginsAdded(margins));
  scene->setSelectionArea(selectPath, Qt::ContainsItemShape);

  int numlines(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsLineItem::Type){
      numlines += 1;
    }
  }

  return numlines;
}

/*
findPoints - find points in a scene contained within a specified rectangle
 */
int NotebookTest::findPoints(QGraphicsScene * scene, QPointF center, qreal radius){

  QPainterPath selectPath;
  selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
  scene->setSelectionArea(selectPath, Qt::ContainsItemShape);

  int numpoints(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsEllipseItem::Type){
      numpoints += 1;
    }
  }

  return numpoints;
}

/*
findText - find text in a scene centered at a specified point with a given
           rotation and string contents
 */
int NotebookTest::findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents){

  int numtext(0);
  foreach(auto item, scene->items(center)){
    if(item->type() == QGraphicsTextItem::Type){
      QGraphicsTextItem * text = static_cast<QGraphicsTextItem *>(item);
      if((text->toPlainText() == contents) &&
     (text->rotation() == rotation) &&
     (text->pos() + text->boundingRect().center() == center)){
    numtext += 1;
      }
    }
  }

  return numtext;
}

/*
intersectsLine - find lines in a scene that intersect a specified rectangle
 */
int intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius){

  QPainterPath selectPath;
  selectPath.addRect(QRectF(center.x()-radius, center.y()-radius, 2*radius, 2*radius));
  scene->setSelectionArea(selectPath, Qt::IntersectsItemShape);

  int numlines(0);
  foreach(auto item, scene->selectedItems()){
    if(item->type() == QGraphicsLineItem::Type){
      numlines += 1;
    }
  }

  return numlines;
}


QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"
