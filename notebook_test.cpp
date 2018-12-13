#include <QTest>
#include "notebook_app.hpp"
#include "thread_safe.hpp"
class NotebookTest : public QObject {
  Q_OBJECT

private slots:
	void findWidgets();
  void initTestCase();
	void checkMaketext();
	void checkKeyPresses();
	void numberOfWidgets();
	void checkPointSingle();
	void checkPointMultiple();
	void checkLineSingle();
	void checkLineMultiple();
	void parseError();
	void checkProcedure();
	void testDiscretePlotLayout();
private:
	NotebookApp Notebook;
};

void NotebookTest::initTestCase(){
	Notebook.show();
}

void NotebookTest::findWidgets()
{
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	QVERIFY2(in, "Input Widget not found");
	QVERIFY2(out, "Output Widget not found");
}

void NotebookTest::checkMaketext()
{
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	QString input = "(make-text \"Hello World\")";
	QTest::keyClicks(in, input);
	QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
	//auto  t = out->scene->items().size();
	QTest::qWait(250);
	QVERIFY(out->scene->items().size() == 1);
	
}

void NotebookTest::checkKeyPresses()
{
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	QTest::keyClicks(in, QString("(+ 2 3)"));

	QTest::keyPress(in, Qt::Key_Return, Qt::AltModifier, 100);
	QTest::qWait(250);
	foreach(QGraphicsItem * subItems, out->scene->items()) {
		if (dynamic_cast<const QGraphicsTextItem*>(subItems) != 0) {
			auto textItem = dynamic_cast<const QGraphicsTextItem*>(subItems);
			QVERIFY2(textItem->toPlainText() != "(5)", "Invalid keypress with alt");
		}
	}

	QTest::keyPress(in, Qt::Key_Return, Qt::NoModifier, 100);
	QTest::qWait(250);
	foreach(QGraphicsItem * subItems, out->scene->items()) {
		if (dynamic_cast<const QGraphicsTextItem*>(subItems) != 0) {
			auto textItem = dynamic_cast<const QGraphicsTextItem*>(subItems);
			QVERIFY2(textItem->toPlainText() != "(5)", "Invalid keypress with no modifier");
		}
	}

	QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
	QTest::qWait(250);
	foreach(QGraphicsItem * subItems, out->scene->items()) {
		if (dynamic_cast<const QGraphicsTextItem*>(subItems) != 0) {
			auto textItem = dynamic_cast<const QGraphicsTextItem*>(subItems);
			auto text = textItem->toPlainText();
			auto newt = text.toStdString();
			QVERIFY2(textItem->toPlainText() == "(5)", "Invalid Output for (+ 2 3)");
		}
	}
}


void NotebookTest::numberOfWidgets()
{
	auto inputChild = Notebook.findChildren<QPlainTextEdit *>();
	auto outputChild = Notebook.findChildren<QGraphicsView *>();
	auto outputChild2 = Notebook.findChildren<QGraphicsScene *>();
	QVERIFY2(inputChild.size() == 1, "Invalid Number of children");
	QVERIFY2(outputChild.size() == 1, "Invalid Number of children");
	QVERIFY2(outputChild2.size() == 0, "Invalid Number of children");

}

void NotebookTest::checkPointSingle() {
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	in->clear();
	QTest::keyClicks(in, QString("(make-point 0 1)"));
	QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
	QTest::qWait(250);
	QVERIFY2(out->scene->items().size() == 1, "Invalid number of graphics shown");
	foreach(QGraphicsItem * subItems, out->scene->items()) {
		if (dynamic_cast<const QGraphicsEllipseItem*>(subItems) != 0) {
			auto ellipseItem = dynamic_cast<const QGraphicsEllipseItem*>(subItems);
			QVERIFY2(ellipseItem->boundingRect() == QRectF(0, 1, 0, 0), "Invalid output for point with 0 diameter");
		}
	}


	QTest::keyClicks(in, QString("(set-property \"size\" 10 (make-point 0 1))"));
	QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
	QTest::qWait(250);
	QVERIFY2(out->scene->items().size() == 1, "Invalid number of graphics shown");
		foreach(QGraphicsItem * subItems, out->scene->items()) {
			if (dynamic_cast<const QGraphicsEllipseItem*>(subItems) != 0) {
				auto ellipseItem = dynamic_cast<const QGraphicsEllipseItem*>(subItems);
				QVERIFY2(ellipseItem->boundingRect() == QRectF(-5, -4, 10, 10), "Invalid output for point with 10 diameter");
			}
		}

}

void NotebookTest::checkPointMultiple()
{
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	QTest::keyClicks(in, QString("(list (set-property \"size\" 10 (make-point 0 1)) (set-property \"size\" 20 (make-point 10 10)))"));
	QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
	QTest::qWait(250);
	QVERIFY2(out->scene->items().size() == 2, "Invalid number of graphics shown");

	auto items = out->scene->items();

	QGraphicsItem * subItem1 = *items.begin();
	auto ellipseItem1 = dynamic_cast<const QGraphicsEllipseItem*>(subItem1);
	QVERIFY2(ellipseItem1->boundingRect() == QRectF(0,0,20,20), "Invalid output for point with 20 diameter at 10 10");

	QGraphicsItem * subItem2 = items[1];
	auto ellipseItem2 = dynamic_cast<const QGraphicsEllipseItem*>(subItem2);
	//auto r2 = ellipseItem2->boundingRect();
	QVERIFY2(ellipseItem2->boundingRect() == QRectF(-5, -4, 10, 10), "Invalid output for point with 10 diameter at 0 1");
}

void NotebookTest::checkLineSingle()
{
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	QTest::keyClicks(in, QString("(make-line (make-point 3 3) (make-point 4 4))"));
	QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
	QTest::qWait(250);
	QVERIFY2(out->scene->items().size() == 1, "Invalid number of graphics shown");

	///QGraphicsLineItem
	QGraphicsItem * line = out->scene->items()[0];
	auto lineItem = dynamic_cast<QGraphicsLineItem*>(line);
	QVERIFY2(lineItem->line() == QLineF(3, 3, 4, 4), "Invalid line");

}

void NotebookTest::checkLineMultiple()
{
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	QTest::keyClicks(in, QString("(list (make-line (make-point 3 3) (make-point 4 4)) (make-line (make-point 1 1) (make-point 0 4)) )"));
	QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
	QTest::qWait(250);
	QVERIFY2(out->scene->items().size() == 2, "Invalid number of graphics shown");

	///QGraphicsLineItem
	QGraphicsItem * line1 = out->scene->items()[0];
	auto lineItem1 = dynamic_cast<QGraphicsLineItem*>(line1);
	QVERIFY2(lineItem1->line() == QLineF(1,1,0,4), "Invalid line");

	QGraphicsItem * line2 = out->scene->items()[1];
	auto lineItem2 = dynamic_cast<QGraphicsLineItem*>(line2);
	QVERIFY2(lineItem2->line() == QLineF(3,3,4,4), "Invalid line");
}

void NotebookTest::parseError() {
	
	QList<QString> inputs = { "(begin","(define","make-line "};
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");

	for (auto a : inputs) {
		QTest::keyClicks(in, a);
		QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
		QTest::qWait(250);
		QGraphicsItem *text = out->scene->items()[0];
		auto error = dynamic_cast<QGraphicsTextItem*>(text);
		QVERIFY(error->toPlainText() == "Error: Invalid Program. Could not parse.");
	}
}

void NotebookTest::checkProcedure() {
	QList<QString> inputs = {"(lambda (x) (+ 2 x))","(lambda (x) (make-point x 3))"};
	auto in = Notebook.findChild<InputWidget *>("input");
	auto out = Notebook.findChild<OutputWidget *>("output");
	for (auto a : inputs) {
		QTest::keyClicks(in, a);
		QTest::keyPress(in, Qt::Key_Return, Qt::ShiftModifier, 100);
		QTest::qWait(250);
		QVERIFY(out->scene->items().size() == 0);
	}
}



/*
findLines - find lines in a scene contained within a bounding box
with a small margin
*/
int findLines(QGraphicsScene * scene, QRectF bbox, qreal margin) {

	QPainterPath selectPath;

	QMarginsF margins(margin, margin, margin, margin);
	selectPath.addRect(bbox.marginsAdded(margins));
	scene->setSelectionArea(selectPath, Qt::ContainsItemShape);

	int numlines(0);
	foreach(auto item, scene->selectedItems()) {
		if (item->type() == QGraphicsLineItem::Type) {
			numlines += 1;
		}
	}

	return numlines;
}

/*
findPoints - find points in a scene contained within a specified rectangle
*/
int findPoints(QGraphicsScene * scene, QPointF center, qreal radius) {

	QPainterPath selectPath;
	selectPath.addRect(QRectF(center.x() - radius, center.y() - radius, 2* radius, 2*radius));
	scene->setSelectionArea(selectPath, Qt::ContainsItemShape);

	int numpoints(0);
	foreach(auto item, scene->selectedItems()) {
		if (item->type() == QGraphicsEllipseItem::Type) {
			numpoints += 1;
		}
		qDebug() << item->boundingRect();
	}

	return numpoints;
}

/*
findText - find text in a scene centered at a specified point with a given
rotation and string contents
*/
int findText(QGraphicsScene * scene, QPointF center, qreal rotation, QString contents) {

	int numtext(0);
	foreach(auto item, scene->items(center)) {
		if (item->type() == QGraphicsTextItem::Type) {
			QGraphicsTextItem * text = static_cast<QGraphicsTextItem *>(item);
			
			if ((text->toPlainText() == contents) &&
				(text->rotation() == rotation) &&
				(text->pos() + text->boundingRect().center() == center)) {
				numtext += 1;
			}
		}
	}

	return numtext;
}

/*
intersectsLine - find lines in a scene that intersect a specified rectangle
*/
int intersectsLine(QGraphicsScene * scene, QPointF center, qreal radius) {

	QPainterPath selectPath;
	selectPath.addRect(QRectF(center.x() - radius, center.y() - radius, 2 * radius, 2 * radius));
	scene->setSelectionArea(selectPath, Qt::IntersectsItemShape);

	int numlines(0);
	foreach(auto item, scene->selectedItems()) {
		if (item->type() == QGraphicsLineItem::Type) {
			numlines += 1;
		}
	}

	return numlines;
}
void NotebookTest::testDiscretePlotLayout() {

	std::string program = R"( 
(discrete-plot (list (list -1 -1) (list 1 1)) 
    (list (list "title" "The Title") 
          (list "abscissa-label" "X Label") 
          (list "ordinate-label" "Y Label") ))
)";
	auto inputWidget = Notebook.findChild<InputWidget *>("input");
	auto outputWidget = Notebook.findChild<OutputWidget *>("output");
	inputWidget->setPlainText(QString::fromStdString(program));
	QTest::keyClick(inputWidget, Qt::Key_Return, Qt::ShiftModifier);
	QTest::qWait(2000);
	auto view = outputWidget->findChild<QGraphicsView *>();
	QVERIFY2(view, "Could not find QGraphicsView as child of OutputWidget");

	auto scene = view->scene();

	// first check total number of items
	// 8 lines + 2 points + 7 text = 17
	auto items = scene->items();
	QCOMPARE(items.size(), 17);

	// make them all selectable
	foreach(auto item, items) {
		item->setFlag(QGraphicsItem::ItemIsSelectable);
	}

	double scalex = 20.0 / 2.0;
	double scaley = 20.0 / 2.0;

	double xmin = scalex * -1;
	double xmax = scalex * 1;
	double ymin = scaley * -1;
	double ymax = scaley * 1;
	double xmiddle = (xmax + xmin) / 2;
	double ymiddle = (ymax + ymin) / 2;

	// check title
	QCOMPARE(findText(scene, QPointF(xmiddle, -(ymax + 3)), 0, QString("The Title")), 1);

	// check abscissa label
	QCOMPARE(findText(scene, QPointF(xmiddle, -(ymin - 3)), 0, QString("X Label")), 1);

	// check ordinate label
	QCOMPARE(findText(scene, QPointF(xmin - 3, -ymiddle), -90, QString("Y Label")), 1);

	// check abscissa min label
	QCOMPARE(findText(scene, QPointF(xmin, -(ymin - 2)), 0, QString("-1")), 1);

	// check abscissa max label
	QCOMPARE(findText(scene, QPointF(xmax, -(ymin - 2)), 0, QString("1")), 1);

	// check ordinate min label
	QCOMPARE(findText(scene, QPointF(xmin - 2, -ymin), 0, QString("-1")), 1);

	// check ordinate max label
	QCOMPARE(findText(scene, QPointF(xmin - 2, -ymax), 0, QString("1")), 1);

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

QTEST_MAIN(NotebookTest)
#include "notebook_test.moc"

